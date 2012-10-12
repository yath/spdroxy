#include "lstutil.h"
#include "misc.h"

#include <unistd.h>
#include <assert.h>

ll *ll_new(void) {
    ll *ret = malloc(sizeof(ll));
    if (!ret)
        efatal("malloc");
    ret->first = ret->last = NULL;
    return ret;
}

static struct ll_item *new_item(void *data) {
    struct ll_item *ret = malloc(sizeof(struct ll_item));
    if (!ret)
        efatal("malloc");
    ret->data = data;
    return ret;
}

static void ll_free(ll *l, ll_item_handler free_handler) {
    struct ll_item *item = l->first;
    while(item) {
        struct ll_item *next = item->next;
        if (free_handler)
            free_handler(item->data);
        free(item);
        item = next;
    }
    free(l);
}

void ll_append(ll *l, void *data) {
    struct ll_item *item = new_item(data);
    item->next = NULL;
    if (l->last) {
        assert(!l->last->next);
        l->last->next = item;
    }
    l->last = item;

    if (!l->first)
        l->first = l->last;
}

void ll_prepend(ll *l, void *data) {
    struct ll_item *item = new_item(data);
    item->next = l->first;
    l->first = item;
    if (!l->last)
        l->last = l->first;
}

void ll_each(ll *l, ll_item_handler handler) {
    debug("foo");
    assert(handler);
    for (struct ll_item *i = l->first; i; i = i->next)
        handler(i->data);
}
