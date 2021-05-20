#ifndef __NGX_CONFIG__H__
#define __NGX_CONFIG__H__

#include <mutex>
#include <string>
#include <map>

class CConfig {
public:
    // 函数功能: 获取单例对象
    static CConfig* GetInstance();
    // 函数功能: 加载配置文件到内存中
    bool LoadConfig(std::string filePath);
    // 函数功能: 根据键值获取配置选项
    std::string GetString(std::string key);
    // 函数功能: 依据键值获取配置选项
    int GetInt(std::string key);
private:
    static CConfig* m_instance;
    static std::mutex m_mutex;
    std::map<std::string, std::string> m_configItemList;
};

#endif  //!__NGX_CONFIG__H__