#include<stdint.h>
#include <string.h>
#include "ngx_socket.h"
#include "ngx_log.h"
#include "ngx_macro.h"

lpngx_connection_t CSocket::ngx_get_connection(int isock) {
    // 获得指向空闲链表头的指针
    lpngx_connection_t c = m_pfree_connections;
    if (c == nullptr){
        ngx_log_file(NGX_LOG_ERR, 0, "Get free connection failed.");
        return NULL;
    }
    
    // 空闲链表指向下一个未使用的节点
    m_free_connection_n--;
    m_pfree_connections = c->data;
    
    memset(c, 0, sizeof(ngx_connection_t));
    c->fd = isock;
    
    // 把c指向的对象中有用的东西保存起来
    uintptr_t instance = c->instance;
    uint64_t iCurrsequence = c->iCurrsequence;

    c->instance = !instance;
    c->iCurrsequence = iCurrsequence;
    ++c->iCurrsequence;

    return c;
}