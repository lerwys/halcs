/*
 * Copyright (C) 2016 LNLS (www.lnls.br)
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
    ASSERT_HAL_TEST(test_boolean, SM_CH, "[sm_ch:isla216p]",            \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)           \
    ASSERT_HAL_ALLOC(ptr, SM_CH, "[sm_ch:isla216p]",                    \
            smch_err_str(SMCH_ERR_ALLOC),                               \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                        \
    CHECK_HAL_ERR(err, SM_CH, "[sm_ch:isla216p]",                       \
            smch_err_str (err_type))

#define SMCH_ISLA216P_WAIT_TRIES            10
#define SMCH_ISLA216P_NAME                  "ISLA216P"
#define SMCH_ISLA216P_USECS_WAIT            1000
#define SMCH_ISLA216P_WAIT(usecs)           usleep(usecs)
#define SMCH_ISLA216P_WAIT_DFLT             SMCH_ISLA216P_WAIT(SMCH_ISLA216P_USECS_WAIT)

struct _smch_isla216p_t {
    smpr_t *proto;                    /* PROTO protocol object */
};

static smch_err_e _smch_isla216p_init (smch_isla216p_t *self);
/* Read/Write 1-byte functions */
static ssize_t _smch_isla216p_write_8 (smch_isla216p_t *self, uint8_t addr,
        const uint8_t *data);
static ssize_t _smch_isla216p_read_8 (smch_isla216p_t *self, uint8_t addr,
        uint8_t *data);

/* Creates a new instance of the SMCH ISLA216P */
smch_isla216p_t * smch_isla216p_new (smio_t *parent, uint64_t base,
        const smpr_proto_ops_t *reg_ops, int verbose)
{
    UNUSED(verbose);
    assert (parent);

    smch_isla216p_t *self = (smch_isla216p_t *) zmalloc (sizeof *self);
    ASSERT_ALLOC(self, err_self_alloc);

    self->proto = smpr_new (SMCH_ISLA216P_NAME, parent, reg_ops, verbose);
    ASSERT_ALLOC(self->proto, err_proto_alloc);

    /* Initalize the PROTO protocol */
    int smpr_err = smpr_open (self->proto, base, NULL /* Default parameters are fine */);
    ASSERT_TEST(smpr_err == 0, "Could not initialize SMPR protocol", err_smpr_init);

    DBE_DEBUG (DBG_SM_CH | DBG_LVL_INFO, "[sm_ch:isla216p] Created instance of SMCH\n");

    smch_err_e err =  _smch_isla216p_init (self);
    ASSERT_TEST(err == SMCH_SUCCESS, "Could not initialize ISLA216P", err_smch_init);

    return self;

err_smch_init:
    smpr_release (self->proto);
err_smpr_init:
    smpr_destroy (&self->proto);
err_proto_alloc:
    free (self);
err_self_alloc:
    return NULL;
}

/* Destroy an instance of the SMCH ISLA216P */
smch_err_e smch_isla216p_destroy (smch_isla216p_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        smch_isla216p_t *self = *self_p;

        smpr_release (self->proto);
        smpr_destroy (&self->proto);
        free (self);
        *self_p = NULL;
    }

    return SMCH_SUCCESS;
}

smch_err_e smch_isla216p_write_8 (smch_isla216p_t *self, uint8_t addr,
        const uint8_t *data)
{
    return (_smch_isla216p_write_8 (self, addr, data) == sizeof(uint8_t))?
        SMCH_SUCCESS : SMCH_ERR_RW_SMPR;
}

smch_err_e smch_isla216p_read_8 (smch_isla216p_t *self, uint8_t addr,
        uint8_t *data)
{
    return (_smch_isla216p_read_8 (self, addr, data) == sizeof(uint8_t))?
        SMCH_SUCCESS : SMCH_ERR_RW_SMPR;
}

smch_err_e smch_isla216p_set_test_mode (smch_isla216p_t *self, uint8_t mode)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    uint8_t data = 0;
    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_TESTIO, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not read from TESTIO register",
            err_smpr_read, SMCH_ERR_RW_SMPR);

    data = (data & ~ISLA216P_TESTIO_OUTMODE_MASK) | ISLA216P_TESTIO_OUTMODE_W(mode);
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_TESTIO, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to TESTIO register",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_write:
err_smpr_read:
    return err;
}

smch_err_e smch_isla216p_get_chipid (smch_isla216p_t *self, uint8_t *chipid)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_CHIPID, chipid);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not read from CHIPID register",
            err_smpr_read, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_read:
    return err;
}

smch_err_e smch_isla216p_get_chipver (smch_isla216p_t *self, uint8_t *chipver)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_CHIPVER, chipver);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not read from CHIPVER register",
            err_smpr_read, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_read:
    return err;
}

smch_err_e smch_isla216p_get_cal_status (smch_isla216p_t *self, uint8_t *cal_status)
{
    uint8_t __cal_status = 0;
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_CALSTATUS, &__cal_status);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not read from CAL_STATUS register",
            err_smpr_read, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

    *cal_status = __cal_status & ISLA216P_CALCSTATUS_CALCDONE;

err_smpr_read:
    return err;
}

smch_err_e smch_isla216p_set_rst (smch_isla216p_t *self, uint8_t rst_operation)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    ASSERT_TEST(rst_operation <= ISLA216P_NAPSLP_SLEEP_MODE,
            "Reset operation is out of range", err_smpr_write,
            SMCH_ERR_INV_FUNC_PARAM);

    uint8_t data = ISLA216P_NAPSLP_W(rst_operation);
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_NAPSLP, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to ISLA216P_REG_NAPSLP",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_write:
    return err;
}

smch_err_e smch_isla216p_get_rst (smch_isla216p_t *self, uint8_t *rst_operation)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    uint8_t data = 0;
    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_NAPSLP, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not read from ISLA216P_REG_NAPSLP register",
            err_smpr_read, SMCH_ERR_RW_SMPR);

    *rst_operation = ISLA216P_NAPSLP_R(data);
    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_read:
    return err;
}

smch_err_e smch_isla216p_set_portconfig (smch_isla216p_t *self, uint8_t config)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;

    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_PORTCONFIG, &config);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to ISLA216P_REG_PORTCONFIG",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    SMCH_ISLA216P_WAIT_DFLT;

err_smpr_write:
    return err;
}

smch_err_e smch_isla216p_get_temp (smch_isla216p_t *self, uint16_t *temp)
{
    smch_err_e err = SMCH_SUCCESS;
    ssize_t rw_err = -1;
    uint8_t data = 0;

    data = ISLA216P_TEMP_CTL_RESET;
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_TEMP_CTL, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not reset temperature counter",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    /* As per ISLA216P25 datasheet, page 28 */
    data = (ISLA216P_TEMP_CTL_PTAT_MODE_EN |
                      ISLA216P_TEMP_CTL_ENABLE |
                      ISLA216P_TEMP_CTL_DIVIDER_W(ISLA216P_TEMP_CTL_DIVIDER_REC_VALUE));
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_TEMP_CTL, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to ISLA216P_REG_TEMP_CTL",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    /* Wait longer than 132us */
    SMCH_ISLA216P_WAIT(200);

    /* Power-down temperature counter */
    data = ISLA216P_TEMP_CTL_PD;
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_TEMP_CTL, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not power-down ISLA216P temperature counter",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    uint8_t temp_code_high = 0;
    uint8_t temp_code_low = 0;
    rw_err = _smch_isla216p_read_8 (self, ISLA216P_REG_TEMP_COUNTER_HIGH, &temp_code_high);
    rw_err += _smch_isla216p_read_8 (self, ISLA216P_REG_TEMP_COUNTER_LOW, &temp_code_low);
    ASSERT_TEST(rw_err == sizeof(uint16_t), "Could not read temperature code",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    /* Check if reading is valid */
    if (temp_code_high & ISLA216P_TEMP_CTL_VALID_READ) {
        *temp = ISLA216P_TEMP_COUNTER_R(temp_code_high, temp_code_low);
    }
    else {
        err = SMCH_ERR_RW_SMPR;
        goto err_smpr_write;
    }
    
    /* Power-down temperature counter again, as per ISLA216P datahsheet */
    data = ISLA216P_TEMP_CTL_PD;
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_TEMP_CTL, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not power-down ISLA216P temperature counter",
            err_smpr_write, SMCH_ERR_RW_SMPR);

err_smpr_write:
    return err;
}

/***************** Static functions *****************/

static smch_err_e _smch_isla216p_init (smch_isla216p_t *self)
{
    smch_err_e err = SMCH_SUCCESS;

    /* Turn on SDO (4-wire mode) */
    err = smch_isla216p_set_portconfig (self, ISLA216P_PORTCONFIG_SDO_ACTIVE);
    ASSERT_TEST(err == SMCH_SUCCESS, "Could not set ISLA216P to 4-wire mode",
            err_smpr_write);

#if 0
    /* Reset registers */
    uint8_t data = ISLA216P_PORTCONFIG_SOFT_RESET;
    DBE_DEBUG (DBG_SM_CH | DBG_LVL_INFO,
            "[sm_ch:isla216p] Writing 0x%02X to addr 0x%02X\n", data, ISLA216P_REG_PORTCONFIG);
    ssize_t rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_PORTCONFIG, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to ISLA216P_REG_PORTCONFIG",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    /* Clear reset */
    data &= ~ISLA216P_PORTCONFIG_SOFT_RESET;
    rw_err = _smch_isla216p_write_8 (self, ISLA216P_REG_PORTCONFIG, &data);
    ASSERT_TEST(rw_err == sizeof(uint8_t), "Could not write to ISLA216P_REG_PORTCONFIG",
            err_smpr_write, SMCH_ERR_RW_SMPR);

    /* Turn on SDO (4-wire mode) again as reset disabled it */
    err = smch_isla216p_set_portconfig (self, ISLA216P_PORTCONFIG_SDO_ACTIVE);
    ASSERT_TEST(err == SMCH_SUCCESS, "Could not set ISLA216P to 4-wire mode",
            err_smpr_write);
#endif

    err = smch_isla216p_set_rst (self, ISLA216P_NAPSLP_NORMAL_OPERATION);
    ASSERT_TEST(err == SMCH_SUCCESS, "Could not set ISLA216P to normal operation",
            err_smpr_write);

err_smpr_write:
    return err;
}

static ssize_t _smch_isla216p_write_8 (smch_isla216p_t *self, uint8_t addr,
        const uint8_t *data)
{
    ssize_t err = sizeof(uint8_t);

    /* We do only 24-bit cycles composed of:
     *
     * 16-bit instruction bits + 8-bit data bits
     *
     * Source: ISLA216P datasheet, pages 22,23,24
     * */

    /* We transmit a WRITE operation, with 1 byte transfer, with address as "addr"
     * and data as "data" */
    uint32_t __addr = ~ISLA216P_HDR_RW & (
                ISLA216P_HDR_BT_W(0x0) |
                ISLA216P_HDR_ADDR_W(addr)
            );
    size_t __addr_size = ISLA216P_INSTADDR_SIZE/SMPR_BYTE_2_BIT;
    uint32_t __data = ISLA216P_DATA_W(*data);
    size_t __data_size = ISLA216P_DATA_SIZE/SMPR_BYTE_2_BIT;

    ssize_t smpr_err = smpr_write_block (self->proto, __addr_size,
            __addr, __data_size, &__data);
    ASSERT_TEST(smpr_err > 0 && (size_t) smpr_err == __data_size,
            "Could not write to SMPR", err_smpr_write, -1);

err_smpr_write:
    return err;
}

static ssize_t _smch_isla216p_read_8 (smch_isla216p_t *self, uint8_t addr,
        uint8_t *data)
{
    ssize_t err = sizeof(uint8_t);

    /* We do Instruction + Data transactions composed of:
     *
     * 16-bit instruction bits + 8-bit data bits (up to 4 beats)
     *
     * Source: ISLA216P datasheet, pages 22,23,24
     * */

    /* We transmit a READ operation, with 1 byte transfer, with address as "addr"  */
    uint32_t __addr = ISLA216P_HDR_RW | (
                ISLA216P_HDR_BT_W(0x0) |
                ISLA216P_HDR_ADDR_W(addr)
            );
    size_t __addr_size = ISLA216P_INSTADDR_SIZE/SMPR_BYTE_2_BIT;
    size_t __data_size = ISLA216P_DATA_SIZE/SMPR_BYTE_2_BIT;

    ssize_t smpr_err = smpr_read_block (self->proto, __addr_size,
            __addr, __data_size, (uint32_t *) data);
    ASSERT_TEST(smpr_err > 0 && (size_t) smpr_err == __data_size,
            "Could not read to SMPR", err_smpr_read, -1);

err_smpr_read:
    return err;
}
