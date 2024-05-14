#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // for usleep() / sleep()
#include <pthread.h>
#include "xgfifo.h"

typedef int (*func_t[])(void *p);

xgff_t fifo;
char   buf[100] = "";

int footprint_full(void *p)
{
    printf("callback: fifo full\n");
}

int footprint_75pct(void *p)
{
    printf("callback: fifo 75pct\n");
    xgff_rd(&fifo, buf, xgff_getItemLen(&fifo) - 1);
}

int footprint_50pct(void *p)
{
    // printf("callback: fifo 50pct\n");
}

// 在POSIX下对xgff_lockFn()的一种实现方式
void xgffex_lock(bool lock, void *mutex)
{
    pthread_mutex_t *pmutex = (pthread_mutex_t *)(mutex);
    if (lock)
    {
        pthread_mutex_lock(pmutex);
    }
    else
    {
        pthread_mutex_unlock(pmutex);
    }
}

int main()
{
    printf("----- xgfifo simple test -----\n");

    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("init mutex failed\n");
        exit(EXIT_FAILURE);
    }

    xgff_init(&fifo, 0); // fail
    xgff_init(&fifo, 3); // fail

    xgff_init(&fifo, 1 << 4);
    xgff_setLock(&fifo, xgffex_lock, &mutex);

    func_t funcs = { footprint_full, footprint_75pct, footprint_50pct };
    xgff_setCallbacks(&fifo, funcs);

    printf("Write fifo len 16: 1234567890123456\n");
    xgff_wr(&fifo, "1234567890123456", 16);

    printf("Read fifo len 12\n");
    xgff_rd(&fifo, buf, 12);

    while (1)
    {
        sleep(1);
        xgff_wr(&fifo, "a", 1);
    }
}
