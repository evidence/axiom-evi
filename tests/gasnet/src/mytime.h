/*!
 * \file mytime.h
 *
 * \version     v1.2
 * \date        2016-12-20
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef __MY_TIME_H
#define __MY_TIME_H

#include <time.h>

static inline long int time_diff_sec(struct timespec* new, struct timespec* old)
{
	return ((new->tv_sec)-(old->tv_sec)) + ((new->tv_nsec)-(old->tv_nsec))/1000000000;
}

static inline long int time_diff_msec(struct timespec* new, struct timespec* old)
{
	return ((new->tv_sec)-(old->tv_sec))*1000 + ((new->tv_nsec)-(old->tv_nsec))/1000000;
}

static inline long int time_sec(struct timespec* t)
{
	return (t->tv_sec) + ((t->tv_nsec)/1000000000);
}

static inline long int time_msec(struct timespec* t)
{
	return ((t->tv_sec)*1000) + ((t->tv_nsec)/1000000);
}

static inline void time_gettime(struct timespec *t)
{
    clock_gettime(CLOCK_REALTIME_COARSE,t);
}

#endif
