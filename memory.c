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


#include "memory.h"
#include "error.h"
#include <sys/times.h>
#include <pthread.h>

//#define  VXWORKS 1
#define LINUX 1
#define DEBUG 1

/* 内存管理系统基地址 */
#define MEMORY_BASE_ADDR 0x01200000

/* 根据内存块大小分配内存池  */
static memory_pool_desc mem_pool[BLK_MEM_TYPE_BUTT];
/* 内存管理块表  */
static memory_block_desc *memory_block_table[BLK_MEM_TYPE_BUTT];

/* 内存使用统计  */
static mem_pool_statics mem_pool_stat[BLK_MEM_TYPE_BUTT];

/* 块描述符基址 */
static unsigned int block_desc_base_addr = 0;
static unsigned int block_desc_end_addr = 0;

/* 内存池基址 */
static unsigned int pool_desc_base_addr = 0;
static unsigned int pool_desc_end_addr = 0;

/*  内存池初始化完成标志*/
static unsigned char mem_block_init_ok = FALSE;

/* 通过系统malloc 与free 的次数*/
static unsigned int system_malloc_num = 0;
static unsigned int system_free_num = 0;

#if DEBUG
	#define ASSERT(x) do {\
						if(!(x))\
							{\
								printf("ASSERT:%s, file:%s, line:%d\r\n", #x, __FILE__, __LINE__);\
							}\
						}while(0)
#else
	#define ASSERT(x)
#endif

/* 块内存进入临界区标记, 用于块内存重入告警 */
#ifdef VXWORKS
static unsigned char mem_block_key;
static volatile unsigned char mem_block_enter_critical = FALSE;

#define BLK_MEM_ENTER_CRITICAL_SECTION() \
{ \
	(void)taskLock(); \
	mem_block_key = intLock(); \
	ASSERT( FALSE == mem_block_enter_critical ); \
	mem_block_enter_critical = TRUE; \
}

#define BLK_MEM_LEAVE_CRITICAL_SECTION() \
{ \
	mem_block_enter_critical = FALSE; \
	(void)intUnlock ( mem_block_key ); \
	(void)taskUnlock(); \
}

#define BLK_MEM_IN_CRITICAL_SECTION()       ( FALSE )

#endif

#ifdef LINUX
pthread_mutex_t mutex;
#define BLK_MEM_ENTER_CRITICAL_SECTION()    pthread_mutex_lock(&mutex);
#define BLK_MEM_LEAVE_CRITICAL_SECTION()    pthread_mutex_unlock(&mutex);
#endif

#define MEM_IN_POOL(addr) ((unsigned int)addr >= pool_desc_base_addr && (unsigned int)addr < pool_desc_end_addr)

#if DEBUG
    #define DBG_PRINT(format, ...) do{\
									printf("FILE:%s, LINE:%d\r\n", __FILE__, __LINE__);\
								   	printf(format, ##__VA_ARGS__);\
								   	printf("\r\n");\
								   }while(0)
#else
	#define DBG_PRINT(format, ...) 
#endif



void mem_pool_init()
{
	unsigned int block_user_size		= 0;
	unsigned int block_type				= 0;
	unsigned int block_num				= 0;
	unsigned int block_index			= 0;
	unsigned int total_pool_size		= 0;
	unsigned int total_block_desc_size	= 0;
	unsigned int total_size				= 0;
	unsigned char *memory_base_addr		= NULL;
	unsigned char *memory_end_addr		= NULL;
	unsigned char *block_desc_addr		= NULL;
	unsigned char *pool_desc_addr			= NULL;
	block_head *blk_head				= NULL;
	block_tail *blk_tail				= NULL;

	/*  初始化每个内存池中的内存块数  */
	mem_pool[BLK_MEM_TYPE_32].block_num		= 5000;
	mem_pool[BLK_MEM_TYPE_64].block_num		= 20;
	mem_pool[BLK_MEM_TYPE_128].block_num	= 40;
	mem_pool[BLK_MEM_TYPE_256].block_num	= 20;
    mem_pool[BLK_MEM_TYPE_512].block_num	= 10;
	mem_pool[BLK_MEM_TYPE_1024].block_num	= 10;
	mem_pool[BLK_MEM_TYPE_2048].block_num	= 10;
	mem_pool[BLK_MEM_TYPE_4096].block_num	= 5000;
	mem_pool[BLK_MEM_TYPE_8192].block_num	= 10;

	mem_block_init_ok = FALSE;

	block_user_size			= 32;
	total_pool_size			= 0;
	total_block_desc_size	= 0;

	for (block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		/*  初始化每个内存池的大小  */
		mem_pool[block_type].block_user_size = block_user_size;
		mem_pool[block_type].block_total_size = BLOCK_HEAD_SIZE + block_user_size + BLOCK_TAIL_SIZE;
		mem_pool[block_type].pool_size = mem_pool[block_type].block_total_size * mem_pool[block_type].block_num;

		total_block_desc_size += mem_pool[block_type].block_num * sizeof(memory_block_desc);
		total_pool_size += mem_pool[block_type].pool_size;
		block_user_size <<= 1;
	}

	total_size = total_block_desc_size + total_pool_size + MEMORY_ALLOC_RESV;
	/*  为整个内存池分配空间  */
#ifdef VXWORKS
	memory_base_addr	= MEMORY_BASE_ADDR;
#else
    pthread_mutex_init(&mutex, NULL);
    memory_base_addr = (unsigned char*)malloc(total_size);
#endif
	memory_end_addr	= memory_base_addr + total_size;

	ASSERT(NULL != memory_base_addr);

	memory_base_addr = ADDR_ALIGN_UP(memory_base_addr, 0x1000);

	/*  初始化内存块描述符地址  */
	block_desc_base_addr = memory_base_addr;
	block_desc_base_addr = ADDR_ALIGN_UP(block_desc_base_addr, 0x100);
	/*  初始化内存池描述符地址  */
	pool_desc_base_addr	= memory_base_addr + total_block_desc_size + MEMORY_ALLOC_RESV / 2;
	pool_desc_base_addr = ADDR_ALIGN_UP(pool_desc_base_addr, 0x100);

	block_desc_addr = block_desc_base_addr;
	pool_desc_addr = pool_desc_base_addr;
	
	for (block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		memory_block_table[block_type] = (memory_block_desc *)block_desc_addr;

		mem_pool[block_type].base_addr	= pool_desc_addr;
		mem_pool[block_type].end_addr	= pool_desc_addr + mem_pool[block_type].pool_size;
		/*  初始化不同大小内存块描述符的起始地址  */
		block_desc_addr += mem_pool[block_type].block_num * sizeof(memory_block_desc);
		block_desc_addr = ADDR_ALIGN_UP(block_desc_addr, 0x100);
		/*  初始化不同大小内存块内存池的起始地址  */
		pool_desc_addr = mem_pool[block_type].end_addr;
		pool_desc_addr = ADDR_ALIGN_UP(pool_desc_addr, 0x100);

		if(pool_desc_addr > memory_base_addr + total_size)
		{
			DBG_PRINT("init error");
			return;
		}
	}
	block_desc_end_addr = (unsigned int)memory_block_table[BLK_MEM_TYPE_BUTT - 1] + \
		mem_pool[BLK_MEM_TYPE_BUTT - 1].block_num * sizeof(memory_block_desc);
	pool_desc_end_addr = mem_pool[BLK_MEM_TYPE_BUTT - 1].end_addr; 
	/*  清空内存块描述符的内容  */
	memset(block_desc_base_addr, 0, block_desc_end_addr - block_desc_base_addr);
	/*  将内存池中的的内容置非法值便于调试  */
	memset(pool_desc_base_addr, MEM_INVALID_BYTE, pool_desc_end_addr - pool_desc_base_addr);

	/*  初始化内存描述块 */
	for (block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		block_num = mem_pool[block_type].block_num;
		for (block_index = 0; block_index < block_num - 1; block_index++)
		{
			memory_block_table[block_type][block_index].idle_next = block_index + 1;
		}
		memory_block_table[block_type][block_num - 1].idle_next = BLK_MEM_INDEX_END;

		mem_pool[block_type].idle_list_head = 0;
		mem_pool[block_type].idle_list_tail = block_num - 1;
		mem_pool[block_type].idle_block_num = block_num;

	}
	/*  内存块头部与尾部初始化  */
	for (block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		block_num = mem_pool[block_type].block_num;
		for (block_index = 0; block_index < block_num; block_index++)
		{
			blk_head = (block_head *)(mem_pool[block_type].base_addr + block_index * mem_pool[block_type].block_total_size);
			blk_tail = (block_tail *)((unsigned char *)blk_head + BLOCK_HEAD_SIZE + mem_pool[block_type].block_user_size);

			blk_head->head_magic_number = BLK_HEAD_MAGIC;
			blk_head->index				= block_index;
			blk_head->type				= block_type;
			blk_head->resv				= MEM_INVALID_BYTE;

			blk_tail->tail_magic_number = BLK_TAIL_MAGIC;
		}
	}
    mem_stat_init();
	mem_block_init_ok = TRUE;
}



void *mem_alloc(unsigned int size, unsigned int alloc_mod)
{
	unsigned int			block_type	= 0;
	unsigned int			tmp_size	= 0;
	unsigned int			block_index	= 0;
	unsigned int			block_addr  = NULL;
	memory_pool_desc		*pool_desc	= NULL;
	memory_block_desc		*block_desc	= NULL;
	block_head				*blk_head	= NULL;
	block_tail				*blk_tail	= NULL;
	void					*ret_block	= NULL;


	if (FALSE == mem_block_init_ok)
	{
		DBG_PRINT("mem pool init failed...\r\n");
		return NULL;
	}

	BLK_MEM_ENTER_CRITICAL_SECTION();
	if (size <= 0)
	{
		DBG_PRINT("size out of range...\r\n");
		BLK_MEM_LEAVE_CRITICAL_SECTION();
		return NULL;
	}
	else if(size > 8192)
	{
		DBG_PRINT("MOD %d alloced %d mem in system heap\r\n", alloc_mod, size);
		system_malloc_num++;
		BLK_MEM_LEAVE_CRITICAL_SECTION();
		return malloc(size);
	}
	
	tmp_size = (size - 1) >> 4;
	for (block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		tmp_size >>= 1;
		if (!tmp_size)
			break;
	}
	/* 块内存类型检查 */
	if(BLK_MEM_TYPE_BUTT <= block_type)
	{
		DBG_PRINT("alloc error...\r\n");
		BLK_MEM_LEAVE_CRITICAL_SECTION();
		return NULL;
	}

	pool_desc = &mem_pool[block_type];
	/* 该大小的内存池中内存块已分配完 */
	if(0 == pool_desc->idle_block_num)
	{
		ASSERT(BLK_MEM_INDEX_NULL == pool_desc->idle_list_head);
		ASSERT(BLK_MEM_INDEX_NULL == pool_desc->idle_list_tail);

		DBG_PRINT("size [%d] out of block...\n", block_type);

		/* 从其高一级的内存池中分配 */
		if (size <= 4096 && size > 0)
		{
			block_type++;
			if (block_type >= BLK_MEM_TYPE_BUTT || 0 == mem_pool[block_type].idle_block_num)
			{
				DBG_PRINT("size [%d] out of block...\n", block_type);
				BLK_MEM_LEAVE_CRITICAL_SECTION();
                mem_pool_stat[block_type].alloc_failed++;
				return NULL;
			}
			pool_desc = &mem_pool[block_type];
		}
        else
        {
            BLK_MEM_LEAVE_CRITICAL_SECTION();
            mem_pool_stat[block_type].alloc_failed++;
            return NULL;
        }
	}
	else
	{
		ASSERT(BLK_MEM_INDEX_NULL != pool_desc->idle_list_head);
		ASSERT(BLK_MEM_INDEX_NULL != pool_desc->idle_list_tail);
		ASSERT(pool_desc->block_num > pool_desc->idle_list_head);
		ASSERT(pool_desc->block_num > pool_desc->idle_list_tail);
	}

	/* 从空闲链表的头部取内存块 */
	block_index = pool_desc->idle_list_head;
	ASSERT(block_index < pool_desc->block_num);

	//pool_desc->idle_list_head = &memory_block_table[block_type][block_index];

	/* 获取指向内存块描述结构的指针 */
	block_desc = &memory_block_table[block_type][block_index];

	ASSERT(FALSE == block_desc->used);
	ASSERT(FALSE == block_desc->leak);

	block_addr = pool_desc->base_addr + pool_desc->block_total_size * block_index;

	if(TRUE == block_desc->error)
	{
		/* 如果内存块头部越界, 重新初始化内存块头部 */
		if (TRUE == block_desc->head_over)
		{
			blk_head = (block_head *)block_addr;

			blk_head->head_magic_number = BLK_HEAD_MAGIC;
			blk_head->index				= block_index;
			blk_head->type				= block_type;
			blk_head->resv				= MEM_INVALID_BYTE;
		}

		if (TRUE == block_desc->tail_over)
		{
			blk_tail =(block_tail *)(block_addr + BLOCK_HEAD_SIZE + mem_pool[block_type].block_user_size);

			blk_tail->tail_magic_number = BLK_TAIL_MAGIC;
		}

		block_desc->error		= FALSE;
		block_desc->head_over	= FALSE;
		block_desc->tail_over	= FALSE;
		block_desc->leak		= FALSE;
	}
	else
	{
		ASSERT(FALSE == block_desc->head_over);
		ASSERT(FALSE == block_desc->tail_over);
		ASSERT(FALSE == block_desc->leak);
	}

	/* 设置块内存描述结构 */
	block_desc->used = TRUE;
	block_desc->alloc_mod = alloc_mod;
#if LINUX 
	block_desc->alloc_tid = 1; ///////////////////////////////////////////
#elif VXWORKS
    block_desc->alloc_tid = 0;//taskIdSelf();
#endif
	block_desc->free_tid = NULL;
	block_desc->alloc_time = time(NULL);

	/******************************************************************/
	/*              设置内存池描述结构中的内存块使用计数              */
	/******************************************************************/

	pool_desc->idle_block_num--;
    pool_desc->busy_block_num++;
	ASSERT(pool_desc->idle_block_num <= pool_desc->block_num);

	/******************************************************************/
	/*                      记录统计信息                              */
	/******************************************************************/
	//mstPoolStat[ulBlkType].ulPidAllocNum[ulPid]++;
	/******************************************************************/
	/*                  从空闲链表的头部摘取内存块                    */
	/******************************************************************/
	/* 如果空闲链表只剩下一个元素 */
	if(pool_desc->idle_list_head == pool_desc->idle_list_tail)
	{
		ASSERT(BLK_MEM_INDEX_END == block_desc->idle_next);
		ASSERT(0 == pool_desc->idle_block_num);

		pool_desc->idle_list_head = pool_desc->idle_list_tail = BLK_MEM_INDEX_NULL;

	}
	else
	{
		ASSERT(BLK_MEM_INDEX_NULL != block_desc->idle_next);
		ASSERT(BLK_MEM_INDEX_END != block_desc->idle_next);
		ASSERT(pool_desc->block_num > block_desc->idle_next);
		ASSERT(0 != pool_desc->idle_block_num);

		pool_desc->idle_list_head = block_desc->idle_next;

	}
	block_desc->idle_next = BLK_MEM_INDEX_NULL;

	/* 实际返回给用户的指针 */
	ret_block = (void *)(block_addr + BLOCK_HEAD_SIZE);

	BLK_MEM_LEAVE_CRITICAL_SECTION();

	return ret_block;
}
 
void mem_free(unsigned char *mem_addr)
{
	unsigned int		tid;
	unsigned int		blk_index;
	unsigned int		blk_type;
	unsigned char		head_over;
	unsigned char		tail_over;
	block_head			*blk_head;
    block_tail          *blk_tail;
	int					ret_val;
	int					ret_val_idx;

    memory_pool_desc	*pool_desc;
	memory_block_desc	*block_desc;

    if (NULL == mem_addr)
    {
        DBG_PRINT("free address invalid\r\n");
        return;
    }
	if(!MEM_IN_POOL(mem_addr))
	{
		system_free_num++;
		free(mem_addr);
		return;
	}
    ASSERT(TRUE == mem_block_init_ok);

	blk_head = (block_head *)(mem_addr - BLOCK_HEAD_SIZE);
	/* 获取当前的TID */
#ifdef VXWORKS
    tid = taskIdSelf();
#elif LINUX
    tid = 2;
#endif
	if (tid <= 0)
	{
		DBG_PRINT("[mem_free]tid invalid...\r\n");
		ASSERT(1);
	}

	head_over = FALSE;
	tail_over = FALSE;
	/******************************************************************/
	/*                  头尾越界检查                                  */
	/******************************************************************/
	BLK_MEM_ENTER_CRITICAL_SECTION();
	/* 检查内存块头部信息是否正确 */
	ret_val = blk_mem_head_check(blk_head);

    /* 内存块头部信息非法 */
	if(SUCCESS_NO_ERROR != ret_val)
	{
		/* 启动复杂地址计算, 获取内存块类型和在内存池中的索引 */
		ret_val_idx = blk_mem_type_idx_check((unsigned int)blk_head, &blk_type, &blk_index);
		if(SUCCESS_NO_ERROR != ret_val_idx)
		{
            DBG_PRINT("free address invalid.\r\n");
			BLK_MEM_LEAVE_CRITICAL_SECTION();
			return;
		}
		else
        {
			head_over = TRUE;
            DBG_PRINT("head over at tid: 0x%lx.\r\n", memory_block_table[blk_type][blk_index].alloc_tid);
        }

	}
	else
	{
		blk_index = blk_head->index;
		blk_type = blk_head->type;
	}
	
	/* 确定内存池描述结构和内存块描述结构 */
    pool_desc = &mem_pool[blk_type];
    block_desc = &memory_block_table[blk_type][blk_index];

    /* 检查内存块尾部信息是否正确 */
    blk_tail = (block_tail *)((unsigned char *)blk_head + BLOCK_HEAD_SIZE + pool_desc->block_user_size);
    
    ret_val = blk_mem_tail_check(blk_tail);
    
    /* 内存块尾部被改写了 */
    if( SUCCESS_NO_ERROR != ret_val )
    {
        tail_over = TRUE;
        DBG_PRINT("tail over at tid: 0x%lx.\r\n", memory_block_table[blk_type][blk_index].alloc_tid);
    }
    /******************************************************************/
    /*                  重复释放检查                                  */
    /******************************************************************/

    /* 如果块内存重复释放 */
    if( TRUE != block_desc->used && TRUE != head_over)
    {
        BLK_MEM_LEAVE_CRITICAL_SECTION();
        return;
    }
    /******************************************************************/
    /*                  设置内存块的描述结构                          */
    /******************************************************************/
    block_desc->used = FALSE;
    block_desc->leak = FALSE;

#if LINUX
    block_desc->free_tid = 2;
#elif VXWORKS
    block_desc->alloc_tid = 0;
#endif

    /* 如果内存块头部或尾部被改写, 设置错误标记位 */
    if( ( TRUE == head_over ) || ( TRUE == tail_over ) )
    {
        /* 设置内存块描述结构中的错误标记 */
        block_desc->error      = TRUE;
        block_desc->head_over  = ( TRUE == head_over ) ? TRUE : FALSE;
        block_desc->tail_over  = ( TRUE == tail_over ) ? TRUE : FALSE;
    }


    /******************************************************************/
    /*                  将内存块挂接到空闲链表尾部                    */
    /******************************************************************/
    /* 如果空闲链表为空 */
    if( BLK_MEM_INDEX_NULL == pool_desc->idle_list_head )
    {
        ASSERT(BLK_MEM_INDEX_NULL == pool_desc->idle_list_tail);
        ASSERT(0 == pool_desc->idle_block_num);

        pool_desc->idle_list_head = pool_desc->idle_list_tail = blk_index;
    }
    /* 如果空闲链表非空 */
    else
    {
        ASSERT(0 < pool_desc->idle_block_num);
        ASSERT(pool_desc->block_num > pool_desc->idle_block_num);
        ASSERT(pool_desc->block_num > pool_desc->idle_list_tail);
        ASSERT(pool_desc->block_num > pool_desc->idle_list_head);
        ASSERT(BLK_MEM_INDEX_NULL == block_desc->idle_next);
        ASSERT(BLK_MEM_INDEX_END  == memory_block_table[blk_type][pool_desc->idle_list_tail].idle_next);

        memory_block_table[blk_type][pool_desc->idle_list_tail].idle_next = blk_index;

        pool_desc->idle_list_tail = blk_index;
    }

    block_desc->idle_next = BLK_MEM_INDEX_END;

    pool_desc->idle_block_num++;
    pool_desc->busy_block_num--;


    BLK_MEM_LEAVE_CRITICAL_SECTION();

}


int blk_mem_head_check(block_head *blk_head)
{
	unsigned int block_type;
	unsigned int block_index;
	memory_pool_desc *pool_desc;


	ASSERT(blk_head >= pool_desc_base_addr);
	ASSERT(blk_head < pool_desc_end_addr);
    /* 内存块的头部越界标记是否正确 */
	if(BLK_HEAD_MAGIC != blk_head->head_magic_number)
	{
		DBG_PRINT("[blk_mem_head_check]block_head_flag_invalid.\r\n");
		return ERR_OS_BLK_MEM_HEAD_FLAG_INVALID;
	}

	block_type = blk_head->type;
	block_index = blk_head->index;

	/* 判断内存块头部的块内存类型是否合法 */
	if(block_type > BLK_MEM_TYPE_BUTT)
	{
		DBG_PRINT("[blk_mem_head_check]block_type_invalid.\r\n");
		return ERR_OS_BLK_MEM_HEAD_TYPE_INVALID;
	}

	pool_desc = &mem_pool[block_type];

	/* 判断内存块头部的内存块索引是否合法 */
	if(block_index > pool_desc->block_num)
		return ERR_OS_BLK_MEM_HEAD_INDEX_TOO_BIG;

	/* 判断内存块头部地址是否对齐 */
	if(0 != ((unsigned int)blk_head - pool_desc->base_addr) % pool_desc->block_total_size)
		return ERR_OS_BLK_MEM_HEAD_ADDR_NOT_ALIGN;

	/* 判断内存块头部的内存块索引是否合法 */
	if(block_index != ((unsigned int)blk_head - pool_desc->base_addr) / pool_desc->block_total_size)
		return ERR_OS_BLK_MEM_HEAD_INDEX_INVALID;

	return SUCCESS_NO_ERROR;

}

int blk_mem_tail_check(block_tail *blk_tail)
{
	ASSERT(blk_tail > pool_desc_base_addr);
	ASSERT(blk_tail < pool_desc_end_addr);

	if(BLK_TAIL_MAGIC != blk_tail->tail_magic_number)
	{
		DBG_PRINT("[blk_mem_tail_check]block_tail_flag_invalid.\r\n");
		return ERR_OS_BLK_MEM_TAIL_FLAG_INVALID;
	}
	return SUCCESS_NO_ERROR;
}

int blk_mem_type_idx_check(unsigned int blk_head, unsigned int *blk_type, unsigned int *blk_index)
{
	unsigned int block_type;
	memory_pool_desc *pool_desc;
	/* 入口指针检查 */
	if( NULL == *blk_type || NULL == *blk_index )
		return ERR_OS_BLK_MEM_PTR_NULL;

	/* 判断地址是否在块内存的范围内 */
	if( ( blk_head < pool_desc_base_addr ) || ( blk_head >= pool_desc_end_addr ) )
		return ERR_OS_BLK_MEM_ADDR_NOT_IN_POOL;

	/* 判断地址落在哪种类型的内存池内 */
	for( block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++ )
	{
		pool_desc = &mem_pool[block_type];

		ASSERT( pool_desc->end_addr == ( pool_desc->base_addr + pool_desc->pool_size ) );
		ASSERT( pool_desc->base_addr >= pool_desc_base_addr );
		ASSERT( pool_desc->end_addr  <= pool_desc_end_addr );

		if((blk_head >= pool_desc->base_addr) && (blk_head <  pool_desc->end_addr))
			break;
	}

	/* 没有找到相应的块内存类型 */
	if( BLK_MEM_TYPE_BUTT <= block_type )
		return ERR_OS_BLK_MEM_TYPE_INVALID;

	pool_desc = &mem_pool[block_type];

	*blk_index = (blk_head - pool_desc_base_addr) / pool_desc->block_total_size;
	*blk_type = block_type;

	/* 判断内存块头部地址是否对齐 */
	if(0 != ((blk_head - pool_desc->base_addr) % pool_desc->block_total_size))
		return ERR_OS_BLK_MEM_HEAD_ADDR_NOT_ALIGN;
	
	return SUCCESS_NO_ERROR;
}

void mem_stat_init()
{
    unsigned int blk_type;
    
    for (blk_type = 0; blk_type < BLK_MEM_TYPE_BUTT; blk_type++)
    {
        mem_pool_stat[blk_type].total_blocks = mem_pool[blk_type].block_num;
        mem_pool_stat[blk_type].idle_blcoks = mem_pool[blk_type].block_num;
        mem_pool_stat[blk_type].busy_blocks = 0;
        mem_pool_stat[blk_type].alloc_failed = 0;
    }
    
}
void *mem_realloc(unsigned char *addr, unsigned int size, unsigned int alloc_mod)
{
    unsigned int        ret_val;
    unsigned int        blk_type;
    unsigned int        blk_index;
    block_head          *blk_head;
    void                *ret_addr;
    memory_pool_desc    *pool_desc;

    BLK_MEM_ENTER_CRITICAL_SECTION();
    /* 空指针判断  */
    if (NULL  == addr)
    {
        DBG_PRINT("[realloc]invalid pointer.\r\n");
        ret_addr = NULL;
    }
    else
    {
        blk_head = (block_head *)((unsigned char *)addr - BLOCK_HEAD_SIZE);
        /* 检查输入指针是否在内存池内  */
        ret_val = blk_mem_type_idx_check((unsigned int)blk_head, &blk_type, &blk_index);
        if(SUCCESS_NO_ERROR != ret_val)
        {
            DBG_PRINT("realloc address invalid.\r\n");
            ret_addr = NULL;
        }
        else
        {
            pool_desc = &mem_pool[blk_type];
            /* 如果内存块空间还够，则返回当前指针 */
            if (size <= pool_desc->block_user_size)
            {
                ret_addr = addr;
            }
            /* 如果内存块空间不够，则重新申请新内存块，拷贝原内存块内容，并释放原内存块 */
            else
            {
                ret_addr = mem_alloc(size, alloc_mod);
                memcpy(ret_addr, addr, pool_desc->block_user_size);
                mem_free(addr);
            }

        }
    }

    BLK_MEM_LEAVE_CRITICAL_SECTION();
    return ret_addr;
}


void show_mem_stat()
{
    unsigned int blk_type;
		printf("[mem_pool_stat]: system malloc: %d    free: %d \r\n", system_malloc_num, system_free_num);
        printf("[mem_pool_stat]: block_type    busy_blocks    idle_blocks\r\n");
    for (blk_type = 0; blk_type < BLK_MEM_TYPE_BUTT; blk_type++)
    {        
        printf("                    %d             %d             %d\r\n",\
            blk_type, mem_pool[blk_type].busy_block_num, mem_pool[blk_type].idle_block_num);
    }
}

void mem_check()
{
	unsigned int block_type;
	unsigned int block_index;
	unsigned char *block_addr;
	memory_pool_desc *pool_desc;
	memory_block_desc *block_desc;
	int error_no = 0;


	

	printf("cheking memory...\r\n");
	for(block_type = 0; block_type < BLK_MEM_TYPE_BUTT; block_type++)
	{
		printf("block type:%d\r\n", block_type);
		pool_desc = &mem_pool[block_type];
		for(block_index = 0; block_index < pool_desc->block_num; block_index++)
		{
			block_desc = &memory_block_table[block_type][block_index];
			if(block_desc->error || block_desc->head_over || block_desc->leak)
				DBG_PRINT("block_desc:%d error:error:%d, head_over:%d, leak:%d.\r\n", block_index, block_desc->error, block_desc->head_over, block_desc->leak);	
			
			block_addr = pool_desc->base_addr + pool_desc->block_total_size * block_index;
			error_no = blk_mem_head_check((block_head *)block_addr);
			if(error_no)
				DBG_PRINT("head over write.index:%d,tid:%lx,mod_id:%d.\r\n", block_index, block_desc->alloc_tid, block_desc->alloc_mod);	

			error_no = blk_mem_tail_check((block_tail *)(block_addr + BLOCK_HEAD_SIZE + pool_desc->block_user_size));
			if(error_no)
				DBG_PRINT("tail over write.index:%d,tid:%lx,mod_id:%d.\r\n", block_index, block_desc->alloc_tid, block_desc->alloc_mod);
				
			
		}
		printf("checked %d blocks\r\n", pool_desc->block_num);

	}
	printf("memory check finished.\r\n");

}


void mem_efficiency_test(int test_second, int test_count)
{
    unsigned char   *alloc_addr[100];
    unsigned int    alloc_size[BLK_MEM_TYPE_BUTT];
    unsigned int    blk_type;
    unsigned int    count;
    unsigned int    op_count;
    clock_t         start_time, current_time;

    for (blk_type = 0; blk_type < BLK_MEM_TYPE_BUTT; blk_type++)
    {
        alloc_size[blk_type] = mem_pool[blk_type].block_user_size - 5;
    }

    start_time = clock();
    op_count = 0;
    for(;;)
    {
        for (blk_type = 0; blk_type < BLK_MEM_TYPE_BUTT; blk_type++)
        {
            for (count = 0; count < test_count; count++)
            {
                alloc_addr[count] = (unsigned char *)mem_alloc(alloc_size[blk_type], MEM_TYPE_BLOCK);
            }

            for (count = 0; count < test_count; count++)
            {
                mem_free(alloc_addr[count]);
            }

            op_count += test_count;

            current_time = clock();

            if((current_time - start_time) / 1000 == test_second)
                goto report;
        }
    }

report:
    printf("\r\n[block memory alloc free efficiency test]:\r\n");
    printf("%d seconds finished %d alloc/free oprations\r\n", test_second, op_count);

    return;
}



#ifdef _cplusplus
}
#endif

