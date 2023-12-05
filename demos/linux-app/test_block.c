#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
}

int footprint_50pct(void *p)
{
    printf("callback: fifo 50pct\n");
}

int main()
{
    printf("----- xgfifo block test -----\n");
    xgff_init(&fifo, 1 << 5);

    func_t funcs = {footprint_full, footprint_75pct, footprint_50pct};
    xgff_setCallbacks(&fifo, funcs);

    byte *pdata;
    size_t len;
    xgff_getBlockWrInfo(&fifo, &pdata, &len);
    printf("pdata: %p, len: %ld\n", pdata, len);

    memcpy(pdata, "12345678901234567890", 20);
    xgff_ackBlockWr(&fifo, 20);

    xgff_getBlockWrInfo(&fifo, &pdata, &len);
    memcpy(pdata, "1234", 4);
    xgff_ackBlockWr(&fifo, 4);

    xgff_getBlockWrInfo(&fifo, &pdata, &len);
    memcpy(pdata, "12345678901234567890", len);
    xgff_ackBlockWr(&fifo, len);

    xgff_getBlockRdInfo(&fifo, &pdata, &len);
    printf("pdata: %p, len: %ld\n", pdata, len);
    memcpy(buf, pdata, 10);
    xgff_ackBlockRd(&fifo, 10);

    return 0;
}
