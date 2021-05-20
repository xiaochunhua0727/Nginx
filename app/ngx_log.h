#ifndef _NGX_LOG_H__
#define _NGX_LOG_H__

// 支持的格式字符类型
enum Fmt
{
    UInt = 1,
    Int = 2,
    Float = 3,
    DotDigitalFloat = 4,
    String = 5,
    Pid = 6
};

// 格式字符的位置类型和位置
struct fmtInfo
{
    Fmt fmtType;
    size_t index;
};

typedef struct
{
    int log_level; //日志级别 或者日志类型，ngx_macro.h里分0-8共9个级别
    int fd;        //日志文件描述符
} ngx_log_t;

extern ngx_log_t ngx_log;

// 向控制台打印日志信息
#define ngx_log_terminal(err, fmt, args...) ngx_log_warning(err, fmt, __FILE__, __LINE__, ##args)
#define ngx_log_file(level, err, fmt, args...) ngx_log_error_core(level, err, fmt, __FILE__, __LINE__, ##args)

void ngx_log_init();
void ngx_log_warning(int err, const char *fmt, const char *file, int line, ...);
// level为日志等级，err为错误码
void ngx_log_error_core(int level, int err, const char *fmt, const char *fileName, int line, ...);  

#endif
