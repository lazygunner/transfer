#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG_CONFIG 1
#undef DEBUG_CONFIG_PARSE

#define CONFIG_FILE "transfer.ini"
#define CONFIG_ENABLE 1
#define CONFIG_DISABLE 0

#define MAX_CONFIG_STRING_LENGTH 33
#define MAX_CONFIG_NAME_LENGTH 20

#define DEFAULT_CONTROL_PORT 6260
#define DEFAULT_DATA_PORT 6160
#define DEFAULT_SERVER_IP "192.168.2.158"
#define DEFAULT_FILE_PATH "files/"
#define DEFAULT_TRAIN_NO "abcd1234"

/* 声明全局变量，用于其他文件使用 */
extern unsigned short g_control_port;
extern unsigned short g_data_port;
extern char g_server_ip[MAX_CONFIG_STRING_LENGTH];
extern char g_file_path[MAX_CONFIG_STRING_LENGTH];
extern char g_train_no[MAX_CONFIG_STRING_LENGTH];

enum status_config{
        STATUS_CONFIG_SUCCESS = 0,
        STATUS_CONFIG_INVALID_VALUE = 1,
        STATUS_CONFIG_SET_FAILED = 2,
        STATUS_CONFIG_NOT_FOUND = 404,
        STATUS_CONFIG_DEFAULT
};

enum cfg_type{
        CONFIG_TYPE_INT,
        CONFIG_TYPE_SHORT,
        CONFIG_TYPE_STR,
        CONFIG_TYPE_BOOL,
        CONFIG_TYPE_DEFAULT
};

#define STATUS_CONFIG enum status_config
#define TYPE_CONFIG enum cfg_type


typedef struct _cfg_entity 
{
        void *value;
        char name[MAX_CONFIG_NAME_LENGTH];
        TYPE_CONFIG type;
}cfg_entity;

/* 用于定义config entities 的宏 */
#define CONFIG_DEFINE_BEGIN(CFG) \
        cfg_entity CFG[] = {

#define CONFIG_DEFINE_END \
                {(void*)NULL, "config_end", CONFIG_TYPE_DEFAULT}\
        };

#define CONFIG_ENTITY(ENTITY, NAME, TYPE) \
            {(void*)ENTITY, NAME, TYPE},



static STATUS_CONFIG set_globle_values(cfg_entity entity, void *input_value);
static void handle_config_status(char* pos, STATUS_CONFIG stat);
static int ini_browse_callback(const char *section, const char *key, const char *value, const void *userdata);
void cfg_test();
void cmd_init_config();
STATUS_CONFIG cmd_parse_config();
STATUS_CONFIG cmd_set_config(char *config_name, char *value, char *file_name);
void set_default_config();
