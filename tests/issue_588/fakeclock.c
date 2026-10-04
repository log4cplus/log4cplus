#define _GNU_SOURCE
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static _Atomic long long epoch_override;

void issue588_set_epoch(long long epoch)
{
    atomic_store(&epoch_override, epoch);
}

static long long fake_epoch(void)
{
    long long epoch = atomic_load(&epoch_override);
    if (epoch)
        return epoch;
    const char *configured = getenv("ISSUE588_EPOCH");
    return configured ? strtoll(configured, NULL, 10) : 0;
}

int clock_gettime(clockid_t clock, struct timespec *value)
{
    long long epoch = fake_epoch();
    if (epoch && (clock == CLOCK_REALTIME || clock == CLOCK_REALTIME_COARSE)) {
        value->tv_sec = epoch;
        value->tv_nsec = 0;
        return 0;
    }
    return syscall(SYS_clock_gettime, clock, value);
}

int gettimeofday(struct timeval *value, void *zone)
{
    long long epoch = fake_epoch();
    if (epoch) {
        value->tv_sec = epoch;
        value->tv_usec = 0;
        return 0;
    }
    return syscall(SYS_gettimeofday, value, zone);
}

time_t time(time_t *result)
{
    struct timespec value;
    clock_gettime(CLOCK_REALTIME, &value);
    if (result)
        *result = value.tv_sec;
    return value.tv_sec;
}
