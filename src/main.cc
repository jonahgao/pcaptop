#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include "net_types.h"
#include "common.h"
#include "ui.h"


// SIZE_ETHERNET + MAX IP HEADER LENGTH + TCP FRIST 4 BYTES, 80 shouble be enough
#define CAPTURE_LENGTH 80
#define READ_TIMEOUT 1000 //ms

static std::string interface = "eth0";
static uint16_t port = 50088;
static bool verbose = false;
static pcap_t *pt = NULL;
static struct in_addr if_ip_addr;              // 接口IP地址,用来区分direction

/** 
 * 获取接口IP地址
 */
void getInterfaceIP(const char *interface, struct in_addr* if_ip_addr) {
    struct ifreq ifr = {};
    ifr.ifr_addr.sa_family = AF_INET;

    int s = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */
    if (s == -1) {
        perror("getInterfaceIP new socket failed.");
        exit(-1);
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ);

    if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
        fprintf(stderr, "Unable to get IP address for interface: %s\n", interface);
        perror("ioctl(SIOCGIFADDR)");
        exitUI();
        exit(-1);
    }
    else {
         memcpy(if_ip_addr, &((*(struct sockaddr_in *) &ifr.ifr_addr).sin_addr), sizeof(struct in_addr)); 
    }
}

void getPacket(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet) {
    const struct sniff_ethernet *ethernet = (struct sniff_ethernet *)packet;

    const struct sniff_ip *ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    u_int size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        fprintf(stderr, "   * Invalid IP header length: %u bytes\n", size_ip);
        return;
    }

    const struct sniff_tcp *tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    u_int size_tcp = TH_OFF(tcp) * 4;
    if (size_tcp < 20) {
        printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
        return;
    }

    static DataPoint data;
    data.flags = tcp->th_flags;
    data.pktlen = pkthdr->len;
    uint16_t sport = ntohs(tcp->th_sport);
    uint16_t dport = ntohs(tcp->th_dport);
    if (ip->ip_dst.s_addr == if_ip_addr.s_addr && dport == port) { // incoming
        data.ip = inet_ntoa(ip->ip_src);
        data.port = sport;
        data.direc = IN;
    }
    else if (ip->ip_src.s_addr == if_ip_addr.s_addr && sport == port) { // outcoming
        data.ip = inet_ntoa(ip->ip_dst);
        data.port = dport;
        data.direc = OUT;
    }

    addData(data);
}

volatile sig_atomic_t in_signal_handler = 0;
void signalHandler(int signo) {
    if (in_signal_handler)
        raise(signo);

    in_signal_handler = 1;

    exitUI();

    exit(0);
}

void setupSignals() {
    signal(SIGHUP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "i:p:hv")) != -1) {
        switch (opt) {
            case 'i':
                interface = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-i interface] [-p port]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // 打印启动参数
    printf("startup param: interface=%s, local port=%d, verbose=%d\n", interface.c_str(), port, verbose);


    // 获取接口硬件地址
    getInterfaceIP(interface.c_str(), &if_ip_addr);
    printf("%s ip address: %s\n", interface.c_str(), inet_ntoa(if_ip_addr));

    char errBuf[PCAP_ERRBUF_SIZE];

    // 打开接口
    pt = pcap_open_live(interface.c_str(), CAPTURE_LENGTH, false, READ_TIMEOUT, errBuf);
    if (pt == NULL) {
        printf("open interface error: %s\n", errBuf);
        exit(EXIT_FAILURE);
    }

    // 设置filter
    struct bpf_program filter;
    char expr[50] = {0};
    snprintf(expr, 50, "tcp port %u", port);
    int ret = pcap_compile(pt, &filter, expr, 1, 0);
    if (-1 == ret) {
        fprintf(stderr, "complie filter expr(%s) error: %s\n", 
                "dst port 80", pcap_geterr(pt));
        exit(EXIT_FAILURE);
    }
    ret = pcap_setfilter(pt, &filter);
    if (-1 == ret) {
        fprintf(stderr, "set filter expr error: %s\n", pcap_geterr(pt));
        exit(EXIT_FAILURE);
    }

    initUI(interface.c_str());

    setupSignals();

    pcap_loop(pt, -1, getPacket, NULL);

    pcap_close(pt);

    return 0;
}
