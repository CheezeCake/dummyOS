/*
 *                ...
 *              ;::::;
 *            ;::::; :;
 *          ;:::::'   :;
 *         ;:::::;     ;.
 *        ,:::::'       ;           OOO\
 *        ::::::;       ;          OOOOO\
 *        ;:::::;       ;         OOOOOOOO
 *       ,;::::::;     ;'         / OOOOOOO
 *     ;:::::::::`. ,,,;.        /  / DOOOOOO
 *   .';:::::::::::::::::;,     /  /     DOOOO
 *  ,::::::;::::::;;;;::::;,   /  /        DOOO
 * ;`::::::`'::::::;;;::::: ,#/  /          DOOO
 * :`:::::::`;::::::;;::: ;::#  /            DOOO
 * ::`:::::::`;:::::::: ;::::# /              DOO
 * `:`:::::::`;:::::: ;::::::#/               DOO
 *  :::`:::::::`;; ;:::::::::##                OO
 *  ::::`:::::::`;::::::::;:::#                OO
 *  `:::::`::::::::::::;'`:;::#                O
 *   `:::::`::::::::;' /  / `:#
 *    ::::::`:::::;'  /  /   `#
 */

#include <kernel/locking/semaphore.h>
#include <kernel/sched/sched.h>
#include <kernel/kassert.h>
#include <kernel/kthread.h>
#include <libk/list.h>

static void reaper_work(void* data);

static LIST_DEFINE(reaper_list);
static sem_t sem;

void reaper_init(void)
{
	struct thread* reaper;

	kassert(kthread_create("[reaper]", reaper_work, NULL, &reaper) == 0);
	reaper->priority = SCHED_PRIORITY_LEVEL_MAX;
	kassert(sched_add_thread(reaper) == 0);

	semaphore_init(&sem, 0);
}

void reaper_reap(struct thread* thr)
{
	kassert(thread_get_ref(thr) == 0);
	list_push_back(&reaper_list, &thr->p_thr_list);
	semaphore_up(&sem);
}

static void reaper_work(void* data)
{
	struct thread* marked;

	while (1) {
		semaphore_down(&sem);

		marked = list_entry(list_front(&reaper_list), struct thread, p_thr_list);
		list_pop_front(&reaper_list);

		__thread_destroy(marked);
	}
}
