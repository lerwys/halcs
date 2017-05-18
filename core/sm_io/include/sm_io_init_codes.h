/*
 * Copyright (C) 2017 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _SM_IO_INIT_CODES_H_
#define _SM_IO_INIT_CODES_H_

/* Messaging OPCODES */
#define INIT_OPCODE_TYPE                     uint32_t
#define INIT_OPCODE_SIZE                     (sizeof (INIT_OPCODE_TYPE))

#define INIT_OPCODE_SET_GET_CHECK            0
#define INIT_NAME_SET_GET_CHECK              "check"
#define INIT_OPCODE_END                      1

/* Messaging Reply OPCODES */
#define INIT_REPLY_TYPE                      uint32_t
#define INIT_REPLY_SIZE                      (sizeof (INIT_REPLY_TYPE))

#define INIT_OK                              0   /* Operation was successful */
#define INIT_ERR                             1   /* Generic error */

#endif
