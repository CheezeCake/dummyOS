#ifndef _LIBK_REFCOUNT_H_
#define _LIBK_REFCOUNT_H_

typedef struct refcount {
	int cnt;
} refcount_t;

void refcount_init(refcount_t* r);
void refcount_inc(refcount_t* r);
void refcount_dec(refcount_t* r);
int refcount_get(refcount_t* r);

#endif
