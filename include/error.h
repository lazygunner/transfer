#ifndef __ERR_NO_
#define __ERR_NO_

#define SUCCESS_NO_ERROR 0
#define RET_SUCCESS 0
/* 支撑模块的操作系统子模块的内部错误码 */
typedef enum tag_OS_ERR_CODE
{
	/*00*/  ERR_OS_GENERAL_FAIL = 0,

	/*01*/  ERR_OS_ROOT_INIT_FAIL,              /* root的初始化失败 */
	/*02*/  ERR_OS_MEM_INIT_MEM_TOO_SMALL,      /* 内存初始化, 分配的内存太小 */
	/*03*/  ERR_OS_MEM_ALLOC_TYPE,              /* 内存申请, 输入参数内存类型错误 */
	/*04*/  ERR_OS_MEM_ALLOC_SIZE_ZERO,         /* 内存申请, 输入参数申请内存大小为0 */
	/*05*/  ERR_OS_MEM_ALLOC_SIZE_TOO_LARGE,    /* 内存申请, 输入参数申请内存太大 */
	/*06*/  ERR_OS_MEM_ALLOC_MEM_EMPTY,         /* 内存申请, 内存已分配完, 无法再分配 */
	/*07*/  ERR_OS_MEM_FREE_ADDR_MISMATCH,      /* 内存释放, 输入参数内存地址不匹配 */
	/*08*/  ERR_OS_MEM_FREE_OVERWRITE,          /* 内存释放, 释放的内存被越界改写 */
	/*09*/  ERR_OS_MEM_REE_NOT_USED,            /* 内存释放, 释放没有申请的内存 */

	/*10*/  ERR_OS_SCHD_CREATE_QUE,             /* 调度, 创建消息队列失败 */
	/*11*/  ERR_OS_SCHD_CREATE_TASK,            /* 调度, 创建任务失败 */
	/*12*/  ERR_OS_SCHD_ALLOC_MSG,              /* 调度, 申请消息包失败 */
	/*13*/  ERR_OS_SCHD_SEND_MSG,               /* 调度, 发送消息包失败 */
	/*14*/  ERR_OS_SCHD_FREE_MSG,               /* 调度, 释放消息包失败 */

	/*xx*/  ERR_OS_BLK_MEM_INIT_FAIL,           /* 块内存, 初始化失败*/
	/*xx*/  ERR_OS_BLK_MEM_STAT_INIT_FAIL,      /* 块内存, 统计信息初始化失败*/
	/*xx*/  ERR_OS_BLK_MEM_ALLOC_SIZE_INVALID,  /* 块内存, 申请的长度非法 */
	/*xx*/  ERR_OS_BLK_MEM_PID_INVALID,         /* 块内存, PID非法 */
	/*xx*/  ERR_OS_BLK_MEM_PTR_NULL,            /* 块内存, 指针空 */
	/*xx*/  ERR_OS_BLK_MEM_USE_UP,              /* 块内存, 块内存耗尽 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INVALID,        /* 块内存, 内存块头部非法 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_FLAG_INVALID,   /* 块内存, 内存块头部越界标记非法 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_TYPE_INVALID,   /* 块内存, 内存块头部的块内存类型非法 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INDEX_TOO_BIG,  /* 块内存, 内存块头部块的内存块索引太大 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_ADDR_NOT_ALIGN, /* 块内存, 内存块头部地址不对齐 */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INDEX_INVALID,  /* 块内存, 内存块头部块的内存块索引非法 */
	/*xx*/  ERR_OS_BLK_MEM_TAIL_INVALID,        /* 块内存, 内存块尾部非法 */
	/*xx*/  ERR_OS_BLK_MEM_TAIL_FLAG_INVALID,   /* 块内存, 内存块尾部部越界标记非法 */
	/*xx*/  ERR_OS_BLK_MEM_RE_FREE,             /* 块内存, 重复释放 */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_NOT_IN_POOL,    /* 块内存, 地址不在内存池内 */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_NOT_IN_USR_BLK, /* 块内存, 地址不在内存块的用户地址范围内 */
	/*xx*/  ERR_OS_BLK_MEM_TYPE_INVALID,        /* 块内存, 块内存类型非法 */
	/*xx*/  ERR_OS_BLK_MEM_BLK_ADDR_INVALID,    /* 块内存, 内存块的地址非法 */
	/*xx*/  ERR_OS_BLK_MEM_LEAK,                /* 块内存, 内存块泄漏 */
	/*xx*/  ERR_OS_BLK_MEM_NOT_LEAK,            /* 块内存, 内存块没有泄漏 */
	/*xx*/  ERR_OS_BLK_MEM_CAL_POOL_CRC,        /* 块内存, 计算内存池的CRC错误 */
	/*xx*/  ERR_OS_BLK_MEM_POOL_CRC_INVALID,    /* 块内存, 内存池的CRC非法 */
	/*xx*/  ERR_OS_BLK_MEM_INDEX_INVALID,       /* 块内存, 内存块索引非法 */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_INVALID,    /* 块内存, 空闲队列指针非法 */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_END_TOO_MUCH,/* 块内存, 空闲队列中的结束指针太多 */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_NOT_MATCH,  /* 块内存, 空闲队列指针与使用标记不匹配 */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_NUM_INVALID,    /* 块内存, 空闲计数错误 */
	/*xx*/  ERR_OS_BLK_MEM_DYN_BUSY_NUM_INVALID,/* 块内存, 动态使用计数错误 */
	/*xx*/  ERR_OS_BLK_MEM_STA_BUSY_NUM_INVALID,/* 块内存, 静态使用计数错误 */
	/*xx*/  ERR_OS_BLK_MEM_BLK_NUM_INVALID,     /* 块内存, 块内存总计数错误 */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_QUE_INVALID,    /* 块内存, 空闲队列错误 */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_IN_BLK_HEAD,    /* 块内存, 地址在内存块头部范围内 */
	/*xx*/  ERR_OS_BLK_MEM_TERM_INVALIDE,       /* 块内存, 命令行终端无效 */
	/*xx*/  ERR_OS_BLK_MEM_COMM_FAIL_REASON_INVALID,  /* 块内存, 通用错误原因非法 */
	/*xx*/  ERR_OS_BLK_MEM_ALLOC_FAIL_REASON_INVALID, /* 块内存, 申请错误原因非法 */
	/*xx*/  ERR_OS_BLK_MEM_FREE_FAIL_REASON_INVALID,  /* 块内存, 释放错误原因非法 */
	/*xx*/  ERR_OS_BLK_MEM_STAT_OBJ_INVALID,    /* 块内存, 统计对象非法 */
	/*xx*/  ERR_OS_BLK_MEM_POOL_STAT_OBJ_INVALID,/* 块内存, 内存池统计对象非法 */
	/*xx*/  ERR_OS_BLK_MEM_LEAK_CHECK_OFF,      /* 块内存, 内存池泄漏检查开关关闭 */

            ERROR_RETRYABLE,                /* Temporary failure. The GUI
                                               should wait briefly */
            ERROR_FATAL,                    /* Fatal error */
            ERROR_RETRYABLE_NO_WAIT,         /* Temporary failure. The GUI
                                               should not wait and should
                                               reconnect */
            ERROR_NOTRANS,                   /* Custom error. This is
                                               returned when a FXP transfer
                                               is sessioned */
            ERROR_TIMEDOUT,                /* Connected timed out */

            ERR_FILE_NAME_NULL,   
            ERR_FILE_NOT_EXIST,
            ERR_FILE_SIZE_ZERO,

            ERR_BLOCK_LIST_NULL,
            ERR_BLOCK_LIST_EMPTY,


            ERR_MALLOC,
	ERR_OS_BUTT
}OS_ERR_CODE;

#endif
