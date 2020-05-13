/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#ifndef _SM_CH_PCA9547_H_
#define _SM_CH_PCA9547_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SMCH_PCA9547_NO_CHANNEL                 0xFF /* No Channel selected */

/***************** Our methods *****************/

/* Creates a new instance of the SMCH PCA9547 */
smch_pca9547_t * smch_pca9547_new (smio_t *parent, uint64_t base,
        const smpr_proto_ops_t *reg_ops, int verbose);
/* Destroy an instance of the SMCH PCA9547 */
smch_err_e smch_pca9547_destroy (smch_pca9547_t **self_p);

/* Read/Write to PCA9547 single bytes */
smch_err_e smch_pca9547_write_8 (smch_pca9547_t *self, const uint8_t *data);
smch_err_e smch_pca9547_read_8 (smch_pca9547_t *self, uint8_t *data);

/* Enable specific I2C channel */
smch_err_e smch_pca9547_en_chan (smch_pca9547_t *self, uint8_t chan);

#ifdef __cplusplus
}
#endif

#endif
