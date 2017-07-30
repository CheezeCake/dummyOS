#include <stdint.h>

#include <kernel/locking/semaphore.h>

#define MAX_SEMAPHORES 256
#define SEM_AVAILABLE_SIZE (MAX_SEMAPHORES / 32)

#define semaphore_available(sem) (((sem_available[sem / 32] >> (sem % 32)) & 1) == 0)

static int sem_values[MAX_SEMAPHORES] = { 0 };
static uint32_t sem_available[SEM_AVAILABLE_SIZE] = { 0 };

int semaphore_create(sem_t* sem, int n)
{
	int avail_idx = 0;
	while (sem_available[avail_idx] == 0xffffffff)
		++avail_idx;

	if (avail_idx == MAX_SEMAPHORES)
		return -1;

	const uint32_t avail = sem_available[avail_idx];
	int shift = 0;
	while (shift < 32 && ((avail >> shift) & 1) == 1)
		++shift;

	const int sem_idx = avail_idx * 32 + shift;
	sem_values[sem_idx] = n;
	sem_available[avail_idx] |= (1 << shift);

	*sem = sem_idx;

	return 1;
}

int semaphore_destroy(sem_t* sem)
{
	const int avail_idx = *sem / 32;
	const int shift = *sem % 32;

	// trying to destroy an available semaphore
	if (((sem_available[avail_idx] >> shift) & 1) == 0)
		return -1;

	sem_values[*sem] = 0;
	sem_available[avail_idx] &= ~(1 << shift);

	return 0;
}

int semaphore_up(const sem_t* sem)
{
	if (semaphore_available(*sem))
		return -1;

	++sem_values[*sem];

	return 0;

}

int semaphore_down(const sem_t* sem)
{
	if (semaphore_available(*sem))
		return -1;

	--sem_values[*sem];

	return 0;
}

int semaphore_getvalue(const sem_t* sem, int* val)
{
	if (semaphore_available(*sem))
		return -1;

	*val = sem_values[*sem];

	return 0;

}
