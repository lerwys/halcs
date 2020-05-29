/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "halcs_server.h"
/* Private headers */
#include "sm_io_fmc250m_4ch_codes.h"
#include "sm_io_fmc250m_4ch_defaults.h"
#include "sm_io_fmc250m_4ch_exports.h"
#include "sm_io_fmc250m_4ch_core.h"
#include "sm_io_fmc250m_4ch_exp.h"
#include "hw/wb_fmc250m_4ch_regs.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...)  \
    ASSERT_HAL_TEST(test_boolean, SM_IO, "[sm_io:fmc250m_4ch_exp]", \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_ALLOC(ptr, SM_IO, "[sm_io:fmc250m_4ch_exp]", \
            smio_err_str(SMIO_ERR_ALLOC),                   \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                            \
    CHECK_HAL_ERR(err, SM_IO, "[sm_io:fmc250m_4ch_exp]",    \
            smio_err_str (err_type))

#define FMC250M_4CH_CHECK_ACTIVE(self)                          \
    ({                                                          \
        smio_fmc250m_4ch_t *fmc250m = smio_get_handler (self);  \
        if (fmc250m == NULL) {                                  \
            DBE_DEBUG (DBG_SM_IO | DBG_LVL_ERR, "[sm_io:fmc250m_4ch_exp] "\
                "Could not get SMIO FMC250M handler\n");        \
            return -FMC250M_4CH_ERR;                            \
        }                                                       \
        if (fmc250m->type != TYPE_FMC250M_4CH_ACTIVE) {         \
            DBE_DEBUG (DBG_SM_IO | DBG_LVL_ERR, "[sm_io:fmc250m_4ch_exp] "\
                "Board is not of ACTIVE type. Unimplemented "   \
                "function for this type of FMC250M_4CH board\n"); \
            return -FMC250M_4CH_UNINPL;                         \
        }                                                       \
    })

static smch_err_e smch_isla216p_test_mode_compat (smch_isla216p_t *self,
        uint32_t *mode);

/************************************************************/
/************ Specific FMC_250M_4CH Operations **************/
/************************************************************/

/********************* ADC RAW Data registers (for simple debug) **************/

/* FIXME: Override DATA<channel_number>_VAL macros with the DATA_GLOBAL ones.
 * As the macros are defined with 32-bits and there is a shift involved, the
 * compiler complains about the size of the macro being bigger the the type
 * (uint32_t). So, we define our own macros here and don't use the generated
 * ones*/
#define WB_FMC_250M_4CH_CSR_DATA_GLOBAL_MASK     ((1ULL<<32)-1)
#define WB_FMC_250M_4CH_CSR_DATA_GLOBAL_W(val)   (val)
#define WB_FMC_250M_4CH_CSR_DATA_GLOBAL_R(val)   (val)

/* We receive data as 32-bit, but the 16 MSB are zeroed. It is conveninent
 * to sign extend the bits */
static int _rw_halcs_fmc250m_4ch_data_fmt (uint32_t *data)
{
    *data = WBGEN2_SIGN_EXTEND(*data, 15);

    return PARAM_OK;
}

rw_param_format_fp rw_halcs_fmc250m_4ch_data_fmt_fp = _rw_halcs_fmc250m_4ch_data_fmt;

RW_PARAM_FUNC(fmc250m_4ch, adc_data0) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH0_STA, VAL, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            rw_halcs_fmc250m_4ch_data_fmt_fp, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data1) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH1_STA, VAL, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            rw_halcs_fmc250m_4ch_data_fmt_fp, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data2) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH2_STA, VAL, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            rw_halcs_fmc250m_4ch_data_fmt_fp, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data3) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH3_STA, VAL, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            rw_halcs_fmc250m_4ch_data_fmt_fp, SET_FIELD);
}

/******************************ADC Data Delay Values *************************/

RW_PARAM_FUNC(fmc250m_4ch, adc_data_dly0) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH0_FN_DLY, DATA_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data_dly1) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH1_FN_DLY, DATA_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data_dly2) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH2_FN_DLY, DATA_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_data_dly3) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH3_FN_DLY, DATA_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

/******************************ADC Clock Delay Values *************************/

RW_PARAM_FUNC(fmc250m_4ch, adc_clk_dly0) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH0_FN_DLY, CLK_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_clk_dly1) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH1_FN_DLY, CLK_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_clk_dly2) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH2_FN_DLY, CLK_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_clk_dly3) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH3_FN_DLY, CLK_CHAIN_DLY, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

/******************************** ADC Delay Lines *****************************/

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_line0) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH0_FN_SEL, LINE, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_line1) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH1_FN_SEL, LINE, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_line2) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH2_FN_SEL, LINE, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_line3) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            CH3_FN_SEL, LINE, MULT_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

/******************************** ADC Delay Update ****************************/

/* On FMC250, all update delays set the same register., but we keep FMC130M
 * compatibility and register one function for each */

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_updt0) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, UPDATE_CLK_DLY, SINGLE_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_updt1) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, UPDATE_CLK_DLY, SINGLE_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_updt2) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, UPDATE_CLK_DLY, SINGLE_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

RW_PARAM_FUNC(fmc250m_4ch, adc_dly_updt3) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, UPDATE_CLK_DLY, SINGLE_BIT_PARAM,
            /* no minimum value */, /* no maximum value */, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

/***************************** Convenient ADC Delay ***************************/

#define FMC250M_4CH_IDELAY_CLK_LINE                             (0x01 << 16)    /* Bit 16 is clock */
#define FMC250M_4CH_IDELAY_DATA_LINES                           0x00FFFF        /* Bits 0 to 15 are data */
#define FMC250M_4CH_IDELAY_ALL_LINES                            (FMC250M_4CH_IDELAY_CLK_LINE | \
                                                                    FMC250M_4CH_IDELAY_DATA_LINES)

#define FMC_250M_4CH_IDELAY_LINE_MASK                           WB_FMC_250M_4CH_CSR_CH0_FN_SEL_LINE_MASK
#define FMC_250M_4CH_IDELAY_LINE_SHIFT                          WB_FMC_250M_4CH_CSR_CH0_FN_SEL_LINE_SHIFT
#define FMC_250M_4CH_IDELAY_LINE_W(value)                       WB_FMC_250M_4CH_CSR_CH0_FN_SEL_LINE_W(value)
#define FMC_250M_4CH_IDELAY_LINE_R(reg)                         WB_FMC_250M_4CH_CSR_CH0_FN_SEL_LINE_R(reg)
#define FMC_250M_4CH_IDELAY_LINE_UPDT                           WB_FMC_250M_4CH_CSR_CH0_FN_SEL_UPDATE

#define FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_W(value)         WB_FMC_250M_4CH_CSR_CH0_FN_DLY_DATA_CHAIN_DLY_W(value)
#define FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_R(reg)           WB_FMC_250M_4CH_CSR_CH0_FN_DLY_DATA_CHAIN_DLY_R(reg)

#define FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_W(value)          WB_FMC_250M_4CH_CSR_CH0_FN_DLY_CLK_CHAIN_DLY_W(value)
#define FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_R(reg)            WB_FMC_250M_4CH_CSR_CH0_FN_DLY_CLK_CHAIN_DLY_R(reg)

/* Low-level ADC delay function. Must be called with the correct arguments, so
 * only internal functions shall use this */
static int _fmc250m_4ch_set_adc_dly_ll (smio_t* owner, uint64_t addr_dly,
        uint64_t addr_sel, uint64_t addr_updt, uint32_t dly_val,
        uint32_t dly_type)
{
    uint32_t dly_sel_val = 0;
    uint32_t dly_reg_val = 0;
    uint32_t dly_updt = 0;
    bool is_dly_type_data = (dly_type & DLY_TYPE_DATA)? true : false;
    bool is_dly_type_clk = (dly_type & DLY_TYPE_CLK)? true : false;

    if (is_dly_type_data) {
        /* Prepare line selection register */
        dly_sel_val = FMC_250M_4CH_IDELAY_LINE_W(FMC250M_4CH_IDELAY_DATA_LINES);
        /* Prepare update delay register */
        dly_updt = WB_FMC_250M_4CH_CSR_ADC_CTL_UPDATE_DATA_DLY;
        /* Write data delay register */
        dly_reg_val = FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_W(dly_val);
    }

    if (is_dly_type_clk) {
        /* Prepare line selection register */
        dly_sel_val |= FMC_250M_4CH_IDELAY_LINE_W(FMC250M_4CH_IDELAY_CLK_LINE);
        /* Prepare update delay register */
        dly_updt |= WB_FMC_250M_4CH_CSR_ADC_CTL_UPDATE_CLK_DLY;
        /* Write clock delay register */
        dly_reg_val |= FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_W(dly_val);
    }

    /* Write data/clk delay value */
    smio_thsafe_client_write_32 (owner, addr_dly, &dly_reg_val);
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
            "[sm_io:fmc250m_4ch] ADC data delay set to %u\n",
            FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_R(dly_reg_val));
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
            "[sm_io:fmc250m_4ch] ADC clock delay set to %u\n",
            FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_R(dly_reg_val));

    /* Write delay lines selection */
    smio_thsafe_client_write_32 (owner, addr_sel, &dly_sel_val);
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
            "[sm_io:fmc250m_4ch] ADC delay lines value set to %u\n", dly_sel_val);

    /* Update delay registers */
    smio_thsafe_client_write_32 (owner, addr_updt, &dly_updt);
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
            "[sm_io:fmc250m_4ch] ADC delay update set to %u\n", dly_updt);

    /* Do a readback test to guarantee the delay is set correctly. FIXME: FPGA
     * might not implement readback values for all LINES! So, this test might
     * fail */
    uint32_t dly_val_rb = 0;
    usleep (1000);

    if (is_dly_type_data) {
        smio_thsafe_client_read_32 (owner, addr_dly, &dly_val_rb);
        DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
                "[sm_io:fmc250m_4ch] ADC data delay read value is %u\n",
                FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_R(dly_val_rb));
        ASSERT_TEST(FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_R(dly_val_rb) ==
                FMC_250M_4CH_IDELAY_CAL_DATA_CHAIN_DLY_R(dly_reg_val),
                "Could not set ADC data delay correctly. Readback test failed", err_adc_dly);
    }

    if (is_dly_type_clk) {
        smio_thsafe_client_read_32 (owner, addr_dly, &dly_val_rb);
        DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE,
                "[sm_io:fmc250m_4ch] ADC clk delay read value is %u\n",
                FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_R(dly_val_rb));
        ASSERT_TEST(FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_R(dly_val_rb) ==
                FMC_250M_4CH_IDELAY_CAL_CLK_CHAIN_DLY_R(dly_reg_val),
                "Could not set ADC clk delay correctly. Readback test failed", err_adc_dly);
    }

    return -FMC250M_4CH_OK;

err_adc_dly:
    return -FMC250M_4CH_ERR;
}

/* Macros to avoid repetition of the function body */
#define FMC250M_4CH_ADC_DLY_FUNC_NAME(channel)                                  \
    _fmc250m_4ch_adc_dly ## channel

#define FMC250M_4CH_ADC_DLY_FUNC_HEADER(channel)                                \
    static int FMC250M_4CH_ADC_DLY_FUNC_NAME(channel) (void *owner, void *args, void *ret)

#define FMC250M_4CH_ADC_DLY_FUNC_BODY(owner, args, ret, channel)                \
    do {                                                                        \
        UNUSED(ret);                                                             \
        assert (owner);                                                         \
        assert (args);                                                          \
                                                                                \
        SMIO_OWNER_TYPE *self = SMIO_EXP_OWNER(owner);                          \
        uint32_t dly_type = *(uint32_t *) EXP_MSG_ZMQ_FIRST_ARG(args);          \
        uint32_t dly_val = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);            \
                                                                                \
        /* Check if dly_type is withing range */                                \
        ASSERT_TEST((dly_type & ~DLY_TYPE_ALL) == 0, "Delay type is invalid",   \
                err_dly_type_range);                                            \
                                                                                \
        /* Delay value will be masked inside _fmc250m_4ch_set_adc_dly_ll */     \
                                                                                \
        return _fmc250m_4ch_set_adc_dly_ll (self,                               \
                WB_FMC_250M_4CH_CSR_REG_CH ## channel ## _FN_DLY,               \
                WB_FMC_250M_4CH_CSR_REG_CH ## channel ## _FN_SEL,               \
                WB_FMC_250M_4CH_CSR_REG_ADC_CTL,                                \
                dly_val,                                                        \
                dly_type);                                                      \
                                                                                \
err_dly_type_range:                                                             \
        return -FMC250M_4CH_ERR;                                                \
    } while(0)

FMC250M_4CH_ADC_DLY_FUNC_HEADER(0)
{
    FMC250M_4CH_ADC_DLY_FUNC_BODY(owner, args, ret, 0);
}

FMC250M_4CH_ADC_DLY_FUNC_HEADER(1)
{
    FMC250M_4CH_ADC_DLY_FUNC_BODY(owner, args, ret, 1);
}

FMC250M_4CH_ADC_DLY_FUNC_HEADER(2)
{
    FMC250M_4CH_ADC_DLY_FUNC_BODY(owner, args, ret, 2);
}

FMC250M_4CH_ADC_DLY_FUNC_HEADER(3)
{
    FMC250M_4CH_ADC_DLY_FUNC_BODY(owner, args, ret, 3);
}

/***************************** ADC ISLA216P Control ***************************/

#define HALCS_FMC250M_4CH_RST_ADCS_MIN            0 /* Do nothing on RST_ADCS pin */
#define HALCS_FMC250M_4CH_RST_ADCS_MAX            1 /* Pulse RST_ADCS pin */

RW_PARAM_FUNC(fmc250m_4ch, rst_adcs) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, RST_ADCS, SINGLE_BIT_PARAM,
            HALCS_FMC250M_4CH_RST_ADCS_MIN, HALCS_FMC250M_4CH_RST_ADCS_MAX, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

#define HALCS_FMC250M_4CH_RST_DIV_ADCS_MIN        0  /* Do nothing on RST_DIV_ADCS pin */
#define HALCS_FMC250M_4CH_RST_DIV_ADCS_MAX        1  /* Pulse RST_DIV_ADCS pin */

RW_PARAM_FUNC(fmc250m_4ch, rst_div_adcs) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, RST_DIV_ADCS, SINGLE_BIT_PARAM,
            HALCS_FMC250M_4CH_RST_DIV_ADCS_MIN, HALCS_FMC250M_4CH_RST_DIV_ADCS_MAX, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

#define HALCS_FMC250M_4CH_SLEEP_ADCS_MIN        	0  /* Do nothing on SLEEP_ADCS pin */
#define HALCS_FMC250M_4CH_SLEEP_ADCS_MAX        	1  /* Pulse SLEEP_ADCS pin */

RW_PARAM_FUNC(fmc250m_4ch, sleep_adcs) {
    SET_GET_PARAM(fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR,
            ADC_CTL, SLEEP_ADCS, SINGLE_BIT_PARAM,
            HALCS_FMC250M_4CH_SLEEP_ADCS_MIN, HALCS_FMC250M_4CH_SLEEP_ADCS_MAX, NO_CHK_FUNC,
            NO_FMT_FUNC, SET_FIELD);
}

/* Macros to avoid repetition of the function body ISLA216P */
typedef smch_err_e (*smch_isla216p_func_fp) (smch_isla216p_t *self, uint32_t *param);

#define FMC250M_4CH_ISLA216P_FUNC_NAME(func_name)                                 \
        _fmc250m_4ch_isla216p_ ## func_name

#define FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(func_name)                          \
        static int FMC250M_4CH_ISLA216P_FUNC_NAME(func_name) (void *owner, void *args, void *ret)

#define FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, max_channel, read_func, write_func,   \
                error_msg)                                                        \
        do {                                                                      \
            assert (owner);                                                       \
            assert (args);                                                        \
                                                                                  \
            int err = -FMC250M_4CH_OK;                                            \
            SMIO_OWNER_TYPE *self = SMIO_EXP_OWNER(owner);                        \
            smio_fmc250m_4ch_t *fmc250m = smio_get_handler (self);                \
            ASSERT_TEST(fmc250m != NULL, "Could not get SMIO FMC250M handler",    \
                    err_get_fmc250m_handler, -FMC250M_4CH_ERR);                   \
            uint32_t rw = *(uint32_t *) EXP_MSG_ZMQ_FIRST_ARG(args);              \
            uint32_t channel = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);          \
            uint32_t param = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);            \
                                                                                  \
            ASSERT_TEST(channel < max_channel, "Received ISLA216P channel too big", \
                    err_rcv_channel, -FMC250M_4CH_ERR);                           \
                                                                                  \
            DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] Calling " \
                    "ISLA216P function, for channel #%u\n", channel);              \
                                                                                   \
            smch_isla216p_t *smch_isla216p = SMIO_ISLA216P_HANDLER(fmc250m, channel); \
                                                                                   \
            smch_err_e serr = SMCH_SUCCESS;                                        \
            /* Call specific function */                                           \
            if (rw) {                                                              \
                WHEN(ISEMPTY(read_func))(                                          \
                    UNUSED(ret);                                                    \
                    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                            "ISLA216P read function not implemented\n");           \
                    err = -FMC250M_4CH_UNINPL;                                     \
                    return err;                                                    \
                )                                                                  \
                WHENNOT(ISEMPTY(read_func))(                                       \
                    UNUSED(param);                                                  \
                    uint32_t value = 0;                                            \
                    serr = ((smch_isla216p_func_fp) read_func) (smch_isla216p, \
                            &value);                                               \
                    if (serr != SMCH_SUCCESS) {                                    \
                        err = -FMC250M_4CH_ERR;                                    \
                    }                                                              \
                    else {                                                         \
                        *((uint32_t *) ret) = value;                               \
                        err = sizeof (value);                                      \
                        DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                                "ISLA216P function read value = 0x%08X\n", value); \
                    }                                                              \
                )                                                                  \
            }                                                                      \
            else {                                                                 \
                WHEN(ISEMPTY(write_func))(                                         \
                    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                            "ISLA216P write function not implemented\n");          \
                    err = -FMC250M_4CH_UNINPL;                                     \
                    return err;                                                    \
                )                                                                  \
                WHENNOT(ISEMPTY(write_func))(                                      \
                    serr = ((smch_isla216p_func_fp) write_func) (smch_isla216p, \
                            &param);                                               \
                    if (serr != SMCH_SUCCESS) {                                    \
                        err = -FMC250M_4CH_ERR;                                    \
                    }                                                              \
                    else {                                                         \
                        err = -FMC250M_4CH_OK;                                     \
                    }                                                              \
                )                                                                  \
            }                                                                      \
                                                                                   \
    err_rcv_channel:                                                               \
    err_get_fmc250m_handler:                                                       \
            return err;                                                            \
        } while(0)

typedef smch_err_e (*smch_isla216p_func_fp2) (smch_isla216p_t *self, uint32_t *param1, uint32_t *param2);

/* FIXME. Code almost the same as above */
#define FMC250M_4CH_ISLA216P_FUNC_BODY2(owner, args, ret, max_channel, read_func, write_func,   \
                error_msg)                                                        \
        do {                                                                      \
            assert (owner);                                                       \
            assert (args);                                                        \
                                                                                  \
            int err = -FMC250M_4CH_OK;                                            \
            SMIO_OWNER_TYPE *self = SMIO_EXP_OWNER(owner);                        \
            smio_fmc250m_4ch_t *fmc250m = smio_get_handler (self);                \
            ASSERT_TEST(fmc250m != NULL, "Could not get SMIO FMC250M handler",    \
                    err_get_fmc250m_handler, -FMC250M_4CH_ERR);                   \
            uint32_t rw = *(uint32_t *) EXP_MSG_ZMQ_FIRST_ARG(args);              \
            uint32_t channel = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);          \
            uint32_t param1 = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);           \
            uint32_t param2 = *(uint32_t *) EXP_MSG_ZMQ_NEXT_ARG(args);           \
                                                                                  \
            ASSERT_TEST(channel < max_channel, "Received ISLA216P channel too big", \
                    err_rcv_channel, -FMC250M_4CH_ERR);                           \
                                                                                  \
            DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] Calling " \
                    "ISLA216P function, for channel #%u\n", channel);              \
                                                                                   \
            smch_isla216p_t *smch_isla216p = SMIO_ISLA216P_HANDLER(fmc250m, channel); \
                                                                                   \
            smch_err_e serr = SMCH_SUCCESS;                                        \
            /* Call specific function */                                           \
            if (rw) {                                                              \
                WHEN(ISEMPTY(read_func))(                                          \
                    UNUSED(ret);                                                    \
                    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                            "ISLA216P read function not implemented\n");           \
                    err = -FMC250M_4CH_UNINPL;                                     \
                    return err;                                                    \
                )                                                                  \
                WHENNOT(ISEMPTY(read_func))(                                       \
                    uint32_t value = 0;                                            \
                    serr = ((smch_isla216p_func_fp2) read_func) (smch_isla216p,    \
                            &param1, &value);                                      \
                    if (serr != SMCH_SUCCESS) {                                    \
                        err = -FMC250M_4CH_ERR;                                    \
                    }                                                              \
                    else {                                                         \
                        *((uint32_t *) ret) = value;                               \
                        err = sizeof (value);                                      \
                        DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                                "ISLA216P function read value = 0x%08X\n", value); \
                    }                                                              \
                )                                                                  \
            }                                                                      \
            else {                                                                 \
                WHEN(ISEMPTY(write_func))(                                         \
                    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] " \
                            "ISLA216P write function not implemented\n");          \
                    err = -FMC250M_4CH_UNINPL;                                     \
                    return err;                                                    \
                )                                                                  \
                WHENNOT(ISEMPTY(write_func))(                                      \
                    serr = ((smch_isla216p_func_fp2) write_func) (smch_isla216p,   \
                            &param1, &param2);                                     \
                    if (serr != SMCH_SUCCESS) {                                    \
                        err = -FMC250M_4CH_ERR;                                    \
                    }                                                              \
                    else {                                                         \
                        err = -FMC250M_4CH_OK;                                     \
                    }                                                              \
                )                                                                  \
            }                                                                      \
                                                                                   \
    err_rcv_channel:                                                               \
    err_get_fmc250m_handler:                                                       \
            return err;                                                            \
        } while(0)

static smch_err_e smch_isla216p_test_mode_compat (smch_isla216p_t *self,
        uint32_t *mode)
{
    uint8_t test_mode = *(uint8_t *) mode;
    return smch_isla216p_set_test_mode (self, test_mode);
}

#define HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL      4

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(test_mode)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            /* No read function */, smch_isla216p_test_mode_compat,
            "Could not set/get ISLA216P test mode");
}

static smch_err_e smch_isla216p_set_rst_compat (smch_isla216p_t *self,
        uint32_t *rst_arg)
{
    uint8_t rst = *(uint8_t *) rst_arg;
    return smch_isla216p_set_rst (self, rst);
}

static smch_err_e smch_isla216p_get_rst_compat (smch_isla216p_t *self,
        uint32_t *rst_arg)
{
    return smch_isla216p_get_rst (self, (uint8_t *) rst_arg);
}

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(rst)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            smch_isla216p_get_rst_compat, smch_isla216p_set_rst_compat,
            "Could not set/get ISLA216P reset");
}

static smch_err_e smch_isla216p_portconfig_compat (smch_isla216p_t *self,
        uint32_t *portconfig_arg)
{
    uint8_t portconfig = *(uint8_t *) portconfig_arg;
    return smch_isla216p_set_portconfig (self, portconfig);
}

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(portconfig)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            /* No read function */, smch_isla216p_portconfig_compat,
            "Could not set/get portconfig");
}

static smch_err_e smch_isla216p_set_reg_compat (smch_isla216p_t *self,
        uint32_t *addr_arg, uint32_t *data_arg)
{
    uint8_t addr = *(uint8_t *) addr_arg;
    const uint8_t *data = (const uint8_t *) data_arg;
    return smch_isla216p_write_8 (self, addr, data);
}

static smch_err_e smch_isla216p_get_reg_compat (smch_isla216p_t *self,
        uint32_t *addr_arg, uint32_t *data_arg)
{
    uint8_t addr = *(uint8_t *) addr_arg;
    uint8_t *data = (uint8_t *) data_arg;
    return smch_isla216p_read_8 (self, addr, data);
}

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(reg)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY2(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            smch_isla216p_get_reg_compat, smch_isla216p_set_reg_compat,
            "Could not set/get ISLA216P register");
}

static smch_err_e smch_isla216p_temp_compat (smch_isla216p_t *self,
        uint32_t *temp_code_arg)
{
    uint16_t *temp_code = (uint16_t *) temp_code_arg;
    return smch_isla216p_get_temp (self, temp_code);
}

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(temp)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            smch_isla216p_temp_compat, /* No write function */,
            "Could not set/get temperature");
}

static smch_err_e smch_isla216p_cal_status_compat (smch_isla216p_t *self,
        uint32_t *cal_status_arg)
{
    uint8_t *cal_status_code = (uint8_t *) cal_status_arg;
    return smch_isla216p_get_cal_status (self, cal_status_code);
}

FMC250M_4CH_ISLA216P_FUNC_NAME_HEADER(cal_status)
{
    FMC250M_4CH_ISLA216P_FUNC_BODY(owner, args, ret, HALCS_FMC250M_4CH_ISLA216P_MAX_CHANNEL,
            smch_isla216p_cal_status_compat, /* No write function */,
            "Could not set/get cal_statuserature");
}

/* Exported function pointers */
const disp_table_func_fp fmc250m_4ch_exp_fp [] = {
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data0),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data1),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data2),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data3),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data_dly0),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data_dly1),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data_dly2),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_data_dly3),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_clk_dly0),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_clk_dly1),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_clk_dly2),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_clk_dly3),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_line0),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_line1),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_line2),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_line3),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_updt0),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_updt1),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_updt2),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, adc_dly_updt3),
    FMC250M_4CH_ADC_DLY_FUNC_NAME(0),
    FMC250M_4CH_ADC_DLY_FUNC_NAME(1),
    FMC250M_4CH_ADC_DLY_FUNC_NAME(2),
    FMC250M_4CH_ADC_DLY_FUNC_NAME(3),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, rst_adcs),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, rst_div_adcs),
    RW_PARAM_FUNC_NAME(fmc250m_4ch, sleep_adcs),
    FMC250M_4CH_ISLA216P_FUNC_NAME(test_mode),
    FMC250M_4CH_ISLA216P_FUNC_NAME(rst),
    FMC250M_4CH_ISLA216P_FUNC_NAME(portconfig),
    FMC250M_4CH_ISLA216P_FUNC_NAME(reg),
    FMC250M_4CH_ISLA216P_FUNC_NAME(temp),
    FMC250M_4CH_ISLA216P_FUNC_NAME(cal_status),
    NULL
};

/************************************************************/
/***************** Export methods functions *****************/
/************************************************************/

static smio_err_e _fmc250m_4ch_do_op (void *owner, void *msg);

/* Attach an instance of sm_io to dev_io function pointer */
smio_err_e fmc250m_4ch_attach (smio_t *self, void *args)
{
    UNUSED(self);
    UNUSED(args);
    return SMIO_ERR_FUNC_NOT_IMPL;
}

/* Deattach an instance of sm_io to dev_io function pointer */
smio_err_e fmc250m_4ch_deattach (smio_t *self)
{
    UNUSED(self);
    return SMIO_ERR_FUNC_NOT_IMPL;
}

/* Export (register) sm_io to handle operations function pointer */
smio_err_e fmc250m_4ch_export_ops (smio_t *self,
        const disp_op_t** smio_exp_ops)
{
    UNUSED(self);
    UNUSED(smio_exp_ops);
    return SMIO_ERR_FUNC_NOT_IMPL;
}

/* Unexport (unregister) sm_io to handle operations function pointer */
smio_err_e fmc250m_4ch_unexport_ops (smio_t *self)
{
    UNUSED(self);
    return SMIO_ERR_FUNC_NOT_IMPL;
}


/* Generic wrapper for receiving opcodes and arguments to specific funtions function pointer */
/* FIXME: Code repetition! _devio_do_smio_op () function does almost the same!!! */
smio_err_e _fmc250m_4ch_do_op (void *owner, void *msg)
{
    UNUSED(owner);
    UNUSED(msg);
    return SMIO_ERR_FUNC_NOT_IMPL;
}

#define FMC250M_4CH_ISLA216P_INV_TEMP_CODE 	ISLA216P_TEMP_COUNTER_MASK
#define FMC250M_4CH_ISLA216P_RST_MAX_TRIES      10

smio_err_e _fmc250m_4ch_do_mgmt_op (void *owner, void *msg)
{
    assert (owner);
    assert (msg);

    smio_err_e err = SMIO_SUCCESS;
    SMIO_OWNER_TYPE *self = SMIO_EXP_OWNER(owner);
    smio_fmc250m_4ch_t *fmc250m = smio_get_handler (self);
    GEN_MSG_ZMQ_TYPE *recv_msg = GEN_MSG_ZMQ(msg);
    uint32_t my_inst_id = smio_get_inst_id (self);

    /* We expect the following */
    /*
     * Arg1:    (uint32_t) smio_id
     * Arg2:    (uint64_t) base
     * Arg3:    (uint32_t) inst_id
     * Arg4:    (uint32_t) dest_smio_id
     * Arg5:    (uint32_t) dest_inst_id
     * Arg6:    (string)   message
     * */
    STR_MSG_ZMQ_TYPE *smio_id_str = STR_MSG_ZMQ_FIRST_ARG(recv_msg);
    STR_MSG_ZMQ_TYPE *base_str = STR_MSG_ZMQ_NEXT_ARG(recv_msg);
    STR_MSG_ZMQ_TYPE *inst_id_str = STR_MSG_ZMQ_NEXT_ARG(recv_msg);
    STR_MSG_ZMQ_TYPE *dest_smio_id_str = STR_MSG_ZMQ_NEXT_ARG(recv_msg);
    STR_MSG_ZMQ_TYPE * dest_inst_id_str = STR_MSG_ZMQ_NEXT_ARG(recv_msg);
    STR_MSG_ZMQ_TYPE *mgmt_msg_zmq_str = STR_MSG_ZMQ_NEXT_ARG(recv_msg);

    /* Conversions */
    uint32_t inst_id = strtol(inst_id_str, NULL, 10);

    /* Check if FMC_ACTIVE_CLK SMIO sent this message */
    if (inst_id == my_inst_id && streq (mgmt_msg_zmq_str, "INIT_OK")) {
        DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:_fmc250m_4ch_do_mgmt_op] Resetting ADCs\n");
        /* Reset ISLA216P. Do the same as the config_defaults () routine
         * would do. We call this late initialization */
        uint32_t tries = 0;
        bool init_err = true;

        for (tries = 0; tries < FMC250M_4CH_ISLA216P_RST_MAX_TRIES && init_err; ++tries) {
            /* Let's be sure we are in PIN CONTROL before proceeding */
            uint32_t i;
            for (i = 0; i < NUM_FMC250M_4CH_ISLA216P; ++i) {
                smch_isla216p_t *smch_isla216p = SMIO_ISLA216P_HANDLER(fmc250m, i);
                smch_isla216p_set_rst (smch_isla216p, FMC250M_4CH_PINCTRL_RST_MODE_ADC);
            }

            SET_PARAM(self, fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR, ADC_CTL,
                    RST_ADCS, SINGLE_BIT_PARAM, FMC250M_4CH_DFLT_RST_ADCS, /* min */, /* max */,
                    NO_CHK_FUNC, SET_FIELD);
            SET_PARAM(self, fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR, ADC_CTL,
                    RST_DIV_ADCS, SINGLE_BIT_PARAM, FMC250M_4CH_DFLT_RST_DIV_ADCS, /* min */, /* max */,
                    NO_CHK_FUNC, SET_FIELD);
            SET_PARAM(self, fmc250m_4ch, 0x0, WB_FMC_250M_4CH_CSR, ADC_CTL,
                    SLEEP_ADCS, SINGLE_BIT_PARAM, FMC250M_4CH_DFLT_SLEEP_ADCS, /* min */, /* max */,
                    NO_CHK_FUNC, SET_FIELD);

            for (i = 0; i < NUM_FMC250M_4CH_ISLA216P; ++i) {
                smch_isla216p_t *smch_isla216p = SMIO_ISLA216P_HANDLER(fmc250m, i);
                smch_isla216p_set_rst (smch_isla216p, FMC250M_4CH_DFLT_RST_MODE_ADC);
                smch_isla216p_set_portconfig (smch_isla216p, FMC250M_4CH_DFLT_PORTCONFIG_ADC);
                usleep (1000);
                smch_isla216p_set_portconfig (smch_isla216p, FMC250M_4CH_DFLT_RESET_ADC);
                usleep (1000);
                smch_isla216p_set_portconfig (smch_isla216p, FMC250M_4CH_DFLT_PORTCONFIG_ADC);
                usleep (1000);

                /* Check if we can read ADC temperature code. If the code is 0x7FF it means the ADC
                 * was not reset properly. Most likely due to ADC input clock not present */
                uint16_t temp_code = FMC250M_4CH_ISLA216P_INV_TEMP_CODE;
                smch_err_e serr = smch_isla216p_get_temp (smch_isla216p, &temp_code);
                /* Try resetting ADCs again */
                if (serr != SMCH_SUCCESS || temp_code == FMC250M_4CH_ISLA216P_INV_TEMP_CODE) {
                    DBE_DEBUG (DBG_SM_IO | DBG_LVL_WARN, "[sm_io:_fmc250m_4ch_do_mgmt_op] ADC %u reported "
                        "invalid temp. code in reset try %u\n", i, tries);
                    init_err = true;
                    /* Give some time for ADCs to settle */
                    usleep (1000);
                    break;
                }
                /* All ADCs reported good temperature codes */
                init_err = false;
            }
        }

        if (init_err)  {
            DBE_DEBUG (DBG_SM_IO | DBG_LVL_WARN, "[sm_io:_fmc250m_4ch_do_mgmt_op] Exceeded maximum ADC reset tryouts. "
                "Some ADCs might not be reliable. Reset it manually!\n");
            err = SMIO_ERR_CONFIG_DFLT;

        }
    }
    else {
        DBE_DEBUG (DBG_SM_IO | DBG_LVL_WARN, "[sm_io:_fmc250m_4ch_do_mgmt_op] Unexpected message received\n");
    }

    /* Clenup strings */
    STR_MSG_ZMQ_CLENUP_ARG(smio_id_str);
    STR_MSG_ZMQ_CLENUP_ARG(base_str);
    STR_MSG_ZMQ_CLENUP_ARG(inst_id_str);
    STR_MSG_ZMQ_CLENUP_ARG(dest_smio_id_str);
    STR_MSG_ZMQ_CLENUP_ARG(dest_inst_id_str);
    STR_MSG_ZMQ_CLENUP_ARG(mgmt_msg_zmq_str);

    return err;
}

smio_err_e fmc250m_4ch_do_op (void *self, void *msg)
{
    return _fmc250m_4ch_do_op (self, msg);
}

smio_err_e fmc250m_4ch_do_mgmt_op (void *self, void *msg)
{
    return _fmc250m_4ch_do_mgmt_op (self, msg);
}

const smio_ops_t fmc250m_4ch_ops = {
    .attach             = &fmc250m_4ch_attach,          /* Attach sm_io instance to dev_io */
    .deattach           = &fmc250m_4ch_deattach,        /* Deattach sm_io instance to dev_io */
    .export_ops         = &fmc250m_4ch_export_ops,      /* Export sm_io operations to dev_io */
    .unexport_ops       = &fmc250m_4ch_unexport_ops,    /* Unexport sm_io operations to dev_io */
    .do_op              = &fmc250m_4ch_do_op,           /* Generic wrapper for handling specific operations */
    .do_mgmt_op         = &fmc250m_4ch_do_mgmt_op       /* Generic wrapper for handling internal SMIO operations */
};

/************************************************************/
/****************** Bootstrap Operations ********************/
/************************************************************/

smio_err_e fmc250m_4ch_init (smio_t * self)
{
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] Initializing fmc250m_4ch\n");

    smio_err_e err = SMIO_SUCCESS;

    err = smio_set_id (self, FMC250M_4CH_SDB_DEVID);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO id", err_set_id);
    err = smio_set_name (self, FMC250M_4CH_SDB_NAME);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO name", err_set_name);

    /* Set SMIO ops pointers */
    err = smio_set_ops (self, &fmc250m_4ch_ops);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO operations",
            err_smio_set_ops);
    err = smio_set_thsafe_client_ops (self, &smio_thsafe_client_zmq_ops);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO thsafe operations",
            err_smio_set_thsafe_ops);

    /* Fill the disp_op_t description structure with the callbacks. */

    /* disp_op_t structure is const and all of the functions performing on it
     * obviously receives a const argument, but here (and only on the SMIO
     * initialization) we need to make an exception if we want to keep the
     * functions' description and the function pointers separate */
    err = smio_init_exp_ops (self, (disp_op_t **) fmc250m_4ch_exp_ops,
            fmc250m_4ch_exp_fp);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not fill SMIO "
            "function descriptors with the callbacks", err_fill_desc);

    err = smio_set_exp_ops (self, fmc250m_4ch_exp_ops);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO exported operations",
            err_smio_set_exp_ops);

    /* Initialize specific structure */
    smio_fmc250m_4ch_t *smio_handler = smio_fmc250m_4ch_new (self);
    ASSERT_ALLOC(smio_handler, err_smio_handler_alloc, SMIO_ERR_ALLOC);
    err = smio_set_handler (self, smio_handler);
    ASSERT_TEST(err == SMIO_SUCCESS, "Could not set SMIO handler",
            err_smio_set_handler);

    return err;

err_smio_set_handler:
    smio_fmc250m_4ch_destroy (&smio_handler);
err_smio_handler_alloc:
    smio_set_exp_ops (self, NULL);
err_smio_set_exp_ops:
err_fill_desc:
    smio_set_thsafe_client_ops (self, NULL);
err_smio_set_thsafe_ops:
    smio_set_ops (self, NULL);
err_smio_set_ops:
err_set_name:
err_set_id:
    return err;
}

/* Destroy sm_io instance of fmc250m_4ch */
smio_err_e fmc250m_4ch_shutdown (smio_t *self)
{
    DBE_DEBUG (DBG_SM_IO | DBG_LVL_TRACE, "[sm_io:fmc250m_4ch_exp] Shutting down fmc250m_4ch\n");

    smio_err_e err = SMIO_SUCCESS;
    smio_fmc250m_4ch_t *fmc250m = smio_get_handler (self);
    ASSERT_TEST(fmc250m != NULL, "Could not get FMC250M handler",
            err_fmc250m_handler, SMIO_ERR_ALLOC /* FIXME: improve return code */);

    /* Destroy SMIO instance */
    smio_fmc250m_4ch_destroy (&fmc250m);
    /* Nullify operation pointers */
    smio_set_exp_ops (self, NULL);
    smio_set_thsafe_client_ops (self, NULL);
    smio_set_ops (self, NULL);

err_fmc250m_handler:
    return err;
}

const smio_bootstrap_ops_t fmc250m_4ch_bootstrap_ops = {
    .init            = &fmc250m_4ch_init,
    .shutdown        = &fmc250m_4ch_shutdown,
    .config_defaults = &fmc250m_4ch_config_defaults
};

SMIO_MOD_DECLARE(FMC250M_4CH_SDB_DEVID, FMC250M_4CH_SDB_NAME, fmc250m_4ch_bootstrap_ops)
