#pragma once

#include <limits.h>
#include <stdint.h>
#include <stddef.h>

typedef long long     off_t;

typedef int status_t;

typedef uintptr_t addr_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;

typedef int kobj_id;

typedef uint32_t lk_time_t;
typedef unsigned long long lk_bigtime_t;
#define INFINITE_TIME UINT32_MAX

#define TIME_GTE(a, b) ((int32_t)((a) - (b)) >= 0)
#define TIME_LTE(a, b) ((int32_t)((a) - (b)) <= 0)
#define TIME_GT(a, b) ((int32_t)((a) - (b)) > 0)
#define TIME_LT(a, b) ((int32_t)((a) - (b)) < 0)

enum handler_return {
	INT_NO_RESCHEDULE = 0,
	INT_RESCHEDULE,
};

typedef signed long int ssize_t;
