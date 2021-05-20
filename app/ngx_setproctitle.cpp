#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "ngx_variables.h"
#include "ngx_setproctitle.h"
#include "ngx_log.h"
#include "ngx_macro.h"

// 函数功能: 移动环境变量的存储位置
void ngx_init_setproctitle() {
    for (size_t cnt = 0; environ[cnt]; ++cnt){
        g_environlen += strlen(environ[cnt]) + 1;
    }
    
    g_pEnvmem = (char*)malloc(g_environlen);
    memset(g_pEnvmem, 0, g_environlen);
    
    size_t size;
    char* pTemp = g_pEnvmem;

    for (size_t cnt = 0; environ[cnt]; ++cnt){
        size = strlen(environ[cnt]) + 1;
        strcpy(pTemp, environ[cnt]);
        environ[cnt] = pTemp;
        pTemp += size;
    }    
}

// 函数功能: 设置进程的标题
void ngx_setproctitle(const char *title) {
    size_t argvSize = 0;
    size_t titleSize = strlen(title);
    size_t totalSize = 0;

    // 统计argv变量占用的字节数
    for (size_t i = 0; g_os_argv[i]; i++){
        argvSize += strlen(g_os_argv[i]) + 1;
    }
    
    totalSize = argvSize + g_environlen;
    if (titleSize >= totalSize){
        ngx_log_file(NGX_LOG_NOTICE, 0, "the title is too long");
        exit(1);
    }
    
    g_os_argv[1] = nullptr;
    strcpy(g_os_argv[0],title);
}