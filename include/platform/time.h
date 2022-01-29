#pragma once

#include <sys/types.h>

/* Routines to read the current system time since an arbitrary point in the
 * past. Usually system boot time.
 */

/* Time in units of milliseconds */
lk_time_t current_time(void);

/* Time in units of microseconds */
lk_bigtime_t current_time_hires(void);
