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
    xgff_init(&fifo, 1 << 3);

    printf("Write fifo: 123456\n");
    xgff_wr(&fifo, "123456", 6);
    xgff_rd(&fifo, buf, 4);
    printf("Get fifo: %s\n", buf);
    printf("fifo item len: %d\n", xgff_getItemLen(&fifo)); // 2
    printf("fifo left len: %d\n", xgff_getLeftLen(&fifo)); // 6

    printf("Write fifo: abcdefzz\n");
    xgff_wr(&fifo, "abcdefzz", 8);                         // only write 6 bytes
    printf("fifo item len: %d\n", xgff_getItemLen(&fifo)); // 8
    printf("fifo left len: %d\n", xgff_getLeftLen(&fifo)); // 0

    xgff_rd(&fifo, buf, 100000);
    printf("Get fifo: %s\n", buf);
    printf("fifo item len: %d\n", xgff_getItemLen(&fifo)); // 0
    printf("fifo left len: %d\n", xgff_getLeftLen(&fifo)); // 8
}

/*
Result:
    ----- xgfifo simple test -----
    xgff_init(): invalid argument.
    xgff_init(): invalid argument.
    Write fifo: 123456
    Get fifo: 1234
    fifo item len: 2
    fifo left len: 6
    Write fifo: abcdefzz
    fifo item len: 8
    fifo left len: 0
    Get fifo: 56abcdef
    fifo item len: 0
    fifo left len: 8
*/