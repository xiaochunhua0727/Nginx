#ifndef __NGX_PROCESS_CYCLE__H__
#define __NGX_PROCESS_CYCLE__H__

#include "ngx_socket.h"

extern CSocket g_socket;

// 函数功能: 主进程处理业务函数并创建子进程
void ngx_master_process_cycle();
// 函数功能: 创建所有的工作进程
static void ngx_start_worker_processes(int threadnums);
// 函数功能: 创建子进程
static int ngx_spawn_process(int inum,const char *pprocname);
// 函数功能: 子进程处业务代码函数
static void ngx_worker_process_cycle(int inum,const char *pprocname);
// 函数功能: 子进程进行相应的初始化
static void ngx_worker_process_init(int inum);

#endif  //!__NGX_PROCESS_CYCLE__H__