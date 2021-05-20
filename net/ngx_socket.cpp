#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/epoll.h> 

#include "ngx_socket.h"
#include "ngx_config.h"
#include "ngx_log.h"
#include "ngx_macro.h"

CSocket::CSocket() {
    m_ListenPortCount = 1;
    // epoll允许的最多连接项数
    m_worker_connections = 1;
}

CSocket::~CSocket() {
    std::vector<lpngx_listening_t>::iterator ite = m_ListenSocketList.begin();
    for ( ; ite != m_ListenSocketList.end(); ++ite ){
        delete(*ite);
    }
    
    m_ListenSocketList.clear();
}

bool CSocket::Initialize() {
    ReadConf();
    bool isOpen = ngx_open_listening_sockets();
    return isOpen;
}

bool CSocket::ngx_open_listening_sockets() {
    // CConfig* pConfig = CConfig::GetInstance();
    // m_ListenPortCount = pConfig->GetInt("ListenPortCount");

    int servSock;
    int servPort;
    struct sockaddr_in servAddr; 
    memset( &servAddr, 0, sizeof(struct sockaddr_in) );
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr =  htonl(INADDR_ANY);

    for ( size_t i = 0; i < m_ListenPortCount; ++i ) {
        servSock = socket(AF_INET, SOCK_STREAM, 0);
        if (servSock == -1) {
            ngx_log_file( NGX_LOG_ERR, errno, "create socket failed.");
            return false;
        }

        int reuseaddr = 1;
        
        // 设置允许端口重用
        if(setsockopt(servSock,SOL_SOCKET, SO_REUSEADDR,(const void *) &reuseaddr, sizeof(reuseaddr)) == -1) {
            ngx_log_file(NGX_LOG_ERR, errno,"setsockopt(SO_REUSEADDR) failed");
            return false;
        }
        
        // 设置端口不阻塞
        if ( setnonblocking(servSock) == false ){
            ngx_log_file(NGX_LOG_EMERG, errno, "setnonblocking(servSock) failed.");
            close(servSock);
            return false;
        }
        
        // 从内存中读取服务端的端口号
        std::string ListenPort = "ListenPort" + std::to_string(i);
        CConfig* pConfig = CConfig::GetInstance();
        servPort = pConfig->GetInt(ListenPort);
        servAddr.sin_port = htons(servPort);
        if( bind( servSock , (struct sockaddr*)&servAddr, sizeof(servAddr) ) == -1) {
            ngx_log_file(NGX_LOG_ERR, errno, "create socket failed.");
            close(servSock);
            return false;
        }
         
        // 开始监听套接字
        if ( listen(servSock, NGX_LISTEN_BACKLOG) == -1 ){
            ngx_log_file(NGX_LOG_ALERT, errno, "listen socket failed.");
            close(servSock);
            return false;
        }
        
        // 将监听的端口与套接字保存到类成员变量中
        lpngx_listening_t pListenItem = new ngx_listening_t;
        memset(pListenItem, 0, sizeof(lpngx_listening_t) );
        pListenItem->fd = servSock;
        pListenItem->port = servPort;
        ngx_log_terminal(0, "listen %d port success.", servPort);
        m_ListenSocketList.push_back(pListenItem);
    }
    
    return true;
}

bool CSocket::setnonblocking(int sockfd) {
    int nb = 1; //0：清除，1：设置  

    if( ioctl(sockfd, FIONBIO, &nb) == -1) {
        return false;
    }
    return true;
}

// 函数功能: 从内存中读取配置文件
void CSocket::ReadConf() {
    CConfig* pConfig = CConfig::GetInstance();
    m_worker_connections = pConfig->GetInt("workerConnections");
    m_ListenPortCount = pConfig->GetInt("ListenPortCount");
}

void CSocket::ngx_close_listening_sockets() {
    for (size_t i = 0; i < m_ListenPortCount; i++){
        close(m_ListenSocketList[i]->fd);
        ngx_log_file(NGX_LOG_NOTICE, 0, "port %d closed.", m_ListenSocketList[i]->fd);
    }

    return ;
}

int CSocket::ngx_epoll_init() {
    // 创建epoll文件描述符
    m_epollhandle = epoll_create(m_worker_connections);
    if (m_epollhandle == -1){
        ngx_log_file(NGX_LOG_ERR, errno, "call epoll_create failed.");
        exit(2);
    }
    
    // 记录当前连接池中连接总数
    m_connection_n = m_worker_connections;
    // 连接池
    m_pconnections = new ngx_connection_t[m_connection_n];
    
    // 将数组元素串联起来，构成一个连接池
    int i = m_connection_n;
    lpngx_connection_t next = nullptr;
    lpngx_connection_t c = m_pconnections;
    do {
        i--;
        c[i].data = next;
        c[i].fd = -1;
        c[i].instance = 1;
        c[i].iCurrsequence = 0; // 当前序号从零开始
        
        next = &c[i];  // 指针向前移
    }while (i);
    
    // 空闲链表头指针(初始指向c[0]),及空闲连接链表长度
    m_pfree_connections = next;
    m_free_connection_n = m_connection_n;

    std::vector<lpngx_listening_t>::iterator ite = m_ListenSocketList.end();
    for ( ; ite != m_ListenSocketList.end(); ++ite){
        c = ngx_get_connection((*ite)->fd);
        if (c == NULL){
            ngx_log_file(NGX_LOG_ERR, 0, "Get Connect failed.");
            exit(2);
        }

        // 连接对象和监听对象关联，方便通过连接对象找监听对象
        c->listening = (*ite);
        // //监听对象 和连接对象关联，方便通过监听对象找连接对象
        (*ite)->connection = c;

    }
    
}