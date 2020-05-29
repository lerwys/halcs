/*
 * Copyright (C) 2015  LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "halcs_server.h"

#if defined __BOARD_AFCV3__ && !defined __BOARD_AFCV3_1__

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, DEV_IO, "[dev_io:epics]",         \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)       \
    ASSERT_HAL_ALLOC(ptr, DEV_IO, "[dev_io:epics]",                 \
            devio_err_str(DEVIO_ERR_ALLOC),                         \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                    \
    CHECK_HAL_ERR(err, DEV_IO, "[dev_io:epics]",                    \
            devio_err_str (err_type))

const board_epics_map_t board_epics_map[NUM_MAX_HALCSS+1] = {
         /* board, halcs*/
    /* 0 (INVALID)  */ {-1, -1},
    /* 1            */ {1,   0},
    /* 2            */ {1,   1},
    /* 3            */ {2,   0},
    /* 4            */ {2,   1},
    /* 5            */ {3,   0},
    /* 6            */ {3,   1},
    /* 7            */ {4,   0},
    /* 8            */ {4,   1},
    /* 9            */ {5,   0},
    /* 10           */ {5,   1},
    /* 11           */ {6,   0},
    /* 12           */ {6,   1},
    /* 13           */ {7,   0},
    /* 14           */ {7,   1},
    /* 15           */ {8,   0},
    /* 16           */ {8,   1},
    /* 17           */ {9,   0},
    /* 18           */ {9,   1},
    /* 19           */ {10,  0},
    /* 20           */ {10,  1},
    /* 21           */ {11,  0},
    /* 22           */ {11,  1},
    /* 23           */ {12,  0},
    /* 24           */ {12,  1}
};

/* Reverse halcs <-> EPICS board mapping */
const board_epics_rev_map_t board_epics_rev_map[NUM_MAX_SLOTS+1][NUM_MAX_HALCS_PER_SLOT] = {
     /* board, halcs */    /* halcs_id */
    /* 0, 0 (INVALID) */ {{-1},
    /* 0, 1 (INVALID) */  {-1}},
    /* 1, 0           */ {{1},
    /* 1, 1           */  {2}},
    /* 2, 0           */ {{3},
    /* 2, 1           */  {4}},
    /* 3, 0           */ {{5},
    /* 3, 1           */  {6}},
    /* 4, 0           */ {{7},
    /* 4, 1           */  {8}},
    /* 5, 0           */ {{9},
    /* 5, 1           */  {10}},
    /* 6, 0           */ {{11},
    /* 6, 1           */  {12}},
    /* 7, 0           */ {{13},
    /* 7, 1           */  {14}},
    /* 8, 0           */ {{15},
    /* 8, 1           */  {16}},
    /* 9, 0           */ {{17},
    /* 9, 1           */  {18}},
    /* 10, 0          */ {{19},
    /* 10, 1          */  {20}},
    /* 11, 0          */ {{21},
    /* 11, 1          */  {22}},
    /* 12, 0          */ {{23},
    /* 12, 1          */  {24}}
};

const board_epics_opts_t board_epics_opts[NUM_MAX_SLOTS+1][NUM_MAX_HALCS_PER_SLOT] = {
    /* board, halcs */    /* halcs_id */
    /* 0, 0 (INVALID) */ {{.telnet_port = -1,
                           .telnet_afe_port = -1},
    /* 0, 1 (INVALID) */  {.telnet_port = -1,
                           .telnet_afe_port = -1}},
    /* 1, 0           */ {{.telnet_port = 20000 + 1,
                           .telnet_afe_port = 20100 + 1},
    /* 1, 1           */  {.telnet_port = 20000 + 2,
                           .telnet_afe_port = 20100 + 2}},
    /* 2, 0           */ {{.telnet_port = 20000 + 3,
                           .telnet_afe_port = 20100 + 3},
    /* 2, 1           */  {.telnet_port = 20000 + 4,
                           .telnet_afe_port = 20100 + 4}},
    /* 3, 0           */ {{.telnet_port = 20000 + 5,
                           .telnet_afe_port = 20100 + 5},
    /* 3, 1           */  {.telnet_port = 20000 + 6,
                           .telnet_afe_port = 20100 + 6}},
    /* 4, 0           */ {{.telnet_port = 20000 + 7,
                           .telnet_afe_port = 20100 + 7},
    /* 4, 1           */  {.telnet_port = 20000 + 8,
                           .telnet_afe_port = 20100 + 8}},
    /* 5, 0           */ {{.telnet_port = 20000 + 9,
                           .telnet_afe_port = 20100 + 9},
    /* 5, 1           */  {.telnet_port = 20000 + 10,
                           .telnet_afe_port = 20100 + 10}},
    /* 6, 0           */ {{.telnet_port = 20000 + 11,
                           .telnet_afe_port = 20100 + 11},
    /* 6, 1           */  {.telnet_port = 20000 + 12,
                           .telnet_afe_port = 20100 + 12}},
    /* 7, 0           */ {{.telnet_port = 20000 + 13,
                           .telnet_afe_port = 20100 + 13},
    /* 7, 1           */  {.telnet_port = 20000 + 14,
                           .telnet_afe_port = 20100 + 14}},
    /* 8, 0           */ {{.telnet_port = 20000 + 15,
                           .telnet_afe_port = 20100 + 15},
    /* 8, 1           */  {.telnet_port = 20000 + 16,
                           .telnet_afe_port = 20100 + 16}},
    /* 9, 0           */ {{.telnet_port = 20000 + 17,
                           .telnet_afe_port = 20100 + 17},
    /* 9, 1           */  {.telnet_port = 20000 + 18,
                           .telnet_afe_port = 20100 + 18}},
    /* 10, 0          */ {{.telnet_port = 20000 + 19,
                           .telnet_afe_port = 20100 + 19},
    /* 10, 1          */  {.telnet_port = 20000 + 20,
                           .telnet_afe_port = 20100 + 20}},
    /* 11, 0          */ {{.telnet_port = 20000 + 21,
                           .telnet_afe_port = 20100 + 21},
    /* 11, 1          */  {.telnet_port = 20000 + 22,
                           .telnet_afe_port = 20100 + 22}},
    /* 12, 0          */ {{.telnet_port = 20000 + 23,
                           .telnet_afe_port = 20100 + 23},
    /* 12, 1          */  {.telnet_port = 20000 + 24,
                           .telnet_afe_port = 20100 + 24}},
};

#endif
