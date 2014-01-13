#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include "thread.h"
#include "transfer.h"



int init_send_file(file_desc *f_desc);
int get_file_info(char *path, unsigned char **buf);
void read_thread(void *args);
void send_thread(void *args);
void receive_handler(void *args);
void send_heartbeat_thread(void *args);


#endif
