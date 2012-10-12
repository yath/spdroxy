#ifndef _STRUTIL_H
#define _STRUTIL_H
struct str_data {
    char *buf;
    char *pos;
    size_t size;
    size_t space;
};

typedef struct str_data str;
str *str_new_cp(const char *cp);
void str_set_cpl(str *s, const char *cp, size_t len);
void str_set_cp(str *s, const char *cp);
str *str_new(void);
void str_free(str *s);
void str_cat_cpl(str *s, const char *cp, size_t len);
void str_cat_cp(str *s, const char *cp);
const char *str_get_cp(str *s);
#endif
