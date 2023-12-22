# xgfifo
## 功能简介
xgfifo, 本模块利用一个ring-buffer的结构，实现fifo功能。

已经在linux应用程序中做到线程安全，并且在项目中实际使用。准备找机会再将该模块移植到mcu程序中去，主要还是解决线程安全/中断安全的问题。

本模块提供两套独立的控制函数。

第一套属于常规用法，不做介绍。

第二套一般用于大数据量的处理。通过获取能够“块操作”的指针和长度后，基于指针来操作，省去了额外的内存拷贝。在库配套的例程中，不能很好体现出这个的优势。举个例子，某个fifo容量是1<<24字节（16MB），每次需要从fifo中读出大概1MB数量级的数据，再通过send()/fwrite()函数处理。这种情况下，第二套控制函数的性能将明显优于第一套。

## 函数接口
```C
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
```

## 主要特性
* 遵循C99
* fifo尺寸只能设置为2的幂
* fifo可用容量等于fifo尺寸
* 内部写索引、读索引不“归零”，允许增长至自然溢出
* 两套独立的使用模式
* POSIX线程安全

## 与一些相关库的对比
* 这个库主要参考了linux kernel kfifo，也少量参考了Majerle/lwrb
* kfifo使用C89，本库使用C99，Majerle/lwrb使用C11
* kfifo本身不考虑线程安全（就我目前理解来看），本库至少在已经在POSIX下做到了线程安全，Majerle/lwrb的线程安全还需要依赖调用代码的实现。（或许这才是最合适的方式？）
* kfifo及本库，fifo可用容量等于缓冲区容量；对于Majerle/lwrb及其它一些ring-buffer库，可用容量等于缓冲区容量-1
* 本库主要借用了kfifo的内部索引设计，读索引和写索引不在代码里面“归零”，允许其增长直至溢出。我很喜欢这个设计

## Todo
* 稍早一点的版本在项目中实际用过。当前上传到Github上的版本做了一些改动，可能存在BUG
* 计划移植到嵌入式平台上，在裸机和RTOS下完成线程安全相关的开发与验证

## 许可
采用 MIT 开源协议，细节请阅读项目中的 LICENSE 文件内容。
