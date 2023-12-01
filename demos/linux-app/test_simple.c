#include <stdio.h>
#include <stdlib.h>
#include "xgfifo.h"

int main()
{
    printf("----- xgfifo simple test -----\n");

    xgff_t fifo;
    char buf[100] = "";

    xgff_init(&fifo, 0);
    xgff_init(&fifo, 3);
    xgff_init(&fifo, 1 << 5);

    printf("Write fifo len 6: 123456\n");
    xgff_wr(&fifo, "123456", 6);
    printf("Read fifo len 4\n");
    xgff_rd(&fifo, buf, 4);
    // printf("Get fifo: %s\n", buf);

    printf("Write fifo len 8: abcdefzz\n");
    xgff_wr(&fifo, "abcdefzz", 8);
    printf("Read fifo len all\n");
    xgff_rd(&fifo, buf, 100000);

    printf("Write fifo len 6: ABCDEF\n");
    xgff_wr(&fifo, "ABCDEF", 6);
    printf("Read fifo len 1\n");
    xgff_rd(&fifo, buf, 1);
}
