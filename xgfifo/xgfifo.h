#ifndef XGFIFO_H
#define XGFIFO_H

#include <stdbool.h>
// #include <pthread.h>

#ifndef XGFF_SHOW_FOOTPRINT
// 调试阶段使用，使能后会使用'-'来显示fifo占用情况
// 显示样式如下（红绿双色区分对应的位置是否有数据）
//  ---- ---- ---- ---- 31%
#define XGFF_SHOW_FOOTPRINT 1
#endif

typedef unsigned char byte;

/**
 * @brief 在不同环境下，对于线程同步的处理手段是不一样的，
 *        本模块不含依赖特定环境的代码，
 *        仅提供了一个函数原型，让用户根据使用环境去实现
 *
 * @param lock true 加锁; false 解锁
 * @param mutex 不同环境对应的线程同步数据结构，例如POSIX下的pthread_mutex_t*
 */
typedef void (*xgff_lockFn)(bool lock, void *mutex);

typedef struct xgfifo {
    byte       *data; /* the buffer holding the data */
    size_t      size; /* the size of the allocated buffer, should be power of 2 */
    size_t      mask; /* mask = size - 1 */
    size_t      in;   /* data is added at offset (in % size) */
    size_t      out;  /* data is extracted from off. (out % size) */
    void       *mutex;
    xgff_lockFn lockFn;

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
 * @brief 设置线程安全对应的数据结构
 *
 * @param fifo 指定fifo
 * @param lockFn 加锁/解锁对应的函数，需要在调用者代码中实现
 * @param mutex 加锁/解锁对应的mutex，例如POSIX下的pthread_mutex_t*
 */
int  xgff_setLock(xgff_t *fifo, xgff_lockFn lockFn, void *mutex);
bool xgff_34full(xgff_t *fifo);

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
