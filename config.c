#include "config.h"
#include "lib/lib_minini.h"

static STATUS_CONFIG callback_status = STATUS_CONFIG_SUCCESS;
static int configs_count = 0;

unsigned short g_control_port = 0;
unsigned short g_data_port = 0;
char g_file_path[MAX_CONFIG_STRING_LENGTH] = {0};
char g_server_ip[MAX_CONFIG_STRING_LENGTH] = {0};
char g_train_no[MAX_CONFIG_STRING_LENGTH] = {0};




/*cfg_entities[] = {......}; 
  定义系统包含的配置项，可以自行扩展*/
CONFIG_DEFINE_BEGIN(cfg_entities)
CONFIG_ENTITY(&g_control_port, "control_port", CONFIG_TYPE_SHORT)
CONFIG_ENTITY(&g_data_port, "data_port", CONFIG_TYPE_SHORT)
CONFIG_ENTITY(g_file_path, "file_path", CONFIG_TYPE_STR)
CONFIG_ENTITY(g_server_ip, "server_ip", CONFIG_TYPE_STR)
CONFIG_ENTITY(g_train_no, "train_no", CONFIG_TYPE_STR)
CONFIG_DEFINE_END

void set_default_config()
{
    g_control_port = DEFAULT_CONTROL_PORT;
    g_data_port = DEFAULT_DATA_PORT;
    strcpy(g_file_path, DEFAULT_FILE_PATH);
    strcpy(g_server_ip, DEFAULT_SERVER_IP);
    strcpy(g_train_no, DEFAULT_TRAIN_NO);
}

/* 对全局变量进行赋值  */
static STATUS_CONFIG set_globle_values(cfg_entity entity, void *input_value)
{
    STATUS_CONFIG cfg_status = STATUS_CONFIG_SUCCESS;
    switch(entity.type){
        case CONFIG_TYPE_BOOL:
            if(!strcmp((char *)input_value, "enable"))
                *((char *)entity.value) = CONFIG_ENABLE;
            else if(!strcmp((char *)input_value, "disable"))
                *((char *)entity.value) = CONFIG_DISABLE;
            else
                cfg_status = STATUS_CONFIG_SUCCESS;
            break;
        case CONFIG_TYPE_INT:
            if (input_value < 0)
                cfg_status = STATUS_CONFIG_INVALID_VALUE;
            else
                *((int *)entity.value) = atoi((char *)input_value);
            break;
        case CONFIG_TYPE_SHORT:
            if (input_value < 0)
                cfg_status = STATUS_CONFIG_INVALID_VALUE;
            else
                *((unsigned short *)entity.value) = atoi((char *)input_value);
            break;

         case CONFIG_TYPE_STR:
            if(strlen(input_value) > MAX_CONFIG_STRING_LENGTH)
                cfg_status = STATUS_CONFIG_INVALID_VALUE;
            else
                strcpy((char *)entity.value, input_value);
            break;
        default:
            break;
    }
    return cfg_status;
}

static void handle_config_status(char* pos, STATUS_CONFIG stat)
{   
   
#if 0
    printf("return code at %s : %d\r\n", pos, stat);
#endif
    if(STATUS_CONFIG_SUCCESS != stat)
        callback_status = stat;
    
    return;
}

/* minIni 中遍历ini 文件后的回调函数，用于将读到的配置项配置到相应的全局变量当中 */
static int ini_browse_callback(const char *section, const char *key, const char *value, const void *userdata)
{
    int i;
    int entity_count = 0;
    entity_count = sizeof(cfg_entities) / sizeof(cfg_entity);
    for(i = 0; i < entity_count; i++)
    {
        if(!strcmp(key, cfg_entities[i].name))
            handle_config_status(cfg_entities[i].name,\
                set_globle_values(cfg_entities[i], value));
    }
    return 1;
}


#ifdef DEBUG_CONFIG
void cfg_test()
{
    int i = 0;
    for (i =0; i < configs_count; i++)
    {
        switch(cfg_entities[i].type)
        {
            case CONFIG_TYPE_BOOL:
                printf("%s %s\r\n", cfg_entities[i].name,\
                    *((char *)cfg_entities[i].value) == CONFIG_ENABLE ?\
                    "enable" : "disable");
                break;
            case CONFIG_TYPE_INT:
                printf("%s %d\r\n", cfg_entities[i].name,\
                *((int *)cfg_entities[i].value));
                break;
            case CONFIG_TYPE_STR:
                printf("%s %s\r\n", cfg_entities[i].name,\
                (char *)cfg_entities[i].value);
                break;
            default:
                break;
        }

    }
    return;
}
#endif

/* 接口函数 */

/* 对全局变量进行初始化 */
void cmd_init_config()
{
    int i;
    configs_count = sizeof(cfg_entities) / sizeof(cfg_entity);

    for (i =0; i < configs_count; i++)
    {
        switch(cfg_entities[i].type)
        {
            case CONFIG_TYPE_BOOL:
                *((char *)cfg_entities[i].value) = 0;
                break;
            case CONFIG_TYPE_INT:
                *((int *)cfg_entities[i].value) = 0;
                break;
            case CONFIG_TYPE_SHORT:
                *((unsigned short *)cfg_entities[i].value) = 0;
                break;
            case CONFIG_TYPE_STR:
                memset((char *)cfg_entities[i].value, 0,\
                MAX_CONFIG_STRING_LENGTH);
                break;
            default:
                break;
         }
    }

    return;
}

/* 解析配置文件中的配置信息，并保存到对应全局变量中*/
STATUS_CONFIG cmd_parse_config()
{

    callback_status = STATUS_CONFIG_SUCCESS;

    if (!ini_browse(ini_browse_callback, NULL, CONFIG_FILE))
        return -1;

    return callback_status;
}

/* 设置指定配置项*/
STATUS_CONFIG cmd_set_config(char *config_name, char *value, char *file_name)
{
    int retval = 0;

    retval = ini_puts(file_name, config_name, value, file_name);

    return retval ? STATUS_CONFIG_SUCCESS : STATUS_CONFIG_SET_FAILED;

}






