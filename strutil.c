#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "strutil.h"

#define ALLOC_FACTOR 1.2
#define DEFAULT_ALLOC 3

static void ensure_total_space(str *s, size_t space) {
    //debug("ensure total: %d", space);
    size_t before = s->space;
    if (!s->space)
        s->space = space < 5 ? 5 : space;
    while (s->space < space)
        s->space *= ALLOC_FACTOR;
    if (s->space != before) {
        if (!(s->buf = realloc(s->buf, s->space)))
            efatal("realloc");
        s->pos = s->buf + s->size;
    }
}

static void ensure_free_space(str *s, size_t space) {
   // debug("ensure free: %d", space);
    ensure_total_space(s, s->size + space);
}

str *str_new_cp(const char *cp) {
    str *ret = str_new();
    str_set_cp(ret, cp);
    return ret;
}


void str_set_cpl(str *s, const char *cp, size_t len) {
    ensure_total_space(s, len);
    memcpy(s->buf, cp, len);
    s->size = len;
    s->pos = s->buf + len;
}

void str_set_cp(str *s, const char *cp) {
    str_set_cpl(s, cp, strlen(cp));
}

void str_free(str *s) {
    if (!s)
        return;
    if (s->buf)
        free(s->buf);
    free(s);
}

void str_cat_cpl(str *s, const char *cp, size_t len) {
    //debug("cat '%s' (%d)", cp, len);
    ensure_free_space(s, len);
//    debug("s->buf = %p, s->pos = %p, *s->pos = '%c'", s->buf, s->pos, ' ');
//    debug("pos-buf = %d, size = %d", s->pos - s->buf, s->size);
    memcpy(s->pos, cp, len);
    s->pos += len;
    s->size += len;
}

void str_cat_cp(str *s, const char *cp) {
    str_cat_cpl(s, cp, strlen(cp));
}

const char *str_get_cp(str *s) {
    str_cat_cpl(s, "", 1); /* append \0 */
    s->pos--;
    s->size--;
    return s->buf;
}

str *str_new(void) {
    str *ret = malloc(sizeof(str));
    if (!ret)
        efatal("malloc");
    ret->pos = ret->buf = NULL;
    ret->size = ret->space = 0;
    return ret;
}
