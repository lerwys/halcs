/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _SM_CH_AD9510_H_
#define _SM_CH_AD9510_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************** Our methods *****************/

/* Creates a new instance of the SMCH AD9510 */
smch_ad9510_t * smch_ad9510_new (smio_t *parent, uint64_t base,
        const smpr_proto_ops_t *reg_ops, int verbose);
/* Destroy an instance of the SMCH AD9510 */
smch_err_e smch_ad9510_destroy (smch_ad9510_t **self_p);

/* Read/Write to/from AD9510 Registers */
smch_err_e smch_ad9510_write_8 (smch_ad9510_t *self, uint8_t addr,
        const uint8_t *data);
smch_err_e smch_ad9510_read_8 (smch_ad9510_t *self, uint8_t addr,
        uint8_t *data);

/* Read/Write to/from AD9510 Registers with Update command */
smch_err_e smch_ad9510_write_8_update (smch_ad9510_t *self, uint8_t addr,
        const uint8_t *data);
smch_err_e smch_ad9510_read_8_update (smch_ad9510_t *self, uint8_t addr,
        uint8_t *data);

/* Update AD9510 registers */
smch_err_e smch_ad9510_reg_update (smch_ad9510_t *self);

/* Simple test for configuring a few AD9510 registers */
smch_err_e smch_ad9510_cfg_defaults (smch_ad9510_t *self);

/* AD9510 PLL divider functions */
smch_err_e smch_ad9510_set_pll_a_div (smch_ad9510_t *self, uint32_t *div);
smch_err_e smch_ad9510_get_pll_a_div (smch_ad9510_t *self, uint32_t *div);

smch_err_e smch_ad9510_set_pll_b_div (smch_ad9510_t *self, uint32_t *div);
smch_err_e smch_ad9510_get_pll_b_div (smch_ad9510_t *self, uint32_t *div);

smch_err_e smch_ad9510_set_pll_prescaler (smch_ad9510_t *self, uint32_t *pre);
smch_err_e smch_ad9510_get_pll_prescaler (smch_ad9510_t *self, uint32_t *pre);

smch_err_e smch_ad9510_set_r_div (smch_ad9510_t *self, uint32_t *div);
smch_err_e smch_ad9510_get_r_div (smch_ad9510_t *self, uint32_t *div);

/* AD9510 PLL control functions */
smch_err_e smch_ad9510_set_pll_pdown (smch_ad9510_t *self, uint32_t *pdown);
smch_err_e smch_ad9510_get_pll_pdown (smch_ad9510_t *self, uint32_t *pdown);

smch_err_e smch_ad9510_set_mux_status (smch_ad9510_t *self, uint32_t *mux);
smch_err_e smch_ad9510_get_mux_status (smch_ad9510_t *self, uint32_t *mux);

smch_err_e smch_ad9510_set_cp_current (smch_ad9510_t *self, uint32_t *cp_current);
smch_err_e smch_ad9510_get_cp_current (smch_ad9510_t *self, uint32_t *cp_current);

smch_err_e smch_ad9510_set_outputs (smch_ad9510_t *self, uint32_t *out_en);
smch_err_e smch_ad9510_get_outputs (smch_ad9510_t *self, uint32_t *out_en);

smch_err_e smch_ad9510_set_pll_clk_sel (smch_ad9510_t *self, uint32_t *clk_num);
smch_err_e smch_ad9510_get_pll_clk_sel (smch_ad9510_t *self, uint32_t *clk_num);

#ifdef __cplusplus
}
#endif

#endif
