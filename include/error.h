#ifndef __ERR_NO_
#define __ERR_NO_

#define SUCCESS_NO_ERROR 0
#define RET_SUCCESS 0
/* ֧��ģ��Ĳ���ϵͳ��ģ����ڲ������� */
typedef enum tag_OS_ERR_CODE
{
	/*00*/  ERR_OS_GENERAL_FAIL = 0,

	/*01*/  ERR_OS_ROOT_INIT_FAIL,              /* root�ĳ�ʼ��ʧ�� */
	/*02*/  ERR_OS_MEM_INIT_MEM_TOO_SMALL,      /* �ڴ��ʼ��, ������ڴ�̫С */
	/*03*/  ERR_OS_MEM_ALLOC_TYPE,              /* �ڴ�����, ��������ڴ����ʹ��� */
	/*04*/  ERR_OS_MEM_ALLOC_SIZE_ZERO,         /* �ڴ�����, ������������ڴ��СΪ0 */
	/*05*/  ERR_OS_MEM_ALLOC_SIZE_TOO_LARGE,    /* �ڴ�����, ������������ڴ�̫�� */
	/*06*/  ERR_OS_MEM_ALLOC_MEM_EMPTY,         /* �ڴ�����, �ڴ��ѷ�����, �޷��ٷ��� */
	/*07*/  ERR_OS_MEM_FREE_ADDR_MISMATCH,      /* �ڴ��ͷ�, ��������ڴ��ַ��ƥ�� */
	/*08*/  ERR_OS_MEM_FREE_OVERWRITE,          /* �ڴ��ͷ�, �ͷŵ��ڴ汻Խ���д */
	/*09*/  ERR_OS_MEM_REE_NOT_USED,            /* �ڴ��ͷ�, �ͷ�û��������ڴ� */

	/*10*/  ERR_OS_SCHD_CREATE_QUE,             /* ����, ������Ϣ����ʧ�� */
	/*11*/  ERR_OS_SCHD_CREATE_TASK,            /* ����, ��������ʧ�� */
	/*12*/  ERR_OS_SCHD_ALLOC_MSG,              /* ����, ������Ϣ��ʧ�� */
	/*13*/  ERR_OS_SCHD_SEND_MSG,               /* ����, ������Ϣ��ʧ�� */
	/*14*/  ERR_OS_SCHD_FREE_MSG,               /* ����, �ͷ���Ϣ��ʧ�� */

	/*xx*/  ERR_OS_BLK_MEM_INIT_FAIL,           /* ���ڴ�, ��ʼ��ʧ��*/
	/*xx*/  ERR_OS_BLK_MEM_STAT_INIT_FAIL,      /* ���ڴ�, ͳ����Ϣ��ʼ��ʧ��*/
	/*xx*/  ERR_OS_BLK_MEM_ALLOC_SIZE_INVALID,  /* ���ڴ�, ����ĳ��ȷǷ� */
	/*xx*/  ERR_OS_BLK_MEM_PID_INVALID,         /* ���ڴ�, PID�Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_PTR_NULL,            /* ���ڴ�, ָ��� */
	/*xx*/  ERR_OS_BLK_MEM_USE_UP,              /* ���ڴ�, ���ڴ�ľ� */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INVALID,        /* ���ڴ�, �ڴ��ͷ���Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_FLAG_INVALID,   /* ���ڴ�, �ڴ��ͷ��Խ���ǷǷ� */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_TYPE_INVALID,   /* ���ڴ�, �ڴ��ͷ���Ŀ��ڴ����ͷǷ� */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INDEX_TOO_BIG,  /* ���ڴ�, �ڴ��ͷ������ڴ������̫�� */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_ADDR_NOT_ALIGN, /* ���ڴ�, �ڴ��ͷ����ַ������ */
	/*xx*/  ERR_OS_BLK_MEM_HEAD_INDEX_INVALID,  /* ���ڴ�, �ڴ��ͷ������ڴ�������Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_TAIL_INVALID,        /* ���ڴ�, �ڴ��β���Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_TAIL_FLAG_INVALID,   /* ���ڴ�, �ڴ��β����Խ���ǷǷ� */
	/*xx*/  ERR_OS_BLK_MEM_RE_FREE,             /* ���ڴ�, �ظ��ͷ� */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_NOT_IN_POOL,    /* ���ڴ�, ��ַ�����ڴ���� */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_NOT_IN_USR_BLK, /* ���ڴ�, ��ַ�����ڴ����û���ַ��Χ�� */
	/*xx*/  ERR_OS_BLK_MEM_TYPE_INVALID,        /* ���ڴ�, ���ڴ����ͷǷ� */
	/*xx*/  ERR_OS_BLK_MEM_BLK_ADDR_INVALID,    /* ���ڴ�, �ڴ��ĵ�ַ�Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_LEAK,                /* ���ڴ�, �ڴ��й© */
	/*xx*/  ERR_OS_BLK_MEM_NOT_LEAK,            /* ���ڴ�, �ڴ��û��й© */
	/*xx*/  ERR_OS_BLK_MEM_CAL_POOL_CRC,        /* ���ڴ�, �����ڴ�ص�CRC���� */
	/*xx*/  ERR_OS_BLK_MEM_POOL_CRC_INVALID,    /* ���ڴ�, �ڴ�ص�CRC�Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_INDEX_INVALID,       /* ���ڴ�, �ڴ�������Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_INVALID,    /* ���ڴ�, ���ж���ָ��Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_END_TOO_MUCH,/* ���ڴ�, ���ж����еĽ���ָ��̫�� */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_PRT_NOT_MATCH,  /* ���ڴ�, ���ж���ָ����ʹ�ñ�ǲ�ƥ�� */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_NUM_INVALID,    /* ���ڴ�, ���м������� */
	/*xx*/  ERR_OS_BLK_MEM_DYN_BUSY_NUM_INVALID,/* ���ڴ�, ��̬ʹ�ü������� */
	/*xx*/  ERR_OS_BLK_MEM_STA_BUSY_NUM_INVALID,/* ���ڴ�, ��̬ʹ�ü������� */
	/*xx*/  ERR_OS_BLK_MEM_BLK_NUM_INVALID,     /* ���ڴ�, ���ڴ��ܼ������� */
	/*xx*/  ERR_OS_BLK_MEM_IDLE_QUE_INVALID,    /* ���ڴ�, ���ж��д��� */
	/*xx*/  ERR_OS_BLK_MEM_ADDR_IN_BLK_HEAD,    /* ���ڴ�, ��ַ���ڴ��ͷ����Χ�� */
	/*xx*/  ERR_OS_BLK_MEM_TERM_INVALIDE,       /* ���ڴ�, �������ն���Ч */
	/*xx*/  ERR_OS_BLK_MEM_COMM_FAIL_REASON_INVALID,  /* ���ڴ�, ͨ�ô���ԭ��Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_ALLOC_FAIL_REASON_INVALID, /* ���ڴ�, �������ԭ��Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_FREE_FAIL_REASON_INVALID,  /* ���ڴ�, �ͷŴ���ԭ��Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_STAT_OBJ_INVALID,    /* ���ڴ�, ͳ�ƶ���Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_POOL_STAT_OBJ_INVALID,/* ���ڴ�, �ڴ��ͳ�ƶ���Ƿ� */
	/*xx*/  ERR_OS_BLK_MEM_LEAK_CHECK_OFF,      /* ���ڴ�, �ڴ��й©��鿪�عر� */

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
