/* 
* Copyright (c) 2013,北京交大微联科技有限公司杭州分公司 
* All rights reserved. 
* 
* 文件名称：lib_memory.c
* 文件标识：见配置管理计划书 
* 摘 要：内存管理模块
* 
* 当前版本：1.0
* 作 者：关一鸣 
* 完成日期：2013年8月16日 
* 
* 取代版本：
* 原作者 ：
* 完成日期：
*/



#ifdef _cplusplus
extern"C"{
#endif

#include <stdio.h>
/* For tagging memory, below is the type of the memory. */
enum
{
  MTYPE_TMP = 1,
  MTYPE_MAX
};


#define XMALLOC(size, mtype)       mem_alloc ((size), (mtype))
//#define XCALLOC(mtype, num, size)  zcalloc ((mtype), (num), (size))
#define XREALLOC(mtype, ptr, size) mem_realloc ((ptr), (size), (mtype))
#define XFREE(ptr)          mem_free ((ptr))
//#define XSTRDUP(mtype, str)        zstrdup ((mtype), (str))

void memory_init ();



/* 块内存的大小 */
#define BLK_MEM_32_SIZE                 32
#define BLK_MEM_64_SIZE                 64
#define BLK_MEM_128_SIZE                128
#define BLK_MEM_256_SIZE                256
#define BLK_MEM_512_SIZE                512
#define BLK_MEM_1024_SIZE               1024
#define BLK_MEM_2048_SIZE               2048
#define BLK_MEM_4096_SIZE               4096
#define BLK_MEM_8192_SIZE               8192


typedef enum tag_mem_block_type{
	BLK_MEM_TYPE_32 = 0,
	BLK_MEM_TYPE_64,
	BLK_MEM_TYPE_128,
	BLK_MEM_TYPE_256,
	BLK_MEM_TYPE_512,
	BLK_MEM_TYPE_1024,
	BLK_MEM_TYPE_2048,
	BLK_MEM_TYPE_4096,
	BLK_MEM_TYPE_8192,

	BLK_MEM_TYPE_BUTT

}mem_block_type;

/* 内存类型的枚举定义 */
typedef enum tag_mem_type
{
	MEM_TYPE_BLOCK,              /* 块内存类型 */
	MEM_TYPE_STATIC,             /* 静态内存类型 */

	MEM_TYPE_BUTT
}mem_type;

/* 内存块的头部和尾部越界标志 */
#define BLK_HEAD_MAGIC				0xAAAAAAAA
#define BLK_TAIL_MAGIC				0xBBBBBBBB
#define MEM_INVALID_BYTE			0xCC



/* 内存池描述符 */
typedef struct tag_memory_pool_desc{
	unsigned int base_addr;		/* memory pool 起始地址  */
	unsigned int end_addr;		/* memory pool 结束地址  */
	
	unsigned int pool_size;     /* memory pool 大小 =  block_num * block_total_size */
	unsigned int block_num;	    /* memory pool 块数量  */
	unsigned int block_user_size;	/* 内存块数据区大小  */
	unsigned int block_total_size;  /* 内存块实际大小（加上头和尾结构） */
	
	unsigned int idle_list_head;	/* 空闲链表头 */
	unsigned int idle_list_tail;	/* 空闲链表尾 */

	unsigned int busy_block_num;	/* 使用内存块数量 */
	unsigned int idle_block_num;	/* 空闲数据块数量 */
	unsigned int block_threshold;	/* 内存块使用量告警阀值 */
}memory_pool_desc;

/* 内存块描述符 */
typedef struct tag_memory_block_desc{
	unsigned int	idle_next;		/* 空闲链表下一个 */
	
	unsigned char	used:1;			/* 未使用:0, 使用:1*/
	unsigned char	alloc_mod:7;	/* 申请内存的模块 */
	

	unsigned char	error:1;		/* 正常:0,错误:1 */
	unsigned char	head_over:1;	/* 头部覆盖 */
	unsigned char	tail_over:1;	/* 尾部覆盖 */
	unsigned char	leak:1;			/* 泄露 */
	unsigned char   resv:4;

	unsigned int	alloc_tid;		/* 分配任务tid */
	unsigned int	free_tid;		/* 回收任务tid */
	unsigned int	alloc_time;		/* 分配时间 */

}memory_block_desc;

/* 内存块头部结构 */
typedef struct tag_block_head{
	unsigned int	head_magic_number;	/* 内存块越界标记 */
	unsigned short	index;				/* 内存块索引 */
	unsigned char	resv;
	unsigned char	type;				/* 内存块类型 */
}block_head;

/* 内存块尾部结构 */
typedef struct tag_block_tail{
	unsigned int	tail_magic_number;	/* 内存块越界标记 */
}block_tail;

/* 内存计数 */
typedef struct tag_mem_pool_stat{
    unsigned int    total_blocks;
    unsigned int    busy_blocks;
    unsigned int    idle_blcoks;

    unsigned int    alloc_failed;

}mem_pool_statics;

#define BLOCK_HEAD_SIZE sizeof(block_head)
#define BLOCK_TAIL_SIZE sizeof(block_tail)
#define MEMORY_ALLOC_RESV 0x10000

#define ADDR_ALIGN_UP(addr,align)       (unsigned int)(((unsigned int)addr + (unsigned int)(align - 1)) & ~(unsigned int)(align - 1))
#define ADDR_ALIGN_DOWN(addr,align)     (unsigned int)((unsigned int)addr & ~(unsigned int)(align - 1))

/* 块内存索引的非法值 */
#define BLK_MEM_INDEX_NULL              ( (unsigned int)(-1) )
#define BLK_MEM_INDEX_END               ( (unsigned int)(-1) - 0x33221100 )

#define TRUE 1
#define FALSE 0





void mem_pool_init();
void *mem_alloc(unsigned int size, unsigned int alloc_mod);
void mem_free(unsigned char *mem_addr);
void *mem_realloc(unsigned char *addr, unsigned int size, unsigned int alloc_mod);

void mem_efficiency_test(int test_second, int test_count);
void mem_stat_init();
void show_mem_stat();


#ifdef _cplusplus
}
#endif


