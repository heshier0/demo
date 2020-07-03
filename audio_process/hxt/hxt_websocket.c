#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <uwsc.h>

#include "utils.h"
#include "hxt_defines.h"

static void uwsc_onopen(struct uwsc_client *cl)
{
    uwsc_log_info("onopen\n");

    //added by hekai

    //end added
}

static void uwsc_onmessage(struct uwsc_client *cl,
	void *data, size_t len, bool binary)
{
    printf("Recv:\n");

    if (binary) {
        //文件
    } 
    else 
    {
        printf("[%.*s]\n", (int)len, (char *)data);

    }
}

static void uwsc_onerror(struct uwsc_client *cl, int err, const char *msg)
{
    utils_print("onerror:%d: %s\n", err, msg);
    ev_break(cl->loop, EVBREAK_ALL);
}

static void uwsc_onclose(struct uwsc_client *cl, int code, const char *reason)
{
    utils_print("onclose:%d: %s\n", code, reason);
    ev_break(cl->loop, EVBREAK_ALL);
}

static void signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
    if (w->signum == SIGINT) {
        ev_break(loop, EVBREAK_ALL);
        utils_print("Normal quit\n");
    }
}


int hxt_websocket_start()
{
    struct ev_loop *loop = EV_DEFAULT;
    struct ev_signal signal_watcher;
	int ping_interval = 10;	/* second */
    struct uwsc_client *cl;

    char* hxt_url = hxt_get_websocket_url_cfg();
    char* token = hxt_get_token_cfg();
    char extra_header[1024] = {0};
    strcpy(extra_header, "Sec-WebSocket-Protocol: ");
    strcat(extra_header, "Bearer ");
    strcat(extra_header, token);
    strcat(extra_header, "\r\n");
    utils_print("extra_header:[%s]\n", extra_header);

    cl = uwsc_new(loop, hxt_url, ping_interval, extra_header);
    if (!cl)
    {
        utils_print("hxt webosocket init failed\n");
        return -1;
    }
        
	utils_print("Hxt webosocket start connect...\n");

    cl->onopen = uwsc_onopen;
    cl->onmessage = uwsc_onmessage;
    cl->onerror = uwsc_onerror;
    cl->onclose = uwsc_onclose;

    ev_signal_init(&signal_watcher, signal_cb, SIGINT);
    ev_signal_start(loop, &signal_watcher);

    ev_run(loop, 0);

    free(cl);
       
    return 0;
}