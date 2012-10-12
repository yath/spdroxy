#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "misc.h"
#include "http_parser.h"
#include "strutil.h"
#include "lstutil.h"

#define LISTEN_PORT 8001

const char *addrstr(struct in6_addr *addr) {
    static char buf[INET6_ADDRSTRLEN];
    return inet_ntop(AF_INET6, (struct sockaddr_in *)addr, buf, sizeof(buf));
}

int get_listen_fd(int port) {
    int listenfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listenfd < 0)
        efatal("socket");

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        efatal("setsockopt");

    struct sockaddr_in6 sa = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(port),
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0,
        .sin6_addr = IN6ADDR_ANY_INIT
    };
    if (bind(listenfd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        effatal("bind(%d)", ntohs(sa.sin6_port));

    if (listen(listenfd, 10) < 0)
        efatal("listen");

    return listenfd;
}

char *substr(const char *str, size_t start, size_t len) {
    char *buf = malloc(len+1);
    if (!buf)
        efatal("malloc");
    memcpy(buf, str+start, len);
    buf[len] = '\0';
    return buf;
}

struct header {
    str *key;
    str *value;
};

struct state {
    int fd;
    str *url;
    ll *headers;
    struct header *cur_header;
};

#define X(x) debug("parser function " x)
#define Y(x) do { char *s = substr(at, 0, len); debug("parser function " x " - data(%d): ->%s<-", len, s); free(s); } while(0)
int on_message_begin(http_parser *parser) {
    debug("initializing new state");

    struct state *s = malloc(sizeof(struct state));
    if (!s)
        efatal("malloc");
    s->url = str_new();
    s->headers = ll_new();
    s->cur_header = NULL;
    parser->data = s;
    return 0;
}

int on_url(http_parser *parser, const char *at, size_t len) {
    Y("on_url");
    struct state *s = parser->data;
    str_cat_cpl(s->url, at, len);
    debug("on_url now: %s", str_get_cp(s->url));
    return 0;
}

void flush_header(struct state *s) {
    debug("flush_header");
    assert(s->cur_header->key && s->cur_header->value);
    assert(s->headers);
    struct header *h = s->cur_header;
    debug("==> flush field '%s', value '%s'", str_get_cp(h->key), str_get_cp(h->value));
    ll_append(s->headers, s->cur_header);
    s->cur_header = NULL;
}

int on_header_field(http_parser *parser, const char *at, size_t len) {
    Y("header field");
    struct state *s = parser->data;
    struct header *ch = s->cur_header;
    if (ch && ch->value) {
        flush_header(s);
        ch = NULL;
    }

    if (!ch) {
        ch = s->cur_header = malloc(sizeof(struct header));
        if (!ch)
            efatal("malloc");
        ch->key = str_new();
        ch->value = NULL;
    }
    str_cat_cpl(ch->key, at, len);
    return 0;
}

int on_header_value(http_parser *parser, const char *at, size_t len) {
    Y("header value");
    struct state *s = parser->data;
    struct header *ch = s->cur_header;
    assert(ch);
    if (!ch->value)
        ch->value = str_new();
    str_cat_cpl(ch->value, at, len);
    return 0;
}

int on_headers_complete(http_parser *parser) {
    X("headers complete");
    flush_header(parser->data);
    return 0;
}

void dump_header(struct header *h) {
    debug("Header field '%s', value '%s'", str_get_cp(h->key), str_get_cp(h->value));
}

int on_message_complete(http_parser *parser) {
    struct state *s = parser->data;
    X("message_complete");
    ll_each(s->headers, dump_header);
    debug("URL: %s", str_get_cp(s->url));

    return 0;
}

void handle_client(int fd) {
    char buf[5];
    int ret;
    http_parser_settings ps = {
        .on_message_begin = on_message_begin,
        .on_url = on_url,
        .on_header_field = on_header_field,
        .on_header_value = on_header_value,
        .on_headers_complete = on_headers_complete,
        .on_message_complete = on_message_complete,
    };

    http_parser p;

    http_parser_init(&p, HTTP_REQUEST);

    while ((ret = read(fd, buf, sizeof(buf))) > 0) {
        int x = http_parser_execute(&p, &ps, buf, ret);
        debug("http_parser_execute returned %d", x);
    }
    if (ret == 0) {
        debug("eof");
    } else {
        fatal("read");
    }
}

int main(void) {
    int lfd = get_listen_fd(LISTEN_PORT);

    debug("listening on port %d", LISTEN_PORT);

    struct sockaddr_in6 cli;
    socklen_t clilen = sizeof(cli);
    int connfd;
    while ((connfd = accept(lfd, (struct sockaddr *)&cli, &clilen)) >= 0) {
        printf("client connected: %s\n", addrstr(&cli.sin6_addr));
        handle_client(connfd);
        close(connfd);
    }

    return 0;
}
