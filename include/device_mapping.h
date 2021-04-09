/*
 * Copyright (C) 2015 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _DEVICE_MAPPING_H_
#define _DEVICE_MAPPING_H_

#ifdef __cplusplus
extern "C" {
#endif

/* HALCS reverse mappping structure */
typedef struct {
    int dev_id;
    int smio_id;
} board_device_map_t;

#ifdef __cplusplus
}
#endif

#endif