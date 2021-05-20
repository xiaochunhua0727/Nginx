#ifndef __NGX_SETPROCTITLE__H__
#define __NGX_SETPROCTITLE__H__

// 函数功能: 移动环境变量的存储位置
void ngx_init_setproctitle();
// 函数功能: 设置进程的标题
void ngx_setproctitle(const char *title);

#endif  //!__NGX_SETPROCTITLE__H__