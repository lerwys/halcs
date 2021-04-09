/*
 * Copyright (C) 2015  LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "halcs_server.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, DEV_IO, "[dev_io:device]",        \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)       \
    ASSERT_HAL_ALLOC(ptr, DEV_IO, "[dev_io:device]",                \
            devio_err_str(DEVIO_ERR_ALLOC),                         \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                    \
    CHECK_HAL_ERR(err, DEV_IO, "[dev_io:device]",                   \
            devio_err_str (err_type))

const size_t NUM_MAX_SLOTS              = __NUM_MAX_SLOTS;
const size_t NUM_MAX_HALCS_PER_SLOT     = __NUM_MAX_HALCS_PER_SLOT;
const size_t NUM_MAX_HALCSS             = __NUM_MAX_HALCSS;

/*********************** Static AFCv3 FPGA layout ***********************/

const size_t SDB_ADDRESS                = 0x00000000;

/**************************** Device map ********************************/

const board_device_map_t board_device_map[__NUM_MAX_HALCSS+1] = {
         /* board, halcs*/
    /* 0 (INVALID)  */ {-1, -1},
    /* 1            */ {1,   0},
    /* 2            */ {1,   1},
};