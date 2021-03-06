/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 * Parts taken from lwIP debug system
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _ERRHAND_LOCAL_PRINT_H_
#define _ERRHAND_LOCAL_PRINT_H_

#ifdef __cplusplus
extern "C" {
#endif

char *errhand_lprint_vprintf (const char *format, va_list argptr);
void errhand_lprint_zmq_msg (zmsg_t *msg, FILE *file);

#ifdef __cplusplus
}
#endif

#endif
