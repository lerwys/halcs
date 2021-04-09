/*
 * Copyright (C) 2021 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "acq_chan_gen_defs.h"
#include "boards/afcv3_1/acq_chan.h"
#include "boards/afcv3_1/ddr3_map.h"

#include <stddef.h>

#ifdef __BOARD_AFCV3_1__

/******************************************************/
/*                 Channels definitions               */
/******************************************************/

const size_t NUM_ACQ_CORE_SMIOS         = __NUM_ACQ_CORE_SMIOS;

/* ADC */
const uint32_t ADC_CHAN_ID              = __ADC_CHAN_ID;

/* ADC SWAPPED (after the switching module) */
const uint32_t ADCSWAP_CHAN_ID          = __ADCSWAP_CHAN_ID;

/* MIXER I/Q */
const uint32_t MIXIQ_CHAN_ID            = __MIXIQ_CHAN_ID;

/* DUMMY 0 */
const uint32_t DUMMY0_CHAN_ID           = __DUMMY0_CHAN_ID;

/* TBTDECIM I/Q */
const uint32_t TBTDECIMIQ_CHAN_ID       = __TBTDECIMIQ_CHAN_ID;

/* DUMMY 1 */
const uint32_t DUMMY1_CHAN_ID           = __DUMMY1_CHAN_ID;

/* TBT AMP */
const uint32_t TBTAMP_CHAN_ID           = __TBTAMP_CHAN_ID;

/* TBT PHASE */
const uint32_t TBTPHA_CHAN_ID           = __TBTPHA_CHAN_ID;

/* TBT POS */
const uint32_t TBTPOS_CHAN_ID           = __TBTPOS_CHAN_ID;

/* FOFBDECIM I/Q */
const uint32_t FOFBDECIMIQ_CHAN_ID      = __FOFBDECIMIQ_CHAN_ID;

/* DUMMY 2 */
const uint32_t DUMMY2_CHAN_ID           = __DUMMY2_CHAN_ID;

/* FOFB AMP */
const uint32_t FOFBAMP_CHAN_ID          = __FOFBAMP_CHAN_ID;

/* FOFB PHA */
const uint32_t FOFBPHA_CHAN_ID          = __FOFBPHA_CHAN_ID;

/* FOFB POS */
const uint32_t FOFBPOS_CHAN_ID          = __FOFBPOS_CHAN_ID;

/* MONIT1 AMP */
const uint32_t MONIT1AMP_CHAN_ID        = __MONIT1AMP_CHAN_ID;

/* MONIT1 POS */
const uint32_t MONIT1POS_CHAN_ID        = __MONIT1POS_CHAN_ID;

/* MONIT AMP */
const uint32_t MONITAMP_CHAN_ID         = __MONITAMP_CHAN_ID;

/* MONIT POS */
const uint32_t MONITPOS_CHAN_ID         = __MONITPOS_CHAN_ID;

/* End of channels placeholder */
const uint32_t END_CHAN_ID              = __END_CHAN_ID;

const acq_buf_t __acq_buf[__NUM_ACQ_CORE_SMIOS][__END_CHAN_ID] = {
    /*** Acquisition Core 0 Channel Parameters ***/
    {
        {
            .id = __ADC_CHAN_ID,
            .start_addr = DDR3_ADC0_START_ADDR,
            .end_addr = DDR3_ADC0_END_ADDR
        },
        {
            .id = __ADCSWAP_CHAN_ID,
            .start_addr = DDR3_ADCSWAP0_START_ADDR,
            .end_addr = DDR3_ADCSWAP0_END_ADDR
        },
        {
            .id = __MIXIQ_CHAN_ID,
            .start_addr = DDR3_MIXIQ0_START_ADDR,
            .end_addr = DDR3_MIXIQ0_END_ADDR
        },
        {
            .id = __DUMMY0_CHAN_ID,
            .start_addr = DDR3_DUMMY00_START_ADDR,
            .end_addr = DDR3_DUMMY00_END_ADDR
        },
        {
            .id = __TBTDECIMIQ_CHAN_ID,
            .start_addr = DDR3_TBTDECIMIQ0_START_ADDR,
            .end_addr = DDR3_TBTDECIMIQ0_END_ADDR
        },
        {
            .id = __DUMMY1_CHAN_ID,
            .start_addr = DDR3_DUMMY10_START_ADDR,
            .end_addr = DDR3_DUMMY10_END_ADDR
        },
        {
            .id = __TBTAMP_CHAN_ID,
            .start_addr = DDR3_TBTAMP0_START_ADDR,
            .end_addr = DDR3_TBTAMP0_END_ADDR
        },
        {
            .id = __TBTPHA_CHAN_ID,
            .start_addr = DDR3_TBTPHA0_START_ADDR,
            .end_addr = DDR3_TBTPHA0_END_ADDR
        },
        {
            .id = __TBTPOS_CHAN_ID,
            .start_addr = DDR3_TBTPOS0_START_ADDR,
            .end_addr = DDR3_TBTPOS0_END_ADDR
        },
        {
            .id = __FOFBDECIMIQ_CHAN_ID,
            .start_addr = DDR3_FOFBDECIMIQ0_START_ADDR,
            .end_addr = DDR3_FOFBDECIMIQ0_END_ADDR
        },
        {
            .id = __DUMMY2_CHAN_ID,
            .start_addr = DDR3_DUMMY20_START_ADDR,
            .end_addr = DDR3_DUMMY20_END_ADDR
        },
        {
            .id = __FOFBAMP_CHAN_ID,
            .start_addr = DDR3_FOFBAMP0_START_ADDR,
            .end_addr = DDR3_FOFBAMP0_END_ADDR
        },
        {
            .id = __FOFBPHA_CHAN_ID,
            .start_addr = DDR3_FOFBPHA0_START_ADDR,
            .end_addr = DDR3_FOFBPHA0_END_ADDR
        },
        {
            .id = __FOFBPOS_CHAN_ID,
            .start_addr = DDR3_FOFBPOS0_START_ADDR,
            .end_addr = DDR3_FOFBPOS0_END_ADDR
        },
        {
            .id = __MONIT1AMP_CHAN_ID,
            .start_addr = DDR3_MONIT1AMP0_START_ADDR,
            .end_addr = DDR3_MONIT1AMP0_END_ADDR
        },
        {
            .id = __MONIT1POS_CHAN_ID,
            .start_addr = DDR3_MONIT1POS0_START_ADDR,
            .end_addr = DDR3_MONIT1POS0_END_ADDR
        },
        {
            .id = __MONITAMP_CHAN_ID,
            .start_addr = DDR3_MONITAMP0_START_ADDR,
            .end_addr = DDR3_MONITAMP0_END_ADDR
        },
        {
            .id = __MONITPOS_CHAN_ID,
            .start_addr = DDR3_MONITPOS0_START_ADDR,
            .end_addr = DDR3_MONITPOS0_END_ADDR
        },
    },
    /*** Acquisition Core 1 Channel Parameters ***/
    {
        {
            .id = __ADC_CHAN_ID,
            .start_addr = DDR3_ADC1_START_ADDR,
            .end_addr = DDR3_ADC1_END_ADDR
        },
        {
            .id = __ADCSWAP_CHAN_ID,
            .start_addr = DDR3_ADCSWAP1_START_ADDR,
            .end_addr = DDR3_ADCSWAP1_END_ADDR
        },
        {
            .id = __MIXIQ_CHAN_ID,
            .start_addr = DDR3_MIXIQ1_START_ADDR,
            .end_addr = DDR3_MIXIQ1_END_ADDR
        },
        {
            .id = __DUMMY0_CHAN_ID,
            .start_addr = DDR3_DUMMY01_START_ADDR,
            .end_addr = DDR3_DUMMY01_END_ADDR
        },
        {
            .id = __TBTDECIMIQ_CHAN_ID,
            .start_addr = DDR3_TBTDECIMIQ1_START_ADDR,
            .end_addr = DDR3_TBTDECIMIQ1_END_ADDR
        },
        {
            .id = __DUMMY1_CHAN_ID,
            .start_addr = DDR3_DUMMY11_START_ADDR,
            .end_addr = DDR3_DUMMY11_END_ADDR
        },
        {
            .id = __TBTAMP_CHAN_ID,
            .start_addr = DDR3_TBTAMP1_START_ADDR,
            .end_addr = DDR3_TBTAMP1_END_ADDR
        },
        {
            .id = __TBTPHA_CHAN_ID,
            .start_addr = DDR3_TBTPHA1_START_ADDR,
            .end_addr = DDR3_TBTPHA1_END_ADDR
        },
        {
            .id = __TBTPOS_CHAN_ID,
            .start_addr = DDR3_TBTPOS1_START_ADDR,
            .end_addr = DDR3_TBTPOS1_END_ADDR
        },
        {
            .id = __FOFBDECIMIQ_CHAN_ID,
            .start_addr = DDR3_FOFBDECIMIQ1_START_ADDR,
            .end_addr = DDR3_FOFBDECIMIQ1_END_ADDR
        },
        {
            .id = __DUMMY2_CHAN_ID,
            .start_addr = DDR3_DUMMY21_START_ADDR,
            .end_addr = DDR3_DUMMY21_END_ADDR
        },
        {
            .id = __FOFBAMP_CHAN_ID,
            .start_addr = DDR3_FOFBAMP1_START_ADDR,
            .end_addr = DDR3_FOFBAMP1_END_ADDR
        },
        {
            .id = __FOFBPHA_CHAN_ID,
            .start_addr = DDR3_FOFBPHA1_START_ADDR,
            .end_addr = DDR3_FOFBPHA1_END_ADDR
        },
        {
            .id = __FOFBPOS_CHAN_ID,
            .start_addr = DDR3_FOFBPOS1_START_ADDR,
            .end_addr = DDR3_FOFBPOS1_END_ADDR
        },
        {
            .id = __MONIT1AMP_CHAN_ID,
            .start_addr = DDR3_MONIT1AMP1_START_ADDR,
            .end_addr = DDR3_MONIT1AMP1_END_ADDR
        },
        {
            .id = __MONIT1POS_CHAN_ID,
            .start_addr = DDR3_MONIT1POS1_START_ADDR,
            .end_addr = DDR3_MONIT1POS1_END_ADDR
        },
        {
            .id = __MONITAMP_CHAN_ID,
            .start_addr = DDR3_MONITAMP1_START_ADDR,
            .end_addr = DDR3_MONITAMP1_END_ADDR
        },
        {
            .id = __MONITPOS_CHAN_ID,
            .start_addr = DDR3_MONITPOS1_START_ADDR,
            .end_addr = DDR3_MONITPOS1_END_ADDR
        },
    },
    /*** Acquisition Core 2 Channel Parameters ***/
    {
        {
            .id = __ADC_CHAN_ID,
            .start_addr = DDR3_ADC2_START_ADDR,
            .end_addr = DDR3_ADC2_END_ADDR
        },
        {
            .id = __ADCSWAP_CHAN_ID,
            .start_addr = DDR3_ADCSWAP2_START_ADDR,
            .end_addr = DDR3_ADCSWAP2_END_ADDR
        },
        {
            .id = __MIXIQ_CHAN_ID,
            .start_addr = DDR3_MIXIQ2_START_ADDR,
            .end_addr = DDR3_MIXIQ2_END_ADDR
        },
        {
            .id = __DUMMY0_CHAN_ID,
            .start_addr = DDR3_DUMMY02_START_ADDR,
            .end_addr = DDR3_DUMMY02_END_ADDR
        },
        {
            .id = __TBTDECIMIQ_CHAN_ID,
            .start_addr = DDR3_TBTDECIMIQ2_START_ADDR,
            .end_addr = DDR3_TBTDECIMIQ2_END_ADDR
        },
        {
            .id = __DUMMY1_CHAN_ID,
            .start_addr = DDR3_DUMMY12_START_ADDR,
            .end_addr = DDR3_DUMMY12_END_ADDR
        },
        {
            .id = __TBTAMP_CHAN_ID,
            .start_addr = DDR3_TBTAMP2_START_ADDR,
            .end_addr = DDR3_TBTAMP2_END_ADDR
        },
        {
            .id = __TBTPHA_CHAN_ID,
            .start_addr = DDR3_TBTPHA2_START_ADDR,
            .end_addr = DDR3_TBTPHA2_END_ADDR
        },
        {
            .id = __TBTPOS_CHAN_ID,
            .start_addr = DDR3_TBTPOS2_START_ADDR,
            .end_addr = DDR3_TBTPOS2_END_ADDR
        },
        {
            .id = __FOFBDECIMIQ_CHAN_ID,
            .start_addr = DDR3_FOFBDECIMIQ2_START_ADDR,
            .end_addr = DDR3_FOFBDECIMIQ2_END_ADDR
        },
        {
            .id = __DUMMY2_CHAN_ID,
            .start_addr = DDR3_DUMMY22_START_ADDR,
            .end_addr = DDR3_DUMMY22_END_ADDR
        },
        {
            .id = __FOFBAMP_CHAN_ID,
            .start_addr = DDR3_FOFBAMP2_START_ADDR,
            .end_addr = DDR3_FOFBAMP2_END_ADDR
        },
        {
            .id = __FOFBPHA_CHAN_ID,
            .start_addr = DDR3_FOFBPHA2_START_ADDR,
            .end_addr = DDR3_FOFBPHA2_END_ADDR
        },
        {
            .id = __FOFBPOS_CHAN_ID,
            .start_addr = DDR3_FOFBPOS2_START_ADDR,
            .end_addr = DDR3_FOFBPOS2_END_ADDR
        },
        {
            .id = __MONIT1AMP_CHAN_ID,
            .start_addr = DDR3_MONIT1AMP2_START_ADDR,
            .end_addr = DDR3_MONIT1AMP2_END_ADDR
        },
        {
            .id = __MONIT1POS_CHAN_ID,
            .start_addr = DDR3_MONIT1POS2_START_ADDR,
            .end_addr = DDR3_MONIT1POS2_END_ADDR
        },
        {
            .id = __MONITAMP_CHAN_ID,
            .start_addr = DDR3_MONITAMP2_START_ADDR,
            .end_addr = DDR3_MONITAMP2_END_ADDR
        },
        {
            .id = __MONITPOS_CHAN_ID,
            .start_addr = DDR3_MONITPOS2_START_ADDR,
            .end_addr = DDR3_MONITPOS2_END_ADDR
        },
    },
    /*** Acquisition Core 3 Channel Parameters ***/
    {
        {
            .id = __ADC_CHAN_ID,
            .start_addr = DDR3_ADC3_START_ADDR,
            .end_addr = DDR3_ADC3_END_ADDR
        },
        {
            .id = __ADCSWAP_CHAN_ID,
            .start_addr = DDR3_ADCSWAP3_START_ADDR,
            .end_addr = DDR3_ADCSWAP3_END_ADDR
        },
        {
            .id = __MIXIQ_CHAN_ID,
            .start_addr = DDR3_MIXIQ3_START_ADDR,
            .end_addr = DDR3_MIXIQ3_END_ADDR
        },
        {
            .id = __DUMMY0_CHAN_ID,
            .start_addr = DDR3_DUMMY03_START_ADDR,
            .end_addr = DDR3_DUMMY03_END_ADDR
        },
        {
            .id = __TBTDECIMIQ_CHAN_ID,
            .start_addr = DDR3_TBTDECIMIQ3_START_ADDR,
            .end_addr = DDR3_TBTDECIMIQ3_END_ADDR
        },
        {
            .id = __DUMMY1_CHAN_ID,
            .start_addr = DDR3_DUMMY13_START_ADDR,
            .end_addr = DDR3_DUMMY13_END_ADDR
        },
        {
            .id = __TBTAMP_CHAN_ID,
            .start_addr = DDR3_TBTAMP3_START_ADDR,
            .end_addr = DDR3_TBTAMP3_END_ADDR
        },
        {
            .id = __TBTPHA_CHAN_ID,
            .start_addr = DDR3_TBTPHA3_START_ADDR,
            .end_addr = DDR3_TBTPHA3_END_ADDR
        },
        {
            .id = __TBTPOS_CHAN_ID,
            .start_addr = DDR3_TBTPOS3_START_ADDR,
            .end_addr = DDR3_TBTPOS3_END_ADDR
        },
        {
            .id = __FOFBDECIMIQ_CHAN_ID,
            .start_addr = DDR3_FOFBDECIMIQ3_START_ADDR,
            .end_addr = DDR3_FOFBDECIMIQ3_END_ADDR
        },
        {
            .id = __DUMMY2_CHAN_ID,
            .start_addr = DDR3_DUMMY23_START_ADDR,
            .end_addr = DDR3_DUMMY23_END_ADDR
        },
        {
            .id = __FOFBAMP_CHAN_ID,
            .start_addr = DDR3_FOFBAMP3_START_ADDR,
            .end_addr = DDR3_FOFBAMP3_END_ADDR
        },
        {
            .id = __FOFBPHA_CHAN_ID,
            .start_addr = DDR3_FOFBPHA3_START_ADDR,
            .end_addr = DDR3_FOFBPHA3_END_ADDR
        },
        {
            .id = __FOFBPOS_CHAN_ID,
            .start_addr = DDR3_FOFBPOS3_START_ADDR,
            .end_addr = DDR3_FOFBPOS3_END_ADDR
        },
        {
            .id = __MONIT1AMP_CHAN_ID,
            .start_addr = DDR3_MONIT1AMP3_START_ADDR,
            .end_addr = DDR3_MONIT1AMP3_END_ADDR
        },
        {
            .id = __MONIT1POS_CHAN_ID,
            .start_addr = DDR3_MONIT1POS3_START_ADDR,
            .end_addr = DDR3_MONIT1POS3_END_ADDR
        },
        {
            .id = __MONITAMP_CHAN_ID,
            .start_addr = DDR3_MONITAMP3_START_ADDR,
            .end_addr = DDR3_MONITAMP3_END_ADDR
        },
        {
            .id = __MONITPOS_CHAN_ID,
            .start_addr = DDR3_MONITPOS3_START_ADDR,
            .end_addr = DDR3_MONITPOS3_END_ADDR
        },
    },
};

#endif