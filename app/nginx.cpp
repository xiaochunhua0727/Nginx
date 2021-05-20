#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "ngx_variables.h"
#include "ngx_config.h"
#include "ngx_setproctitle.h"
#include "ngx_log.h"

#include "ngx_macro.h"
#include "ngx_signal.h"
#include "ngx_process_cycle.h"
#include "ngx_daemon.h"
#include "ngx_socket.h"

/* 标题设置相关变量 */
char**g_os_argv;            // 存储标题
int g_environlen = 0;       // 环境变量所占的内存大小
char* g_pEnvmem = nullptr;  // 指向存储环境变量的动态内存

/* 进程本身有关的全局量 */
pid_t ngx_pid;               // 当前进程的pid
pid_t ngx_parent;            // 父进程的pid
int   ngx_process;           // 区分进程的类别
sig_atomic_t  ngx_reap;      // 标记子进程的变化

/* socket相关变量 */
extern CSocket g_socket;  

/* 是否以守护进程模式运行 */
int g_daemonized = 0;

int main(int argc,char** argv) {    
    // 设置进程的标题
    g_os_argv = argv;
    ngx_log.fd = -1;
    ngx_process = NGX_PROCESS_MASTER;
    ngx_pid = getpid();
    ngx_parent = getppid();
    ngx_init_setproctitle();

    // 读取配置文件
    CConfig* pConfig = CConfig::GetInstance();
    if (!pConfig->LoadConfig("nginx.conf")){
        ngx_log_terminal(errno,"read config file failed.");
        exit(1);
    }
    
    // 初始化函数
    ngx_log_init();
    ngx_init_signals();
    if (g_socket.Initialize() == false ) {
        ngx_log_file(NGX_LOG_ERR, errno, " socket init failed.");
        exit(1);
    }
    

    if (pConfig->GetInt("Daemon") == 1 ) {
        int daemonResult = ngx_daemon();
        if ( daemonResult != 0) {
            return 0;
        }
        
        // 标记守护进程为后台进程
        g_daemonized = 1;
    }
    
    // 开始执行工作
    ngx_master_process_cycle();
    
    return 0;
}