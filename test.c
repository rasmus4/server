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
#include <sys/epoll.h>

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


    int epollfd = epoll_create1(0);

	// timer setup
	int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

    struct epoll_event timerEvent = {
        .events = EPOLLIN,
        .data.fd = timerfd
    };
    epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &timerEvent);

	struct itimerspec timspec = {
        .it_interval.tv_sec = 1,
        .it_value.tv_sec = 1   
    };
    struct itimerspec nospec = {0};
    struct itimerspec longspec = {
        .it_interval.tv_sec = 0,
        .it_value.tv_sec = 5  
    };

    struct itimerspec oldspec = {0};

	timerfd_settime(timerfd, 0, &timspec, 0);
    printf("started...\n");
    usleep(2500000);
    printf("pause!...\n");
    timerfd_settime(timerfd, 0, &longspec, &oldspec);

    printf("%ld, %ld\n", oldspec.it_value.tv_sec, oldspec.it_value.tv_nsec);

    uint64_t test;

    for (;;) {
        struct epoll_event event;
        if (epoll_wait(epollfd, &event, 1, -1) != 1) return -1;
        printf("pre-read\n");
        read(timerfd, &test, 8);
        printf("Hmm: %ld\n", test);
        usleep(2000000);
    }
    return 0;
}