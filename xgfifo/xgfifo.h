#ifndef XGFIFO_H
#define XGFIFO_H

#include <pthread.h>

#define XFIFO_PTR_OP

struct xfifo
{
    unsigned char *data; /* the buffer holding the data */
    unsigned int size;   /* the size of the allocated buffer, should be power of 2 */
    unsigned int mask;   // size-1
    unsigned int in;     /* data is added at offset (in % size) */
    unsigned int out;    /* data is extracted from off. (out % size) */
    // unsigned int is;     //in-shadow, in % size
    // unsigned int os;     //out-shadow, out % size
    // unsigned int len_data; /* 已经填入的数据长度 */
    // unsigned int len_left; /* 可用空间          */
    pthread_mutex_t mutex;

    //-------------------- 第二种操作方式专用
    char *ptr_i;        /* 写入位置 */
    char *ptr_o;        /* 读取位置 */
    unsigned int len_i; /* 外部通过ptr_i可写入的最大长度 */
    unsigned int len_o; /* 外部通过ptr_o可读取的最大长度 */
};

int xfifo_init(struct xfifo *fifo, unsigned int size);

int xfifo_34full(struct xfifo *fifo, int do_print);

int xfifo_empty(struct xfifo *fifo);

//---------- 下面4个函数是第1种处理机制

int xfifo_put(struct xfifo *fifo, const void *buf, unsigned int len);

int xfifo_get(struct xfifo *fifo, void *buf, unsigned int len);

int xfifo_getlen(struct xfifo *fifo);

int xfifo_getleftlen(struct xfifo *fifo);

//---------- 下面3个函数是第2种处理机制

static void refresh_len(struct xfifo *fifo, unsigned int in, unsigned int out);

void xfifo_wr_ack(struct xfifo *fifo, unsigned int offset);

void xfifo_rd_ack(struct xfifo *fifo, unsigned int offset);

#endif // XGFIFO_H
