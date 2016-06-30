#ifndef MUTEX_H_NDUEKI2A
#define MUTEX_H_NDUEKI2A
#include <assert.h>
#include <pthread.h>

class Mutex {
public:
    Mutex() {
        assert(pthread_mutex_init(&mu_, NULL) == 0);
    }

    ~Mutex() {
        assert(pthread_mutex_destroy(&mu_) == 0);
    }

    void lock() {
        assert(pthread_mutex_lock(&mu_) == 0);
    }

    void unlock() {
        assert(pthread_mutex_unlock(&mu_) == 0);
    }

private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

private:
    pthread_mutex_t mu_;
};


class LockGuard {
public:
    LockGuard(Mutex& mu) : mutex_(mu) {
        mutex_.lock();
    }

    ~LockGuard() {
        mutex_.lock();
    }

private:
    LockGuard(const LockGuard&);
    LockGuard& operator=(const LockGuard&);

private:
    Mutex& mutex_;
};


#endif /* end of include guard: MUTEX_H_NDUEKI2A */
