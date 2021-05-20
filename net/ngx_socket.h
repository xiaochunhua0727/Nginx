#ifndef _NGX_SOCKET_H__
#define _NGX_SOCKET_H__

#include<stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

#define NGX_LISTEN_BACKLOG  511
#define NGX_MAX_EVENTS      512

typedef class  CSocket CSocket;
typedef struct ngx_listening_s   ngx_listening_t, *lpngx_listening_t;
typedef struct ngx_connection_s  ngx_connection_t,*lpngx_connection_t;
// 成员函数指针，指向处理函数
typedef void (CSocket::*ngx_event_handler_pt)(lpngx_connection_t c);

// 和监听端口有关的结构
struct ngx_listening_s  {
	int port;        //监听的端口号
	int fd;          //套接字句柄socket
	lpngx_connection_t connection;  //连接池中的一个连接，注意这是个指针 
};

// 该结构表示一个TCP连接
struct ngx_connection_s{
	int fd;             //套接字句柄socket
    // 若这个链接被分配给了一个监听套接字，那么这个里边就指向监听套接字对应的那个lpngx_listening_t的内存首地址
	lpngx_listening_t         listening;      		

	//------------------------------------	
    //【位域】失效标志位：0：有效，1：失效
	unsigned instance:1; 
    // 引入的一个序号，每次分配出去时+1，此法也有可能在一定程度上检测错包废包  
	uint64_t iCurrsequence;  
    // 保持客户端的地址
	struct sockaddr s_sockaddr;  

	//和读有关的标志-----------------------
	uint8_t r_ready;        //读准备好标记
	uint8_t w_ready;        //写准备好标记
    //读事件的相关处理方法
	ngx_event_handler_pt      rhandler;    
    //写事件的相关处理方法   
	ngx_event_handler_pt      whandler;       
	// 指向下一个对象，用于把空闲的连接池对象串起来构成一个单向链表，方便取用
	lpngx_connection_t        data;           
};

class CSocket {
public:
    CSocket();
    ~CSocket();
    virtual bool Initialize();
    int ngx_epoll_init();
private:
    bool ngx_open_listening_sockets();
    void ngx_close_listening_sockets();
    bool setnonblocking(int sockfd);
    void ReadConf();

    /* 连接池或连接相关函数 */
    lpngx_connection_t ngx_get_connection(int isock);
    
private:
    // 监听端口的个数
    int m_ListenPortCount; 
    // 允许的最多客户端连接数
    int m_worker_connections;
    // epoll文件描述符
    int m_epollhandle;

    // 当前进程中所有连接对象的总数(连接池的大小)
    int m_connection_n;
    // 连接池中可用连接总数
    int m_free_connection_n;

    /* 与连接池有关的 */
    // 连接池的首地址
    lpngx_connection_t m_pconnections;
    // 空闲连接链表头
    lpngx_connection_t m_pfree_connections;

    // 监听套接字队列
    std::vector<lpngx_listening_t> m_ListenSocketList;
};

#endif 