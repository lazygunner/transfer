#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include "thread.h"
#include "transfer.h"



int init_send_file(file_desc *f_desc);
int get_file_info(char *path, unsigned char **buf);
void handle_thread(void *args);

#endif
