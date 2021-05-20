#ifndef _NGX_SIGNAL_H_
#define _NGX_SIGNAL_H_

#include <string>
#include <signal.h>

typedef struct {
    int signo;
    const char* sigName;
    void (*handler)(int, siginfo_t *, void *);
}ngx_signal_t;

int ngx_init_signals();
static void ngx_process_get_status(void);
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);

#endif // !_NGX_SIGNAL_H_