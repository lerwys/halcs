/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 *
 * Description: Software driver for PCA9547 switch chip
 */

#include "halcs_server.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, SM_CH, "[sm_ch:pca9547]",         \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)       \
    ASSERT_HAL_ALLOC(ptr, SM_CH, "[sm_ch:pca9547]",                 \
            smch_err_str(SMCH_ERR_ALLOC),                           \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                    \
    CHECK_HAL_ERR(err, SM_CH, "[sm_ch:pca9547]",                    \
            smch_err_str (err_type))

#define SMCH_PCA9547_NAME                    "PCA9547"

struct _smch_pca9547_t {
    smpr_t *proto;                    /* PROTO protocol object */
};

static smch_err_e _smch_pca9547_write_8 (smch_pca9547_t *self, const uint8_t *data);
static smch_err_e _smch_pca9547_read_8 (smch_pca9547_t *self, uint8_t *data);

/* Creates a new instance of the SMCH PCA9547 */
smch_pca9547_t * smch_pca9547_new (smio_t *parent, uint64_t base,
        const smpr_proto_ops_t *reg_ops, int verbose)
{
    UNUSED(verbose);
    assert (parent);

    smch_pca9547_t *self = (smch_pca9547_t *) zmalloc (sizeof *self);
    ASSERT_ALLOC(self, err_self_alloc);

    self->proto = smpr_new (SMCH_PCA9547_NAME, parent, reg_ops, verbose);
    ASSERT_ALLOC(self->proto, err_proto_alloc);

    /* Initalize the PROTO protocol */
    int smpr_err = smpr_open (self->proto, base, NULL /* Default parameters are fine */);
    ASSERT_TEST(smpr_err == 0, "Could not initialize SMPR protocol", err_smpr_init);

    DBE_DEBUG (DBG_SM_CH | DBG_LVL_INFO, "[sm_ch:pca9547] Created instance of SMCH\n");
    return self;

err_smpr_init:
    smpr_destroy (&self->proto);
err_proto_alloc:
    free (self);
err_self_alloc:
    return NULL;
}

/* Destroy an instance of the SMCH PCA9547 */
smch_err_e smch_pca9547_destroy (smch_pca9547_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        smch_pca9547_t *self = *self_p;

        smpr_release (self->proto);
        smpr_destroy (&self->proto);
        free (self);
        *self_p = NULL;
    }

    return SMCH_SUCCESS;
}

smch_err_e smch_pca9547_write_8 (smch_pca9547_t *self, const uint8_t *data)
{
    return _smch_pca9547_write_8 (self, data);
}

smch_err_e smch_pca9547_read_8 (smch_pca9547_t *self, uint8_t *data)
{
    return _smch_pca9547_read_8 (self, data);
}

/******************************* Static Functions *****************************/

static smch_err_e _smch_pca9547_write_8 (smch_pca9547_t *self, const uint8_t *data)
{
    smch_err_e err = SMCH_SUCCESS;

    /* PCA9547 write byte transaction is:
     *
     *   Control Byte
     *      8-bit
     * */
    DBE_DEBUG (DBG_SM_CH | DBG_LVL_TRACE, "[sm_ch:pca9547_write_8] data =  0x%02X\n",
            *data);

    ssize_t smpr_err = smpr_write_block (self->proto, 0, 0, sizeof (*data), (uint32_t *) data);
    ASSERT_TEST(smpr_err == sizeof (*data) /* in bytes*/,
            "Could not write data to I2C", err_exit, SMCH_ERR_RW_SMPR);

err_exit:
    return err;
}

static smch_err_e _smch_pca9547_read_8 (smch_pca9547_t *self, uint8_t *data)
{
    smch_err_e err = SMCH_SUCCESS;

    /* PCA9547 read byte transaction is:
     *
     *   Control Byte
     *      8-bit
     * */

    ssize_t smpr_err = smpr_read_block (self->proto, 0, 0, sizeof (*data), (uint32_t *) data);
    ASSERT_TEST(smpr_err == sizeof (*data) /* in bytes*/,
            "Could not read data to I2C", err_exit, SMCH_ERR_RW_SMPR);

    DBE_DEBUG (DBG_SM_CH | DBG_LVL_TRACE, "[sm_ch:pca9547_read_8] data =  0x%02X\n",
            *data);

err_exit:
    return err;
}

smch_err_e smch_pca9547_en_chan (smch_pca9547_t *self, uint8_t chan)
{
    assert (self);
    smch_err_e err = SMCH_SUCCESS;

    ASSERT_TEST(chan < PCA9547_MAX_NUM_CHANNEL || chan == SMCH_PCA9547_NO_CHANNEL,
            "Invalid channel selected", err_exit, SMCH_ERR_INV_FUNC_PARAM);

    uint8_t data = 0;

    if (chan == SMCH_PCA9547_NO_CHANNEL) {
        data = PCA9547_CHANNEL_SEL_EN_OFF;
    }
    else {
        data = PCA9547_CHANNEL_SEL_EN | chan;
    }

    err =  _smch_pca9547_write_8 (self, &data);
    ASSERT_TEST(err == SMCH_SUCCESS, "Could not enable channel", err_exit);

err_exit:
    return err;
}
