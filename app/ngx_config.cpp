#include <mutex>
#include <iostream>
#include <fstream>
#include <regex>
#include "ngx_config.h"
#include "ngx_log.h"

std::mutex CConfig::m_mutex;
// 读取配置文件类对象初始化(静态变量)
CConfig* CConfig::m_instance = nullptr;

/* 类功能:
 * 1. 程序结束时释放CConfig类中创建的单例对象，保证无内存释放
*/
class CDestructConfig {
public:
    ~CDestructConfig() {
        CConfig* pConfig = CConfig::GetInstance();
        if (pConfig != nullptr){
            delete pConfig;
            pConfig = nullptr;
        }
    }
};

// 功能: 获得读取配置文件的单例类对象
CConfig* CConfig::GetInstance() {
    if (m_instance == nullptr){
        std::unique_lock<std::mutex> sbguard(m_mutex);
        if (m_instance == nullptr){
            m_instance = new CConfig;
            // 程序结束时，obj对象释放会导致分配给m_instance的动态内存释放
            static CDestructConfig obj;
        }
    }

    return m_instance;
}

// 函数功能: 将配置文件加载到内存中
bool CConfig::LoadConfig(std::string filePath) {
    std::ifstream fileStream;
    fileStream.open(filePath.c_str(),std::ios::in|std::ios::out);
    // 判断配置文件是否打开成功
    if (!fileStream.is_open()){
        ngx_log_terminal(0, "open config file failed.");
        return false;
    }
    
    size_t equalSignPos = 0;
    std::string key;
    std::string value;
    std::string lineContent;
    std::smatch result;
    std::regex pattern(std::string("[a-z0-9A-Z_]{1,}"));

    // 一直读取配置文件,直到配置文件读取完成
    while (!fileStream.eof()){
        std::getline(fileStream,lineContent);
        equalSignPos = lineContent.find_first_of('=');
        // 忽略掉配置文件中的空白行、注释行等无效行
        if (equalSignPos == std::string::npos){
            continue ;
        }
        
        // 读取每配置项(注: 配置项分为key和value两项)
        key = lineContent.substr(0, equalSignPos);
        value = lineContent.substr(equalSignPos + 1);
        // 删除配置项前后的空白无效字符
        std::regex_search(key, result, pattern);
        key = result.str();
        std::regex_search(value, result, pattern);
        value = result.str();
        m_configItemList.insert(std::pair<std::string,std::string>(key, value));
    }
    
    fileStream.close();

    return true;
}

// 函数功能: 以string格式读取配置文件
std::string CConfig::GetString(std::string key ) {
    if (!m_configItemList.count(key)){
        ngx_log_terminal(0, "key(%s) not exist...", key);
        exit(1);
    }
    return m_configItemList[key];
}

// 函数功能:  以整型格式解读配置项
int CConfig::GetInt(std::string key) {
    if ( !m_configItemList.count(key) ) {
        ngx_log_terminal(0, "key(%s) not exist...", key);
        exit(1);
    }

    return std::stoi(m_configItemList[key]) ;
}