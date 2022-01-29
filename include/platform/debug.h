#pragma once

#include <stdbool.h>

/* Standard console output routines that may be buffered */
void platform_dputc(char c);
int platform_dgetc(char *c, bool wait);
