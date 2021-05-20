#ifndef __NGX_VARIABLES__H__
#define __NGX_VARIABLES__H__

#include <signal.h>
// #include "ngx_socket.h"

extern char **g_os_argv;
extern int g_environlen;
extern char* g_pEnvmem;

extern pid_t ngx_pid;               // 当前进程的pid
extern pid_t ngx_parent;            // 父进程的pid
extern int ngx_process;             // 区分进程的类别
extern sig_atomic_t  ngx_reap;      // 标记子进程的变化
// extern CSocket g_socket; 

#endif  //!__NGX_VARIABLES__H__