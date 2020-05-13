/*
 * Copyright (C) 2017 LNLS (www.lnls.br)
 * Authors: Lucas Russo <lucas.russo@lnls.br>
 *          Janito Vaqueiro Ferreira Filho <janito.filho@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "acq_client.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, LIB_CLIENT, "[libacqclient]",\
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_ALLOC(ptr, LIB_CLIENT, "[libacqclient]",        \
            halcs_client_err_str(HALCS_CLIENT_ERR_ALLOC),       \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                            \
    CHECK_HAL_ERR(err, LIB_CLIENT, "[libacqclient]",           \
            halcs_client_err_str (err_type))

/* Our structure */
struct _acq_client_t {
    halcs_client_t *halcs_client;               /* HALCS client interface */
    smio_acq_data_block_t *acq_block;           /* Acquisition block data */
};

/********************************************************/
/************************ Our API ***********************/
/********************************************************/

static acq_client_t *_acq_client_new (halcs_client_t *halcs_client);

acq_client_t *acq_client_new (char *broker_endp, int verbose,
        const char *log_file_name)
{
    halcs_client_t *halcs_client = halcs_client_new (broker_endp, verbose,
            log_file_name);

    ASSERT_ALLOC (halcs_client, err_halcs_client_new);

    return _acq_client_new (halcs_client);

err_halcs_client_new:
    return NULL;
}

acq_client_t *acq_client_new_time (char *broker_endp, int verbose,
        const char *log_file_name, int timeout)
{
    halcs_client_t *halcs_client = halcs_client_new_time (broker_endp, verbose,
            log_file_name, timeout);

    ASSERT_ALLOC (halcs_client, err_halcs_client_new);

    return _acq_client_new (halcs_client);

err_halcs_client_new:
    return NULL;
}

acq_client_t *acq_client_new_log_mode (char *broker_endp, int verbose,
        const char *log_file_name, const char *log_mode)
{
    halcs_client_t *halcs_client = halcs_client_new_log_mode (broker_endp,
            verbose, log_file_name, log_mode);

    ASSERT_ALLOC (halcs_client, err_halcs_client_new);

    return _acq_client_new (halcs_client);

err_halcs_client_new:
    return NULL;
}

acq_client_t *acq_client_new_log_mode_time (char *broker_endp, int verbose,
        const char *log_file_name, const char *log_mode, int timeout)
{
    halcs_client_t *halcs_client = halcs_client_new_log_mode_time (broker_endp,
            verbose, log_file_name, log_mode, timeout);

    ASSERT_ALLOC (halcs_client, err_halcs_client_new);

    return _acq_client_new (halcs_client);

err_halcs_client_new:
    return NULL;
}

static acq_client_t *_acq_client_new (halcs_client_t *halcs_client)
{
    acq_client_t *self = zmalloc (sizeof *self);
    ASSERT_ALLOC(self, err_self_alloc);
    self->acq_block = zmalloc(sizeof *self->acq_block + BLOCK_SIZE);
    ASSERT_ALLOC(self->acq_block, err_alloc_acq_block);

    self->halcs_client = halcs_client;

    return self;

err_alloc_acq_block:
    free (self);
err_self_alloc:
    return NULL;
}

void acq_client_destroy (acq_client_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        acq_client_t* self = *self_p;

        halcs_client_destroy (&self->halcs_client);

        free (self->acq_block);
        free (self);
        *self_p = NULL;
    }
}

/****************** ACQ SMIO Functions ****************/

static halcs_client_err_e _acq_start (acq_client_t *self, char *service,
        acq_req_t *acq_req);
static halcs_client_err_e _acq_check (acq_client_t *self, char *service);
static halcs_client_err_e _acq_check_timed (acq_client_t *self, char *service,
        int timeout);
static halcs_client_err_e _acq_get_data_block (acq_client_t *self,
        char *service, acq_trans_t *acq_trans);
static halcs_client_err_e _acq_get_curve (acq_client_t *self, char *service,
        acq_trans_t *acq_trans);
static halcs_client_err_e _acq_full (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout);
static halcs_client_err_e _acq_full_compat (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout, bool new_acq);

/* Long string triggers */
static const char *acq_client_trig_lstring [ACQ_CLIENT_TRIG_END] =
{
    [ACQ_CLIENT_TRIG_SKIP]           = "No trigger configured",
    [ACQ_CLIENT_TRIG_EXTERNAL]       = "External Trigger",
    [ACQ_CLIENT_TRIG_DATA_DRIVEN]    = "Data-driven Trigger",
    [ACQ_CLIENT_TRIG_SOFTWARE]       = "Software Trigger",
};

/* Short string triggers */
static const char *acq_client_trig_sstring [ACQ_CLIENT_TRIG_END] =
{
    [ACQ_CLIENT_TRIG_SKIP]           = "Skip",
    [ACQ_CLIENT_TRIG_EXTERNAL]       = "External Trig",
    [ACQ_CLIENT_TRIG_DATA_DRIVEN]    = "Data Trig",
    [ACQ_CLIENT_TRIG_SOFTWARE]       = "Software Trig",
};

/* Convert enumeration type to long string */
const char * acq_client_trig_lstr (acq_client_trig_e err)
{
    return acq_client_trig_lstring [err];
}

/* Convert enumeration type to short string */
const char * acq_client_trig_sstr (acq_client_trig_e err)
{
    return acq_client_trig_sstring [err];
}

halcs_client_err_e acq_start (acq_client_t *self, char *service, acq_req_t *acq_req)
{
    return _acq_start (self, service, acq_req);
}

halcs_client_err_e acq_check (acq_client_t *self, char *service)
{
    return _acq_check (self, service);
}

halcs_client_err_e acq_check_timed (acq_client_t *self, char *service,
        int timeout)
{
    return _acq_check_timed (self, service, timeout);
}

halcs_client_err_e acq_get_data_block (acq_client_t *self, char *service,
        acq_trans_t *acq_trans)
{
    return _acq_get_data_block (self, service, acq_trans);
}

halcs_client_err_e acq_get_curve (acq_client_t *self, char *service,
        acq_trans_t *acq_trans)
{
    return _acq_get_curve (self, service, acq_trans);
}

halcs_client_err_e acq_full (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout)
{
    return _acq_full (self, service, acq_trans, timeout);
}

halcs_client_err_e acq_full_compat (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout, bool new_acq)
{
    return _acq_full_compat (self, service, acq_trans, timeout, new_acq);
}

static halcs_client_err_e _acq_check_timed (acq_client_t *self, char *service,
        int timeout)
{
    return func_polling (self->halcs_client, ACQ_NAME_CHECK_DATA_ACQUIRE,
            service, NULL, NULL, timeout);
}

halcs_client_err_e acq_set_trig (acq_client_t *self, char *service,
        uint32_t trig)
{
    return param_client_write (self->halcs_client, service, ACQ_OPCODE_CFG_TRIG,
            trig);
}

halcs_client_err_e acq_get_trig (acq_client_t *self, char *service,
        uint32_t *trig)
{
    return param_client_read (self->halcs_client, service, ACQ_OPCODE_CFG_TRIG,
            trig);
}

halcs_client_err_e acq_set_data_trig_pol (acq_client_t *self, char *service,
        uint32_t data_trig_pol)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_POL, data_trig_pol);
}

halcs_client_err_e acq_get_data_trig_pol (acq_client_t *self, char *service,
        uint32_t *data_trig_pol)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_POL, data_trig_pol);
}

halcs_client_err_e acq_set_data_trig_sel (acq_client_t *self, char *service,
        uint32_t data_trig_sel)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_SEL, data_trig_sel);
}

halcs_client_err_e acq_get_data_trig_sel (acq_client_t *self, char *service,
        uint32_t *data_trig_sel)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_SEL, data_trig_sel);
}

halcs_client_err_e acq_set_data_trig_filt (acq_client_t *self, char *service,
        uint32_t data_trig_filt)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_FILT, data_trig_filt);
}

halcs_client_err_e acq_get_data_trig_filt (acq_client_t *self, char *service,
        uint32_t *data_trig_filt)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_FILT, data_trig_filt);
}

halcs_client_err_e acq_set_data_trig_thres (acq_client_t *self, char *service,
        uint32_t data_trig_thres)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_THRES, data_trig_thres);
}

halcs_client_err_e acq_get_data_trig_thres (acq_client_t *self, char *service,
        uint32_t *data_trig_thres)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_THRES, data_trig_thres);
}

halcs_client_err_e acq_set_hw_trig_dly (acq_client_t *self, char *service,
        uint32_t hw_trig_dly)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_TRIG_DLY, hw_trig_dly);
}

halcs_client_err_e acq_get_hw_trig_dly (acq_client_t *self, char *service,
        uint32_t *hw_trig_dly)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_TRIG_DLY, hw_trig_dly);
}

halcs_client_err_e acq_set_sw_trig (acq_client_t *self, char *service,
        uint32_t sw_trig)
{
    return param_client_write (self->halcs_client, service, ACQ_OPCODE_SW_TRIG,
            sw_trig);
}

halcs_client_err_e acq_get_sw_trig (acq_client_t *self, char *service,
        uint32_t *sw_trig)
{
    return param_client_read (self->halcs_client, service, ACQ_OPCODE_SW_TRIG,
            sw_trig);
}

halcs_client_err_e acq_set_fsm_stop (acq_client_t *self, char *service,
        uint32_t fsm_stop)
{
    return param_client_write (self->halcs_client, service, ACQ_OPCODE_FSM_STOP,
            fsm_stop);
}

halcs_client_err_e acq_get_fsm_stop (acq_client_t *self, char *service,
        uint32_t *fsm_stop)
{
    return param_client_read (self->halcs_client, service, ACQ_OPCODE_FSM_STOP,
            fsm_stop);
}

halcs_client_err_e acq_set_data_trig_chan (acq_client_t *self, char *service,
        uint32_t data_trig_chan)
{
    return param_client_write (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_CHAN, data_trig_chan);
}

halcs_client_err_e acq_get_data_trig_chan (acq_client_t *self, char *service,
        uint32_t *data_trig_chan)
{
    return param_client_read (self->halcs_client, service,
            ACQ_OPCODE_HW_DATA_TRIG_CHAN, data_trig_chan);
}

/****************** Acquisition Channel properties Functions ****************/

halcs_client_err_e halcs_set_acq_ch_int_width (acq_client_t *self, char *service,
        uint32_t chan, uint32_t ch_int_width)
{
    return param_client_write2 (self->halcs_client, service, ACQ_OPCODE_CH_INT_WIDTH,
            chan, ch_int_width);
}

halcs_client_err_e halcs_get_acq_ch_int_width (acq_client_t *self, char *service,
        uint32_t chan, uint32_t *ch_int_width)
{
    return param_client_write_read (self->halcs_client, service, ACQ_OPCODE_CH_INT_WIDTH,
            chan, ch_int_width);
}

halcs_client_err_e halcs_set_acq_ch_num_coalesce (acq_client_t *self, char *service,
        uint32_t chan, uint32_t ch_num_coalesce)
{
    return param_client_write2 (self->halcs_client, service, ACQ_OPCODE_CH_NUM_COALESCE,
            chan, ch_num_coalesce);
}

halcs_client_err_e halcs_get_acq_ch_num_coalesce (acq_client_t *self, char *service,
        uint32_t chan, uint32_t *ch_num_coalesce)
{
    return param_client_write_read (self->halcs_client, service, ACQ_OPCODE_CH_NUM_COALESCE,
            chan, ch_num_coalesce);
}

halcs_client_err_e halcs_set_acq_ch_num_atoms (acq_client_t *self, char *service,
        uint32_t chan, uint32_t ch_num_atoms)
{
    return param_client_write2 (self->halcs_client, service, ACQ_OPCODE_CH_NUM_ATOMS,
            chan, ch_num_atoms);
}

halcs_client_err_e halcs_get_acq_ch_num_atoms (acq_client_t *self, char *service,
        uint32_t chan, uint32_t *ch_num_atoms)
{
    return param_client_write_read (self->halcs_client, service, ACQ_OPCODE_CH_NUM_ATOMS,
            chan, ch_num_atoms);
}

halcs_client_err_e halcs_set_acq_ch_atom_width (acq_client_t *self, char *service,
        uint32_t chan, uint32_t ch_atom_width)
{
    return param_client_write2 (self->halcs_client, service, ACQ_OPCODE_CH_ATOM_WIDTH,
            chan, ch_atom_width);
}

halcs_client_err_e halcs_get_acq_ch_atom_width (acq_client_t *self, char *service,
        uint32_t chan, uint32_t *ch_atom_width)
{
    return param_client_write_read (self->halcs_client, service, ACQ_OPCODE_CH_ATOM_WIDTH,
            chan, ch_atom_width);
}

halcs_client_err_e halcs_set_acq_ch_sample_size (acq_client_t *self, char *service,
        uint32_t chan, uint32_t ch_sample_size)
{
    return param_client_write2 (self->halcs_client, service, ACQ_OPCODE_CH_SAMPLE_SIZE,
            chan, ch_sample_size);
}

halcs_client_err_e halcs_get_acq_ch_sample_size (acq_client_t *self, char *service,
        uint32_t chan, uint32_t *ch_sample_size)
{
    return param_client_write_read (self->halcs_client, service, ACQ_OPCODE_CH_SAMPLE_SIZE,
            chan, ch_sample_size);
}

static halcs_client_err_e _acq_start (acq_client_t *self, char *service, acq_req_t *acq_req)
{
    assert (self);
    assert (service);
    assert (acq_req);

    uint32_t write_val[4] = {0};
    write_val[0] = acq_req->num_samples_pre;
    write_val[1] = acq_req->num_samples_post;
    write_val[2] = acq_req->num_shots;
    write_val[3] = acq_req->chan;

    const disp_op_t* func = halcs_func_translate(ACQ_NAME_DATA_ACQUIRE);
    halcs_client_err_e err = halcs_func_exec(self->halcs_client, func, service,
            write_val, NULL);

    /* Check if any error occurred */
    ASSERT_TEST(err == HALCS_CLIENT_SUCCESS, "acq_data_acquire: Data acquire was "
            "not requested correctly", err_data_acquire, HALCS_CLIENT_ERR_AGAIN);

    /* If we are here, then the request was successfully acquired*/
    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_data_acquire: "
            "Data acquire was successfully required\n");

err_data_acquire:
    return err;
}

static halcs_client_err_e _acq_check (acq_client_t *self, char *service)
{
    assert (self);
    assert (service);

    const disp_op_t* func = halcs_func_translate(ACQ_NAME_CHECK_DATA_ACQUIRE);
    halcs_client_err_e err = halcs_func_exec(self->halcs_client, func, service,
            NULL, NULL);

    if (err != HALCS_CLIENT_SUCCESS) {
        DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] data acquisition "
                "was not completed\n");
    }
    else {
        DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] data acquisition "
                "was completed\n");
    }

    return err;
}

static halcs_client_err_e _acq_get_data_block (acq_client_t *self, char *service, acq_trans_t *acq_trans)
{
    assert (self);
    assert (service);
    assert (acq_trans);
    assert (acq_trans->block.data);

    halcs_client_err_e err = HALCS_CLIENT_SUCCESS;

    uint32_t write_val[2] = {0};
    write_val[0] = acq_trans->req.chan;
    write_val[1] = acq_trans->block.idx;

    smio_acq_data_block_t *acq_block = self->acq_block;

    /* Sent Message is:
     * frame 0: operation code
     * frame 1: channel
     * frame 2: block required */

    const disp_op_t* func = halcs_func_translate(ACQ_NAME_GET_DATA_BLOCK);
    err = halcs_func_exec(self->halcs_client, func, service, write_val,
            (uint32_t *) acq_block);

    /* Message is:
     * frame 0: error code
     * frame 1: number of bytes read (optional)
     * frame 2: data read (optional) */

    /* Check if any error occurred */
    ASSERT_TEST(err == HALCS_CLIENT_SUCCESS,
            "acq_get_data_block: Data block was not acquired",
            err_get_data_block, HALCS_CLIENT_ERR_SERVER);

    /* Data size effectively returned */
    uint32_t read_size = (acq_trans->block.data_size < acq_block->valid_bytes) ?
        acq_trans->block.data_size : acq_block->valid_bytes;

    /* Copy message contents to user */
    memcpy (acq_trans->block.data, acq_block->data, read_size);

    /* Inform user about the number of bytes effectively copied */
    acq_trans->block.bytes_read = read_size;

    /* Print some debug messages */
    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_data_block: "
            "read_size: %u\n", read_size);
    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_data_block: "
            "acq_trans->block.data: %p\n", acq_trans->block.data);

err_get_data_block:
    return err;
}

static halcs_client_err_e _acq_get_curve (acq_client_t *self, char *service, acq_trans_t *acq_trans)
{
    assert (self);
    assert (service);
    assert (acq_trans);
    assert (acq_trans->block.data);

    halcs_client_err_e err = HALCS_CLIENT_SUCCESS;
    uint32_t req_chan = acq_trans->req.chan;
    uint32_t req_ch_sample_size = 0;

    err = halcs_get_acq_ch_sample_size (self, service, req_chan, &req_ch_sample_size);
    ASSERT_TEST(err == HALCS_CLIENT_SUCCESS,
            "Getting acquisition channel properties failed",
            err_acq_get_ch_prop);

    uint32_t num_samples_shot = acq_trans->req.num_samples_pre +
        acq_trans->req.num_samples_post;
    uint32_t num_samples_multishot = num_samples_shot*acq_trans->req.num_shots;
    uint32_t n_max_samples = BLOCK_SIZE/req_ch_sample_size;
    uint32_t block_n_valid = num_samples_multishot / n_max_samples;
    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_curve: "
            "block_n_valid = %u\n", block_n_valid);

    /* Total bytes read */
    uint32_t total_bread = 0;
    /* Save the original buffer size for later */
    uint32_t data_size = acq_trans->block.data_size;
    uint32_t *original_data_pt = acq_trans->block.data;

    /* Fill all blocks */
    for (uint32_t block_n = 0; block_n <= block_n_valid; block_n++) {
        if (zsys_interrupted) {
            err = HALCS_CLIENT_INT;
            goto halcs_zsys_interrupted;
        }

        acq_trans->block.idx = block_n;
        err = _acq_get_data_block (self, service, acq_trans);

        /* Check for return code */
        ASSERT_TEST(err == HALCS_CLIENT_SUCCESS,
                "_acq_get_data_block failed. block_n is probably out of range",
                err_acq_get_data_block);

        total_bread += acq_trans->block.bytes_read;
        acq_trans->block.data = (uint32_t *)((uint8_t *)acq_trans->block.data + acq_trans->block.bytes_read);
        acq_trans->block.data_size -= acq_trans->block.bytes_read;

        /* Print some debug messages */
        DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_curve: "
                "Total bytes read up to now: %u\n", total_bread);
        DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_curve: "
                "Data pointer addr: %p\n", acq_trans->block.data);
        DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_curve: "
                "Data buffer size: %u\n", acq_trans->block.data_size);
    }

    /* Return to client the total number of bytes read */
    acq_trans->block.bytes_read = total_bread;
    acq_trans->block.data_size = data_size;
    acq_trans->block.data = original_data_pt;

    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] acq_get_curve: "
            "Data curve of %u bytes was successfully acquired\n", total_bread);

halcs_zsys_interrupted:
err_acq_get_data_block:
err_acq_get_ch_prop:
    return err;
}

static halcs_client_err_e _acq_full (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout)
{
    assert (self);
    assert (service);
    assert (acq_trans);
    assert (acq_trans->block.data);

    /* Send Acquisition Request */
    _acq_start (self, service, &acq_trans->req);

    /* Wait until the acquisition is finished */
    halcs_client_err_e err = func_polling (self->halcs_client,
            ACQ_NAME_CHECK_DATA_ACQUIRE, service, NULL, NULL, timeout);

    ASSERT_TEST(err == HALCS_CLIENT_SUCCESS,
            "Data acquisition was not completed",
            err_check_data_acquire, HALCS_CLIENT_ERR_SERVER);

    /* If we are here, then the acquisition was successfully completed*/
    DBE_DEBUG (DBG_LIB_CLIENT | DBG_LVL_TRACE, "[libacqclient] "
            "Data acquisition was successfully completed\n");

    err = _acq_get_curve (self, service, acq_trans);
    ASSERT_TEST(err == HALCS_CLIENT_SUCCESS, "Could not get requested curve",
            err_get_curve, HALCS_CLIENT_ERR_SERVER);

err_get_curve:
err_check_data_acquire:
    return err;
}

/* Wrapper to be compatible with old function behavior */
static halcs_client_err_e _acq_full_compat (acq_client_t *self, char *service,
        acq_trans_t *acq_trans, int timeout, bool new_acq)
{
    if (new_acq) {
        return _acq_full (self, service, acq_trans, timeout);
    }
    else {
        return _acq_get_curve (self, service, acq_trans);
    }
}
