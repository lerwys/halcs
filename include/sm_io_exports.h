/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _SM_IO_EXPORTS_H_
#define _SM_IO_EXPORTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SMIO_OWNER_TYPE                         smio_t
#define SMIO_EXP_OWNER(owner)                   ((smio_t *) owner)

#ifdef __cplusplus
}
#endif

#endif
