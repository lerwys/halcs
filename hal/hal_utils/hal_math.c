/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU LGPL, version 3 or any later version.
 */

#include "hal_math.h"
#include <inttypes.h>

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, HAL_UTILS, "[halultis:math]",         \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)           \
    ASSERT_HAL_ALLOC(ptr, HAL_UTILS, "[halutils:math]",                 \
            halutils_err_str(HALUTILS_ERR_ALLOC),                       \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                        \
    CHECK_HAL_ERR(err, HAL_UTILS, "[halutils:math]",                    \
            halutils_err_str (err_type))

static uint64_t _div_u64_rem (uint64_t dividend, uint32_t divisor,
        uint32_t *remainder);

/* Cuts from Linux kernel */

uint64_t div_u64_rem (uint64_t dividend, uint32_t divisor,
        uint32_t *remainder)
{
    return _div_u64_rem (dividend, divisor, remainder);
}

uint64_t div_u64 (uint64_t dividend, uint32_t divisor)
{
    uint32_t remainder;
    return div_u64_rem (dividend, divisor, &remainder);
}

uint64_t div64_u64 (uint64_t dividend, uint64_t divisor)
{
    return dividend / divisor;
}

int64_t div64_s64 (int64_t dividend, int64_t divisor)
{
    return dividend / divisor;
}

/***************************** Static Functions ******************************/

static uint64_t _div_u64_rem (uint64_t dividend, uint32_t divisor,
        uint32_t *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

