#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xgfifo.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define shadow_in (fifo->in & fifo->mask)
#define shadow_out (fifo->out & fifo->mask)

#define xgff_error(msg) printf("%s(): %s\n", __func__, (msg))

#define xgff_cprint(condition, ...) \
	if (condition)                  \
		printf(__VA_ARGS__);

#define xgff_check_fifo_ptr(arg)         \
	if (fifo == NULL)                    \
	{                                    \
		xgff_error("invalid argument."); \
		return -1;                       \
	}

int xgff_init(xgff_t *fifo, int size)
{
	xgff_check_fifo_ptr(fifo);

	// 检查size，要求不小于4，不大于1<<30，并且为2的幂
	// 在我的典型工作环境下，int为32位，取值范围是-2^^31 ~ +2^^31-1
	// 1<<30 等同于1GiB，一般不会用到这么大
	if (size < 4 || size > 1 << 30 || (size & size - 1) != 0)
	{
		xgff_error("invalid argument.");
		return -1;
	}

	fifo->in = 0;
	fifo->out = 0;
	fifo->size = size;
	fifo->mask = size - 1;
	fifo->data = malloc(size);

	if (fifo->data == NULL)
	{
		xgff_error("malloc fail");
		exit(EXIT_FAILURE);
	}
	if (pthread_mutex_init(&fifo->mutex, NULL) != 0)
	{
		xgff_error("init mutex fail");
		free(fifo->data);
		exit(EXIT_FAILURE);
	}
}

int xgff_free(xgff_t *fifo)
{
	xgff_check_fifo_ptr(fifo);

	pthread_mutex_lock(&fifo->mutex);
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = 0;
	fifo->mask = 0;
	free(fifo->data);
	fifo->data = NULL;
	pthread_mutex_unlock(&fifo->mutex);
}

int xgff_clear(xgff_t *fifo)
{
	xgff_check_fifo_ptr(fifo);

	pthread_mutex_lock(&fifo->mutex);
	fifo->in = 0;
	fifo->out = 0;
	pthread_mutex_unlock(&fifo->mutex);
}

int xgff_getItemLen(xgff_t *fifo)
{
	xgff_check_fifo_ptr(fifo);

	size_t len;
	pthread_mutex_lock(&fifo->mutex);
	// 此处in可能已经溢出，回到一个较小的数，而out还没溢出
	// 即使遇到这种状况，下面的语句仍然是正确的
	len = fifo->in - fifo->out;
	pthread_mutex_unlock(&fifo->mutex);

	return len;
}

int xgff_getLeftLen(xgff_t *fifo)
{
	size_t left_len;

	pthread_mutex_lock(&fifo->mutex);
	// 此处in可能已经溢出，回到一个较小的数，而out还没溢出
	// 即使遇到这种状况，下面的语句仍然是正确的
	left_len = fifo->size - (fifo->in - fifo->out);
	pthread_mutex_unlock(&fifo->mutex);

	return left_len;
}

int xgff_wr(xgff_t *fifo, const void *buf, size_t len)
{
	xgff_check_fifo_ptr(fifo);

	// todo 这里为什么不直接使用pthread_mutex_lock()
	int r;
	r = pthread_mutex_trylock(&fifo->mutex); // 使用这个锁来优先保证写操作

	size_t lenToCopy;

	// 在用户要求数量和fifo剩余容量间取较小值
	len = min(len, fifo->size - (fifo->in - fifo->out));

	/* first put the data starting from fifo->in to buffer end */
	lenToCopy = min(len, fifo->size - (fifo->in & fifo->mask));
	memcpy(fifo->data + (fifo->in & fifo->mask), buf, lenToCopy);

	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(fifo->data, buf + lenToCopy, len - lenToCopy);

	if (r)
	{
		pthread_mutex_lock(&fifo->mutex);
	}
	fifo->in += len;

	// len = fifo->in - fifo->out;
	// printf("fifo len: %d, fifo_in = %u, fifo_out = %u\n",len,fifo->in,fifo->out);

	pthread_mutex_unlock(&fifo->mutex);

	return len;
}

int xgff_rd(xgff_t *fifo, void *buf, size_t len)
{
	xgff_check_fifo_ptr(fifo);

	pthread_mutex_lock(&fifo->mutex);
	size_t lenToCopy;

	// 在用户要求数量和fifo数据量间取较小值
	len = min(len, fifo->in - fifo->out);
	/* first get the data from fifo->out until the end of the buffer */
	lenToCopy = min(len, fifo->size - (fifo->out & fifo->mask));
	memcpy(buf, fifo->data + (fifo->out & fifo->mask), lenToCopy);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buf + lenToCopy, fifo->data, len - lenToCopy);
	fifo->out += len;

	pthread_mutex_unlock(&fifo->mutex);
	return len;
}

//------------------------------------------------------------------------------

int xgff_getBlockWrInfo(xgff_t *fifo, byte **ptr, size_t *len)
{
	xgff_check_fifo_ptr(fifo);
	if (len == NULL)
		return -1;

	pthread_mutex_lock(&fifo->mutex);
	*ptr = fifo->data + shadow_in;
	if (shadow_in > shadow_out)
		*len = fifo->size - shadow_in;
	else if (shadow_in < shadow_out)
		*len = shadow_out - shadow_in;
	else if (fifo->in == fifo->out) // fifo empty
		*len = fifo->size - shadow_in;
	else // shadow_in == shadow_out, fifo full
		*len = 0;
}

int xgff_getBlockRdInfo(xgff_t *fifo, byte **ptr, size_t *len)
{
	xgff_check_fifo_ptr(fifo);
	if (len == NULL)
		return -1;

	pthread_mutex_lock(&fifo->mutex);
	*ptr = fifo->data + shadow_out;
	if (shadow_in > shadow_out)
		*len = shadow_out - shadow_in;
	else if (shadow_in < shadow_out)
		*len = fifo->size - shadow_out;
	else if (fifo->in == fifo->out) // fifo empty
		*len = 0;
	else // shadow_in == shadow_out, fifo full
		*len = fifo->size - shadow_out;
}

int xgff_ackBlockWr(xgff_t *fifo, size_t len)
{
	xgff_check_fifo_ptr(fifo);

	fifo->in += len;
	pthread_mutex_unlock(&fifo->mutex);
}

int xgff_ackBlockRd(xgff_t *fifo, size_t len)
{
	xgff_check_fifo_ptr(fifo);

	fifo->out += len;
	pthread_mutex_unlock(&fifo->mutex);
}

//------------------------------------------------------------------------------

/**
 * 	@ret 1 has been 3/4 full
 *		 0 has not been 3/4 full
 */
int xgff_34full(xgff_t *fifo, bool do_print)
{
	unsigned int left_len;

	pthread_mutex_lock(&fifo->mutex);
	left_len = fifo->size - (fifo->in - fifo->out);
	pthread_mutex_unlock(&fifo->mutex);

	if (left_len < ((fifo->size) >> 2))
	{
		xgff_cprint(do_print, "75%%\n");
		return 1;
	}
	else if (left_len <= ((fifo->size) >> 1))
	{
		xgff_cprint(do_print, "50%%\n");
		return 0;
	}
}
