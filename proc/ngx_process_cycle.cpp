
#include <signal.h>
#include <cerrno>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include "ngx_process_cycle.h"
#include "ngx_log.h"
#include "ngx_macro.h"
#include "ngx_variables.h"
#include "ngx_setproctitle.h"
#include "ngx_config.h"

CSocket g_socket;

// 主进程处理业务函数并创建所有子进程
void ngx_master_process_cycle() {
    // 清空信号集,并将需屏蔽的信号添加到信号集中
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);     //子进程状态改变
    sigaddset(&set, SIGALRM);     //定时器超时
    sigaddset(&set, SIGIO);       //异步I/O
    sigaddset(&set, SIGINT);      //终端中断符
    sigaddset(&set, SIGHUP);      //连接断开
    sigaddset(&set, SIGUSR1);     //用户定义信号
    sigaddset(&set, SIGUSR2);     //用户定义信号
    sigaddset(&set, SIGWINCH);    //终端窗口大小改变
    sigaddset(&set, SIGTERM);     //终止
    sigaddset(&set, SIGQUIT);     //终端退出符
    // 将信号集中的信号进行屏蔽
    if (sigprocmask(SIG_BLOCK, &set, nullptr) == -1){
        ngx_log_file(NGX_LOG_EMERG, errno,  "call sigprocmask function failed.");
        exit(1);
    }
    
    // 为主进程设置标题
    ngx_setproctitle("./nginx master process");
    
    CConfig* pConfig = CConfig::GetInstance();
    int workProcess = pConfig->GetInt("WorkerProcesses");
    ngx_start_worker_processes(workProcess);

    // 清空信号集中的所有信号
    sigemptyset(&set);
    
    // 主进程会执行到并等待外部的驱动信号
    for( ; ; ) {
        // 设置新的屏蔽信号集，且函数进行阻塞。当接收到外部信号时，恢复原先的屏蔽信号，
        // 调用信号处理函数完毕后程序继续往下执行。
        sigsuspend(&set);
        // 若检测到子进程已关闭，则重新创建一个子进程
        if (ngx_reap == 1) {
            ngx_reap = 0;
            ngx_spawn_process(0, "new work process");
        }
        
    }

    return ;
}

// 函数功能: 创建所有的工作进程
void ngx_start_worker_processes(int threadnums) {
    for (size_t i = 0; i < threadnums; i++) {
        ngx_spawn_process(i, "work process");
    }
}

// 函数功能: 创建子进程
int ngx_spawn_process(int inum,const char *pprocname) {
    pid_t pid = fork();
    switch (pid){
    case -1:
        ngx_log_file(NGX_LOG_ERR, errno, "Fork child process failed.");
        exit(1);
        break;
    case 0:
        // 获取子进程的IP和子进程的父进程ID
        ngx_parent = ngx_pid;
        ngx_pid = getpid();
        ngx_worker_process_cycle(inum, pprocname);
        break;
    default:
        break;
    }

    // 只有主进程会执行到此处,父进程不会执行到此处
    return pid;
}

// 函数功能: 子进程的处理业务函数
void ngx_worker_process_cycle(int inum,const char *pprocname) {
    ngx_process = NGX_PROCESS_WORKER;

    // 子进程初始化并设置子进程的标题
    ngx_worker_process_init(inum);
    ngx_setproctitle(pprocname);
    g_socket.ngx_epoll_init();
    for( ; ; ) {
        // 子进程执行的代码
    }

    return ;
}

// 函数功能: 子进程取消对信号的屏蔽
void ngx_worker_process_init(int inum) {
    // 设置需屏蔽的信号集
    sigset_t set; 
    sigemptyset(&set);
    if (sigprocmask(SIG_SETMASK, &set, nullptr) == -1) {
        ngx_log_file(NGX_LOG_ERR, errno, "call sigprocmask function failed.");
        exit(1);
    }

    return ;
}
