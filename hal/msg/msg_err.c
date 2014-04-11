/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU LGPL, version 3 or any later version.
 */

/* Error definitions and output stringification based on the work available
 * at the libsllp project repository: https://github.com/brunoseivam/libsllp */

#include "msg_err.h"

static const char *msg_err [MSG_ERR_END] =
{
    [MSG_SUCCESS]              = "Success",
    [MSG_ERR]                  = "Messaging error",
    [MSG_ERR_ALLOC]            = "Could not allocate message",
    [MSG_FUNC_NOT_IMPL]        = "Function not implemented"
};

/* Convert enumeration type to string */
const char * msg_err_str (msg_err_e err)
{
    return msg_err [err];
}
