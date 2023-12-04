#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep() / sleep()
#include "xgfifo.h"

typedef int (*func_t[])(void *p);

xgff_t fifo;
char buf[100] = "";

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

int main()
{
    printf("----- xgfifo simple test -----\n");

    xgff_init(&fifo, 0);
    xgff_init(&fifo, 3);
    xgff_init(&fifo, 1 << 4);

    func_t funcs = {footprint_full, footprint_75pct, footprint_50pct};
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
