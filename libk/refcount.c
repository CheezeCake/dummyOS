#include <libk/refcount.h>

void refcount_init(refcount_t* r)
{
	atomic_int_init(&r->cnt, 1);
}

void refcount_inc(refcount_t* r)
{
	atomic_int_inc(&r->cnt);
}

void refcount_dec(refcount_t* r)
{
	atomic_int_dec(&r->cnt);
}

int refcount_get(const refcount_t* r)
{
	return atomic_int_get(&r->cnt);
}
