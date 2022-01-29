#pragma once

#include <sys/types.h>

/* Routines the core kernel calls to set up and use hardware timer based callbacks at
 * interrupt context.
 *
 * Most of the time this is handled in the platform layer but a few standalone timer
 * implementations in other parts of the system may implement, depending on the
 * configuration of the build.
 */

typedef enum handler_return (*platform_timer_callback)(void *arg, lk_time_t now);

/* If the platform only supports a single monotonic (fixed rate) timer */
status_t platform_set_periodic_timer(platform_timer_callback callback, void *arg, lk_time_t interval);

/* If the platform implements a full dynamic (can be set to arbitary points in the future) timer */
status_t platform_set_oneshot_timer (platform_timer_callback callback, void *arg, lk_time_t interval);
void     platform_stop_timer(void);
