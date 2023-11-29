#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xfifo.h"

#define min(a,b) ((a)<(b))?(a):(b) 

int xfifo_init(struct xfifo *fifo,unsigned int size)
{
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = size;
	fifo->mask = size-1;
	fifo->data = malloc(size);
	fifo->ptr_i = fifo->data;
	fifo->ptr_o = fifo->data;
	fifo->len_i = size-1;
	fifo->len_o = 0;
	if(pthread_mutex_init(&(fifo->mutex),NULL) != 0 ){  
        printf("Init mutex fail\n");  
        exit(1);  
    }
}

int xfifo_free(struct xfifo *fifo)
{
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = 0;
	fifo->mask = 0;
	free(fifo->data);
	fifo->data = 0;
}

int xfifo_empty(struct xfifo *fifo)
{
	pthread_mutex_lock(&(fifo->mutex));
	fifo->in = 0;
	fifo->out = 0;
	pthread_mutex_unlock(&(fifo->mutex));
	
	return 0;
}

int xfifo_put(struct xfifo *fifo,const void* buf,unsigned int len)
{
	int r;
	r = pthread_mutex_trylock(&(fifo->mutex));//使用这个锁来优先保证写操作
	
	unsigned int l;
	
	len = min(len,fifo->size-(fifo->in-fifo->out));
	
	/* first put the data starting from fifo->in to buffer end */  
	l = min(len, fifo->size - (fifo->in&fifo->mask));
	memcpy(fifo->data+(fifo->in&fifo->mask), buf, l);
	
	/* then put the rest (if any) at the beginning of the buffer */ 
	memcpy(fifo->data, buf + l, len - l);
	
	if(r)
	{
		// printf("xfifo_put locked\n");
		pthread_mutex_lock(&(fifo->mutex));
	}
	fifo->in += len;
	
	// len = fifo->in - fifo->out;
	// printf("fifo len: %d, fifo_in = %u, fifo_out = %u\n",len,fifo->in,fifo->out);
	
	pthread_mutex_unlock(&(fifo->mutex));
	
	return len;
}

int xfifo_get(struct xfifo *fifo,void* buf,unsigned int len)
{
	pthread_mutex_lock(&(fifo->mutex));
	unsigned int l;
	
	// 这段操作风险很大，如果buf是数组或者字符串，倒是OK
	// 但是如果是一个指向结构体的指针，这个操作会破坏堆栈
	// char* pch = (char*)buf+len;
	// *pch = '\0';
	
	len = min(len,fifo->in-fifo->out);
	
	/* first get the data from fifo->out until the end of the buffer */     
    l = min(len, fifo->size - (fifo->out&fifo->mask));     
    memcpy(buf, fifo->data + (fifo->out&fifo->mask), l);     
    
    /* then get the rest (if any) from the beginning of the buffer */     
    memcpy(buf + l, fifo->data, len - l); 

	fifo->out += len;
    
	pthread_mutex_unlock(&(fifo->mutex));
    return len;
}

int xfifo_getlen(struct xfifo *fifo)
{
	unsigned int len;
	
	pthread_mutex_lock(&(fifo->mutex));
	len = fifo->in - fifo->out;
	pthread_mutex_unlock(&(fifo->mutex));

	return len;
}

int xfifo_getleftlen(struct xfifo *fifo)
{
	unsigned int left_len;
	
	pthread_mutex_lock(&(fifo->mutex));
	left_len = fifo->mask-(fifo->in-fifo->out);
	pthread_mutex_unlock(&(fifo->mutex));
	
	return left_len;
}

// return
//	1 has been 3/4 full
//	0 has not been 3/4 full
int xfifo_34full(struct xfifo *fifo,int do_print)
{
	unsigned int left_len;
	unsigned int len;

	pthread_mutex_lock(&(fifo->mutex));
	len = fifo->in-fifo->out;
	left_len = fifo->size-(fifo->in-fifo->out);
	pthread_mutex_unlock(&(fifo->mutex));
	
	if(do_print)
	{
		if(left_len<((fifo->size)>>2))
			printf("75%%\n");
		else if(len>=((fifo->size)>>1))
			printf("50%%\n");
	}

	if(left_len<((fifo->size)>>2))
		return 1;
	else
		return 0;
}

static void refresh_len(struct xfifo *fifo,unsigned int in,unsigned int out)
{
	if(in>=out)
	{
		fifo->len_i = fifo->size - in;//Todo有重合的可能，潜在隐患
		fifo->len_o = in - out;
	}
	else
	{
		fifo->len_i = out - in - 1;
		fifo->len_o = fifo->size - out;
	}
}

/*
	对xfifo的写入是外部直接通过ptr_i指针来操作的
	本函数需要紧接着外部写入代码调用，操作内部的ptr_i、len_i、len_o，作为写入的确认
*/
void xfifo_wr_ack(struct xfifo *fifo,unsigned int offset)
{
	pthread_mutex_lock(&(fifo->mutex));
	fifo->in += offset;
	unsigned int out = fifo->out&fifo->mask;
	unsigned int in = fifo->in&fifo->mask;
	fifo->ptr_i = fifo->data + in;
	refresh_len(fifo,in,out);
	pthread_mutex_unlock(&(fifo->mutex));

	// printf("fin: %d,fout: %d,in: %d,out: %d,len_i: %d,len_o: %d\n",\
	//         fifo->in,fifo->out,in,out,fifo->len_i,fifo->len_o);
}

/*
	对xfifo的读取是外部直接通过ptr_o指针来操作的
	本函数需要紧接着外部读取代码调用，操作内部的ptr_o、len_i、len_o，作为读取的确认
*/
void xfifo_rd_ack(struct xfifo *fifo,unsigned int offset)
{
	pthread_mutex_lock(&(fifo->mutex));
	fifo->out += offset;
	unsigned int out = fifo->out&fifo->mask;
	unsigned int in = fifo->in&fifo->mask;
	fifo->ptr_o = fifo->data + out;
	refresh_len(fifo,in,out);
	pthread_mutex_unlock(&(fifo->mutex));
}

