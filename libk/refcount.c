#include <libk/refcount.h>

void refcount_init(refcount_t* r)
{
	atomic_int_init(&r->cnt, 1);
}

void refcount_init_zero(refcount_t* r)
{
	atomic_int_init(&r->cnt, 0);
}

int refcount_inc(refcount_t* r)
{
	return atomic_int_inc_return(&r->cnt);
}

int refcount_dec(refcount_t* r)
{
	return atomic_int_dec_return(&r->cnt);
}

int refcount_get(const refcount_t* r)
{
	return atomic_int_load(&r->cnt);
}
