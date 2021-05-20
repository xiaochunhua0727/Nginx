#include <iostream>
#include <string>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>  
#include <time.h>      
#include <errno.h>
#include <time.h>
#include <math.h>

#include "ngx_log.h"
#include "ngx_config.h"
#include "ngx_macro.h"

std::vector<std::string> err_levels = {" [stderr] "," [emerg] "," [alert] "," [crit] ",
                               " [error]"," [warn] "," [notice] "," [info] "," [debug] "};

ngx_log_t ngx_log;

void ngx_log_init() {
    CConfig* pConfig = CConfig::GetInstance();
    
    ngx_log.log_level = pConfig->GetInt("LogLevel");
    ngx_log.fd = open(NGX_ERROR_LOG_PATH,O_WRONLY|O_APPEND|O_CREAT,0777);
    
    if (ngx_log.fd == -1){
        ngx_log_terminal(errno,"open %s failed.",NGX_ERROR_LOG_PATH);
        ngx_log.fd = STDERR_FILENO;
    }
    return ;
}

std::vector<fmtInfo> findFmt(const char *fmt) {
    size_t startPos = 0;
    std::string fmtStr(fmt);
    size_t fmtLength = fmtStr.length();
    
    fmtInfo node;
    std::vector<fmtInfo> fmtInfos;

    while (1){
        size_t pos = fmtStr.find_first_of('%',startPos);
        // 若未查找到格式字符串，则直接退出循环
        if (pos == std::string::npos){
            break;
        }
        
        ++pos ;
        if (pos >= fmtLength){
            break;
        }

        switch (fmtStr[pos]){
        case 'u':
            node.fmtType = Fmt::UInt;
            node.index = pos - 1;
            startPos = pos;
            fmtInfos.push_back(node);
            break;
        case 'd':
            node.fmtType = Fmt::Int;
            node.index = pos - 1;
            startPos = pos;
            fmtInfos.push_back(node);
            break;
        case 'f':
            node.fmtType = Fmt::Float;
            node.index = pos - 1;
            startPos = pos;
            fmtInfos.push_back(node);
            break;
        case 's':
            node.fmtType = Fmt::String;
            node.index = pos - 1;
            startPos = pos;
            fmtInfos.push_back(node);
            break;
        case 'p':
            node.fmtType = Fmt::Pid;
            node.index = pos - 1;
            startPos = pos;
            fmtInfos.push_back(node);
            break;
        case '.':
            if (pos + 2 < fmtLength){
                if ('0' < fmtStr[pos + 1] && fmtStr[pos + 1] <= '9' && fmtStr[pos + 2] == 'f'){
                    node.fmtType = Fmt::DotDigitalFloat;
                    node.index = pos - 1;
                    fmtInfos.push_back(node);
                }
               
            }
            startPos = pos;
            break;
        default:
            startPos = pos;
            break;
        }
        
    }
    return fmtInfos;
}

std::string ngx_vslprintf(const char *fmt,va_list args) {
    std::vector<fmtInfo> fmtInfos = findFmt(fmt);
    std::vector<fmtInfo>::iterator ite = fmtInfos.begin();
    std::string errMsg = "";
    std::string valueStr = "";
    size_t startPos = 0;
    size_t dotPos = 0;
    char cStr[100];
    std::string fmtStr(fmt);
    double dValue = 0.0;

    for ( ; ite != fmtInfos.end(); ++ite){
        errMsg += fmtStr.substr(startPos,ite->index - startPos);
        int typeCase = (int)ite->fmtType;
        switch (ite->fmtType){
        case Fmt::UInt:
            valueStr = std::to_string((unsigned int)va_arg(args, unsigned int));
            errMsg += valueStr;
            startPos = ite->index + 2;
            break;
        case Fmt::Int:
            valueStr = std::to_string((int)va_arg(args,int));
            errMsg += valueStr;
            startPos = ite->index + 2;
            break;
        case Fmt::Float:
            valueStr = std::to_string((double)va_arg(args,double));
            errMsg += valueStr;
            startPos = ite->index + 2;
            break;
        case Fmt::DotDigitalFloat:
            valueStr = std::to_string((double)va_arg(args,double));
            dotPos = valueStr.find_first_of('.');
            dValue = std::stod(valueStr.substr(0,(int)valueStr[dotPos + 1] - 45 + dotPos)) + \
                       5 * pow(10, ((int)valueStr[dotPos + 1] - 46) * -1 );
            errMsg += std::to_string(dValue) .substr(0,(int)valueStr[dotPos + 1] - 46 + dotPos);
            startPos = ite->index + 4;
            break;
        case Fmt::String:
            sprintf(cStr,"%s",va_arg(args,char*));
            errMsg += std::string(cStr);
            startPos = ite->index + 2;
            break;
        case Fmt::Pid:
            valueStr = std::to_string((int)va_arg(args,int));
            errMsg += valueStr;
            startPos = ite->index + 2;
            break;
        }
    }
    errMsg += fmtStr.substr(startPos);
    return errMsg;
}

std::string ngx_log_errno(int err) {
    std::string perrorInfo  = std::string(strerror(err));
    std::string leftStr = "(" + std::to_string(err) + ":";
    std::string rightStr = ")";
    return leftStr + perrorInfo + rightStr;
}

void ngx_log_warning(int err,const char* fmt,const char* file, int line, ...) {
    // 定位配置文件出错的位置
    std::string fileName(file);
    std::string errPostion = fileName + "(" + std::to_string(line) + "): ";
    
    // 解析出错信息
    va_list args;
    va_start(args, line);
    std::string errMsg = ngx_vslprintf(fmt, args);
    va_end(args);

    std::string errCodeString = "";
    // 依据错误码找出对应的错误信息
    if (err){
        errCodeString = ngx_log_errno(err);
    }
    
    std::string errInfo = errPostion + errMsg + errCodeString + "\n";

    write( STDIN_FILENO, errInfo.c_str(), strlen(errInfo.c_str()) );
    if (ngx_log.fd > STDIN_FILENO){
        write(ngx_log.fd, errInfo.c_str(), strlen(errInfo.c_str()) );
    }
        
    return ;
}

void ngx_log_error_core(int level,  int err, const char *fmt,const char* fileName, int line, ...) {·    ·   ·                       
    struct timeval tv;
    struct tm tm;
    time_t sec;  // 秒
    memset(&tv, 0, sizeof(struct timeval) );
    memset(&tm, 0, sizeof(struct tm) );
    
    gettimeofday(&tv, nullptr);
    sec = tv.tv_sec;
    localtime_r(&sec, &tm);
    tm.tm_mon++;                 //月份要调整下正常
    tm.tm_year += 1900;          //年份要调整下才正常
    
    std::string errMsg;
    char cTimeStr[100] = {0};
    sprintf(cTimeStr, "%4d/%02d/%02d %02d:%02d:%02d", tm.tm_year, \
            tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);  
    errMsg += cTimeStr;
    errMsg += err_levels[level];
    std::string errPos = std::string(fileName) + ":" + std::to_string(line);
    errMsg += "{" + errPos + "}" + ":";

    va_list args;
    va_start(args,line);
    errMsg += ngx_vslprintf(fmt, args);
    va_end(args);
    if (err){
        errMsg += ngx_log_errno(err) + '\n';
    }
    
    if ( level > ngx_log.log_level ){
        ngx_log_terminal(0, "日志等级太低，不打印到日志文件.");
        return;
    }
    
    size_t n = write(ngx_log.fd, errMsg.c_str(), strlen(errMsg.c_str())); 
    if (n == -1){
        write(ngx_log.fd, errMsg.c_str(), strlen(errMsg.c_str()));
    }

    return ;
}