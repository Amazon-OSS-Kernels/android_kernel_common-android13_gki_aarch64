/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2013 Google Inc.
 */

#ifndef __STDLIB_H_
#define __STDLIB_H_

#include <malloc.h>


#define abs(x) ({								\
		long ret;								\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);						\
			ret = (__x < 0) ? -__x : __x;		\
		} else {								\
			int __x = (x);						\
			ret = (__x < 0) ? -__x : __x;		\
		}										\
		ret;									\
	})

/* lib/rand.c */
#define RAND_MAX -1U
void srand(unsigned int seed);
unsigned int rand(void);
unsigned int rand_r(unsigned int *seedp);


#endif /* __STDLIB_H_ */
