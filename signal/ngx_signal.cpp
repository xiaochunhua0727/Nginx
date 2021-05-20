#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
 #include <sys/wait.h>

#include "ngx_signal.h"
#include "ngx_log.h"
#include "ngx_macro.h"
#include "ngx_variables.h"

ngx_signal_t signals[] = {
    // signo      signame             handler
    { SIGHUP,    "SIGHUP",           ngx_signal_handler },       
    { SIGINT,    "SIGINT",           ngx_signal_handler },       
	{ SIGTERM,   "SIGTERM",          ngx_signal_handler },       
    { SIGCHLD,   "SIGCHLD",          ngx_signal_handler },       
    { SIGQUIT,   "SIGQUIT",          ngx_signal_handler },        
    { SIGIO,     "SIGIO",            ngx_signal_handler },       
    { SIGSYS,    "SIGSYS, SIG_IGN",  NULL               },       
    //信号对应的数字至少是1，所以可以用0作为一个特殊标记                                                       
    { 0,         NULL,               NULL               }         
};

int ngx_init_signals() {
    ngx_signal_t* sig = signals;
    struct sigaction sa;

    for ( ; sig->signo != 0; ++sig){
        // 1. 设置信号的处理函数及需屏蔽的信号
        memset(&sa, 0, sizeof(struct sigaction));
        // sa_sigaction与sa_handler都能设置信号处理函数。若需设置sa_sigaction，
        // 则需将sa_flags设置为SA_SIGINFO
        if (sig->handler){
            sa.sa_sigaction = sig->handler;
            // sa_sigaction should be set instead of sa_handler
            sa.sa_flags = SA_SIGINFO;
        }else {
            // 不进行任何的信号处理
            sa.sa_handler = SIG_IGN;
        }
        // 不对信号进行任何的屏蔽
         if(sigemptyset(&sa.sa_mask) == -1) {
            ngx_log_file(NGX_LOG_ERR, errno, "call sigemptyset failed.");
            exit(1);
         }

         // 2. 将信号处理的信息进程注册
         if ( sigaction(sig->signo, &sa, nullptr) == -1){
             ngx_log_file(NGX_LOG_ERR, errno, "call function sigaction failed.");
             exit(1);
         }else {
             ngx_log_terminal(0, "sigaction(%s) successed.", sig->sigName);
         }
    }
    
    return 0;
}

void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext) {
    ngx_signal_t* sig = signals;
    for ( ; sig->signo != 0; ++sig ){
        if ( sig->signo == signo){
            // 找到对应进程，并进行相应的处理
            break;
        }
    }
    
    if (ngx_process == NGX_PROCESS_MASTER){
        switch (signo){
        case SIGCHLD:
            // 标记子进程发生变化，日后主进程的业务处理代码可能会用到
            ngx_reap = 1;  
            break;
            // 其它信号的处理case
        default:
            break;
        }
    }else if( ngx_process == NGX_PROCESS_WORKER ) {
        // 工作进程的处理代码
    }
    
    ngx_log_file(NGX_LOG_NOTICE, 0, "signal %d(%s) received", signo, sig->sigName);
    
    if( signo == SIGCHLD ) {
        ngx_process_get_status();
    }

    return ;
}

void ngx_process_get_status(void) {
    pid_t pid;
    int status;
    int  err;
    int one = 0;
    for ( ; ; ) {
        pid = waitpid(-1, &status, WNOHANG);
        
        // 子进程未发生任何变化，则直接退出僵尸进行处理函数
        if (pid == 0){
            return ;
        }
        
        // 发生依次
        if (pid == -1){
            err = errno;
            // 被外部信号所打断
            if (err == EINTR) {
                continue ;
            }
            
            // waitpid为非阻塞函数，多次执行循环语句。若第一次进入就出现ECHILD的问题，
            // 则表明程序出现异常。
            if(err == ECHILD && one) {
                return ;
            }

            ngx_log_file(NGX_LOG_ALERT, errno, "waitpid failed.");
            return ;
        }
        
        one = 1;  // 标记waitpid返回了正常的值

        // 进程可能收到信号关闭或执行完成后关闭
        if ( WTERMSIG(status) ){
            ngx_log_file(NGX_LOG_ALERT, 0, "pid = %p exited on signal %d!", pid, WTERMSIG(status));
        }else {
            ngx_log_file(NGX_LOG_ALERT, 0, "pid = %p exited with code: %d!", pid, WEXITSTATUS(status));
        }
   
    }

    return ;
}