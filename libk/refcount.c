#include <kernel/atomic.h>
#include <libk/refcount.h>

void refcount_init(refcount_t* r)
{
	r->cnt = 1;
}

void refcount_inc(refcount_t* r)
{
	atomic_inc_int(r->cnt);
}

void refcount_dec(refcount_t* r)
{
	atomic_dec_int(r->cnt);
}

int refcount_get(refcount_t* r)
{
	int cnt;
	atomic_get_int(r->cnt, cnt);

	return cnt;
}
