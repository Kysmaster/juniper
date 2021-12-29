/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#pragma once

#define SHIFT_4K        (12)
#define SHIFT_16K       (14)
#define SHIFT_64K       (16)

/* arm specific stuff */
#define PAGE_SIZE_SHIFT SHIFT_4K
#define USER_PAGE_SIZE_SHIFT SHIFT_4K

#define PAGE_SIZE (1UL << PAGE_SIZE_SHIFT)
#define USER_PAGE_SIZE (1UL << USER_PAGE_SIZE_SHIFT)

#define CACHE_LINE 64
