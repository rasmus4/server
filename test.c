// System Timer Granularity Analysis
// Developed by: Brandon C. Schlinker (Inbound5 / Develop5)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/prctl.h>

#define __STDC_FORMAT_MACROS

struct timespec diff(struct timespec start, struct timespec end, char *negative)
{
    *negative = ' ';
    struct timespec smaller, larger, temp;
    if (end.tv_sec < start.tv_sec) {
        smaller = end;
        larger = start;
        *negative = '-';
    } else if (end.tv_sec == start.tv_sec) {
        if ((end.tv_nsec < start.tv_nsec)) {
            smaller = end;
            larger = start;
            *negative = '-';
        } else {
            smaller = start;
            larger = end;
        }
    } else {
        smaller = start;
        larger = end;
    }
    if ((larger.tv_nsec - smaller.tv_nsec) < 0) {
        temp.tv_sec = larger.tv_sec - smaller.tv_sec - 1;
        temp.tv_nsec = 1000000000 + larger.tv_nsec - smaller.tv_nsec;
    } else {
        temp.tv_sec = larger.tv_sec - smaller.tv_sec;
        temp.tv_nsec = larger.tv_nsec - smaller.tv_nsec;
    }
    return temp;
}

// perform timer accuracy analysis
int main(int argc, char *argv[])
{
	// set real-time priority
	//struct sched_param schedparm;
	//memset(&schedparm, 0, sizeof(schedparm));
	//schedparm.sched_priority = 1; // highest rt priority
	//sched_setscheduler(0, SCHED_FIFO, &schedparm);

	// set timer slack equal to 1 nanosecond
	//prctl(PR_SET_TIMERSLACK, 1);

	// timer setup
	int timerfd = timerfd_create(CLOCK_MONOTONIC,0);
	struct itimerspec timspec;
	bzero(&timspec, sizeof(timspec));
	timspec.it_interval.tv_sec = 1;
	timspec.it_interval.tv_nsec = 0;//10000000;
	timspec.it_value.tv_sec = 0;
	timspec.it_value.tv_nsec = 1;

	timerfd_settime(timerfd, 0, &timspec, 0);

    struct timespec time1, time2;
    clock_gettime(CLOCK_MONOTONIC, &time1);
    int first = 1;
    long long test;
    int iterations = 0;

	// poll the timerfd, if we missed an expiration, increment
	while(read(timerfd, &test, sizeof(test))){
        if (first) {
            clock_gettime(CLOCK_MONOTONIC, &time1);
            first = 0;
        } else {
            ++iterations;
            clock_gettime(CLOCK_MONOTONIC, &time2);
            char negative;
            struct timespec ts = diff(time1, time2, &negative);
            struct timespec expected;
            expected.tv_sec = iterations;
            expected.tv_nsec = 0;
            
            struct timespec off = diff(ts, expected, &negative);

            //time1 = time2;
            printf("%c%lld.%.9ld\n", negative, (long long)off.tv_sec, off.tv_nsec);
            //printf("%c%lld.%.9ld\n", negative, (long long)ts.tv_sec, ts.tv_nsec);
        }
	}
	// return success
	return(0);
}