# pcaptop
显示到本机某个端口TCP流量最大和连接建立最多（SYN报文）的 top-N ip
## install
prerequisites: 
- libpcap (library and headers)
- libncurses (library and headers)

type `make`

## usage
pcaptop:    
    -i interface        指定接口，默认eth0    
    -p port             指定本地端口，默认80    
    -v                  verbose，打印调试信
