#ifndef _LSTUTIL_H
#define _LSTUTIL_H
typedef void (*ll_item_handler)(void *data);

struct ll_data {
    struct ll_item *first;
    struct ll_item *last;
    ll_item_handler *fh;
};

typedef struct ll_data ll;

struct ll_item {
    void *data;
    struct ll_item *next;
};

ll *ll_new(void);
static void ll_free(ll *l, ll_item_handler free_handler);
void ll_append(ll *l, void *data);
void ll_prepend(ll *l, void *data);
void ll_each(ll *l, ll_item_handler handler);
#endif
