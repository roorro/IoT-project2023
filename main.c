#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "ztimer.h"
#include "shell.h"
#include "msg.h"

#include "net/gcoap.h"
#include "fmt.h"

#include "isl29020.h"
#include "isl29020_params.h"

#define MAIN_QUEUE_SIZE (8)

//Dcleare isl light sensor
static isl29020_t isl29020;

static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context);

static ssize_t _isl29020_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx);

/* CoAP resources. Must be sorted by path (ASCII order). */
static const coap_resource_t _resources[] = {
    { "/light", COAP_GET, _isl29020_handler, NULL },
};

/*
static void _isl29020_usage(char *cmd)
{
    printf("usage: %s\n", cmd);
}*/

static ssize_t _isl29020_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    /* Implement light sensor GET handler */
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    char response[32];
    
    sprintf(response, "Light value: %5i LUX\n", isl29020_read(&isl29020));

    if (pdu->payload_len >= strlen(response)) {
        memcpy(pdu->payload, response, strlen(response));
        return resp_len + strlen(response);
    }
    else {
        puts("gcoap: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

static gcoap_listener_t _listener = {
    &_resources[0],
    ARRAY_SIZE(_resources),
    _encode_link,
    NULL,
    NULL
};

/* Adds link format params to resource list */
static ssize_t _encode_link(const coap_resource_t *resource, char *buf,
                            size_t maxlen, coap_link_encoder_ctx_t *context) {
    ssize_t res = gcoap_encode_link(resource, buf, maxlen, context);

    return res;
}

void gcoap_cli_init(void)
{
    gcoap_register_listener(&_listener);
}

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int gcoap_cli_cmd(int argc, char **argv);
extern void gcoap_cli_init(void);

static const shell_command_t commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { NULL, NULL, NULL}
};

int main(void)
{
    puts("My first RIOT application");

    // Initialize ligh sensor
    if (isl29020_init(&isl29020, &isl29020_params[0]) == 0) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    gcoap_cli_init();
    puts("gcoap example app");

    /* Everything is ready, let's start the shell now */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}