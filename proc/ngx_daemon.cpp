
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include "ngx_daemon.h"
#include "ngx_log.h"
#include "ngx_macro.h"
#include "ngx_variables.h"

int ngx_daemon() {
    switch (fork()){
    case -1:
        ngx_log_file(NGX_LOG_ERR, errno,"fork failed.");
        exit(1);
        break;
    case 0:
        // 子进程不进行任何的处理
        break;
    default:
        // 父进程直接返回1
        return 1;
        break;
    }
    
    // 调整父进程的ID及对应子进程的ID
    ngx_parent = ngx_pid;
    ngx_pid = getpid();

    if ( setsid() == -1 ){
        ngx_log_file(NGX_LOG_ERR, errno, "create new session failed.");
        exit(1);
    }
    
    // 设置为0，不限制文件权限
    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd == -1){
        ngx_log_file(NGX_LOG_ERR, errno, "open /dev/null failed.");
    }
    
    if ( dup2(fd, STDIN_FILENO)  == -1 ){
        ngx_log_file(NGX_LOG_ERR, errno, "dup2(fd, STDIN_FILENO) failed.");
        exit(1);
    }
    
    if ( dup2(fd, STDOUT_FILENO) == -1 ){
        ngx_log_file(NGX_LOG_ERR, errno, "dup2(fd, STDOUT_FILENO) failed.");
        exit(1);
    }
    
    if ( fd >  STDERR_FILENO ){
        if ( close(fd) == -1 ){
            ngx_log_file(NGX_LOG_ERR, errno, "close /dev/null failed.");
            exit(1);
        }
    }
    
    return 0;
}
