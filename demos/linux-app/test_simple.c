#include <stdio.h>
#include <stdlib.h>
#include "xgfifo.h"

typedef int (*func_t[])(void *p);

int footprint_full(void *p)
{
    printf("fifo full\n");
}

int footprint_75pct(void *p)
{
    printf("fifo 75pct\n");
}

int footprint_50pct(void *p)
{
    printf("fifo 50pct\n");
}

int main()
{
    printf("----- xgfifo simple test -----\n");

    xgff_t fifo;
    char buf[100] = "";

    xgff_init(&fifo, 0);
    xgff_init(&fifo, 3);
    xgff_init(&fifo, 1 << 4);

    func_t funcs = {footprint_full, footprint_75pct, footprint_50pct};
    xgff_setCallbacks(&fifo, funcs);

    printf("Write fifo len 6: 123456\n");
    xgff_wr(&fifo, "123456", 6);
    printf("Read fifo len 4\n");
    xgff_rd(&fifo, buf, 4);
    // printf("Get fifo: %s\n", buf);

    printf("Write fifo len 6: abcdef\n");
    xgff_wr(&fifo, "abcdefzz", 6);

    printf("Write fifo len 4: 1A2B\n");
    xgff_wr(&fifo, "1A2B", 4);

    printf("Read fifo len all\n");
    xgff_rd(&fifo, buf, 100000);

    printf("Write fifo len 6: ABCDEF\n");
    xgff_wr(&fifo, "ABCDEF", 6);
    printf("Read fifo len 1\n");
    xgff_rd(&fifo, buf, 1);
}
