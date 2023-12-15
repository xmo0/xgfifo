#ifndef XGFIFO_H
#define XGFIFO_H

#include <stdbool.h>
#include <pthread.h>

#define XGFF_SHOW_FOOTPRINT 1 // 调试阶段使用，使能后会使用'-'来显示fifo占用情况
                              // 显示样式如下（红绿双色区分对应的位置是否有数据）
                              //  ---- ---- ---- ---- 31%

typedef unsigned char byte;

typedef struct xgfifo
{
    byte *data;  /* the buffer holding the data */
    size_t size; /* the size of the allocated buffer, should be power of 2 */
    size_t mask; /* mask = size - 1 */
    size_t in;   /* data is added at offset (in % size) */
    size_t out;  /* data is extracted from off. (out % size) */
    pthread_mutex_t mutex;

    int (*footprint_full)(void *p);
    int (*footprint_75pct)(void *p);
    int (*footprint_50pct)(void *p);
} xgff_t;

/**
 *  下面3个函数用于建立数据结构、销毁数据结构、清空数据
 */
int xgff_init(xgff_t *fifo, int size);                       // 建立数据结构
int xgff_free(xgff_t *fifo);                                 // 销毁数据结构
int xgff_clear(xgff_t *fifo);                                // 清空内部数据
int xgff_setCallbacks(xgff_t *fifo, int (*func[])(void *p)); // 设置回调函数

/**
 *  下面4个函数是基础的使用方式，适用于小数据量的处理，
 *  优点是使用直观，缺点是（可能）存在额外的内存拷贝
 */
int xgff_getItemLen(xgff_t *fifo);                      // 获取当前数据长度
int xgff_getLeftLen(xgff_t *fifo);                      // 获取剩余可用长度
int xgff_wr(xgff_t *fifo, const void *buf, size_t len); // 写入数据
int xgff_rd(xgff_t *fifo, void *buf, size_t len);       // 读取数据

/**
 *  下面4个函数是另外一种使用方式，适用于大数据量的处理，
 *  优点是不存在额外的内存拷贝，缺点是使用方式不直观
 */
int xgff_getBlockWrInfo(xgff_t *fifo, byte **ptr, size_t *len);
int xgff_getBlockRdInfo(xgff_t *fifo, byte **ptr, size_t *len);
int xgff_ackBlockWr(xgff_t *fifo, size_t len);
int xgff_ackBlockRd(xgff_t *fifo, size_t len);

#endif // XGFIFO_H
