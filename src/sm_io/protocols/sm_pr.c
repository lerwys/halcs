/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
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
    ASSERT_HAL_TEST(test_boolean, SM_PR, "[sm_pr]",         \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_ALLOC(ptr, SM_PR, "[sm_pr]",                 \
            smpr_err_str(SMPR_ERR_ALLOC),                   \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                            \
    CHECK_HAL_ERR(err, SM_PR, "[sm_pr]",                    \
            smpr_err_str (err_type))

struct _smpr_t {
    void *proto_handler;                /* Generic pointer to a protocol handler. This
                                            must be cast to a specific type by the
                                            specific protocol functions */
    char *name;                         /* Identification of this smpr instance */
    int verbose;                        /* Print activity to stdout */

    /* SMIO instance this SMPR belongs to. We need this, as
     * all of the low-level operations must go through it */
    smio_t *parent;
    /* Protocol operations */
    const smpr_proto_ops_t *ops;
};

static smpr_err_e _smpr_register_proto_ops (const smpr_proto_ops_t **ops,
        const smpr_proto_ops_t *reg_ops);
static smpr_err_e _smpr_unregister_proto_ops (const smpr_proto_ops_t **ops);

/* Creates a new instance of the Low-level I/O */
smpr_t * smpr_new (char *name, smio_t *parent, const smpr_proto_ops_t *reg_ops,
        int verbose)
{
    assert (name);
    assert (parent);

    smpr_t *self = (smpr_t *) zmalloc (sizeof *self);
    ASSERT_ALLOC(self, err_self_alloc);

    /* Initialize Protocol */
    self->proto_handler = NULL;       /* This is set by the specific protocol functions */
    self->name = strdup (name);
    ASSERT_ALLOC(self->name, err_name_alloc);
    self->verbose = verbose;

    /* Initilialize SMIO parent */
    self->parent = parent;
    ASSERT_TEST(self->parent != NULL, "SMIO parent cannot be NULL",
            err_parent_null);

    /* Attach protocol operation to instance of smpr */
    smpr_err_e err = _smpr_register_proto_ops (&self->ops, reg_ops);
    ASSERT_TEST(err == SMPR_SUCCESS, "Could not register SMPR operation",
            err_register_ops);

    DBE_DEBUG (DBG_SM_PR | DBG_LVL_INFO, "[sm_pr] Created instance of smpr\n");
    return self;

err_register_ops:
err_parent_null:
    free (self->name);
err_name_alloc:
    free (self);
err_self_alloc:
    return NULL;
}

/* Destroy an instance of the Low-level I/O */
smpr_err_e smpr_destroy (smpr_t **self_p)
{
    assert (self_p);

    if (*self_p) {
        smpr_t *self = *self_p;

        _smpr_unregister_proto_ops (&self->ops);
        free (self->name);
        free (self);
        *self_p = NULL;
    }

    return SMPR_SUCCESS;
}

/* Register Specific Protocol operations to smpr instance */
smpr_err_e smpr_set_handler (smpr_t *self, void *handler)
{
    assert (self);
    assert (handler);

    smpr_err_e err = SMPR_SUCCESS;

    ASSERT_TEST(self->proto_handler == NULL, "Trying to set another handler to "
            "SMPR instance", err_dup_handler, SMPR_ERR_DUP_HANDLER);
    DBE_DEBUG (DBG_SM_PR | DBG_LVL_INFO, "[sm_pr] Setting protocol handler: %s\n",
            self->name);
    self->proto_handler = handler;

err_dup_handler:
    return err;
}

/* Get protocol handler */
void *smpr_get_handler (smpr_t *self)
{
    assert (self);
    return self->proto_handler;
}

/* Unregister Specific Protocol operations to smpr instance */
void *smpr_unset_handler (smpr_t *self)
{
    assert (self);

    void *proto_handler = self->proto_handler;
    DBE_DEBUG (DBG_SM_PR | DBG_LVL_INFO, "[sm_pr] Unsetting protocol handler: %s\n",
            self->name);
    self->proto_handler = NULL;

    return proto_handler;
}

smio_t *smpr_get_parent (smpr_t *self)
{
    assert (self);
    return self->parent;
}

const smpr_proto_ops_t *smpr_get_ops (smpr_t *self)
{
    assert (self);
    return self->ops;
}

const char *smpr_get_ops_name (smpr_t *self)
{
    assert (self);

    if (self->ops == NULL) {
        return NULL;
    }
    return self->ops->proto_name;
}

/**************** Helper Functions ***************/

/* Register Specific Protocol operations to smpr instance. Helper function */
static smpr_err_e _smpr_register_proto_ops (const smpr_proto_ops_t **ops,
        const smpr_proto_ops_t *reg_ops)
{
    *ops = reg_ops;
    DBE_DEBUG (DBG_SM_PR | DBG_LVL_INFO, "[sm_pr] Proto ops set\n");
    return SMPR_SUCCESS;
}

/* Unregister Specific Protocol operations to smpr instance. Helper function */
static smpr_err_e _smpr_unregister_proto_ops (const smpr_proto_ops_t **ops)
{
    *ops = NULL;
    DBE_DEBUG (DBG_SM_PR | DBG_LVL_INFO, "[sm_pr] Proto ops unset\n");
    return SMPR_SUCCESS;
}

/********* Low-level generic methods API *********/

#define CHECK_FUNC(func_p)                              \
    do {                                                \
        if(func_p == NULL) {                            \
            DBE_DEBUG (DBG_SM_PR | DBG_LVL_ERR,         \
                    "[ll_io] %s\n",                     \
                    smpr_err_str (SMPR_ERR_FUNC_NOT_IMPL)); \
            return -SMPR_ERR_FUNC_NOT_IMPL;             \
        }                                               \
    } while(0)

/* Declare wrapper for all SMPR functions API */
#define SMPR_FUNC_WRAPPER(func_name, ...)               \
{                                                       \
    assert (self);                                      \
    assert (self->ops);                                 \
    CHECK_FUNC (self->ops->func_name);                  \
    return self->ops->func_name (self, ##__VA_ARGS__);  \
}

/**** Open device ****/
int smpr_open (smpr_t *self, uint64_t base, void* args)
    SMPR_FUNC_WRAPPER (proto_open, base, args)

/**** Release device ****/
int smpr_release (smpr_t *self)
    SMPR_FUNC_WRAPPER (proto_release)

/**** Read data from device ****/
ssize_t smpr_read_16 (smpr_t *self, size_t size_offs, uint64_t offs, uint16_t *data)
    SMPR_FUNC_WRAPPER (proto_read_16, size_offs, offs, data)
ssize_t smpr_read_32 (smpr_t *self,size_t size_offs,  uint64_t offs, uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_read_32, size_offs, offs, data)
ssize_t smpr_read_64 (smpr_t *self, size_t size_offs, uint64_t offs, uint64_t *data)
    SMPR_FUNC_WRAPPER (proto_read_64, size_offs, offs, data)

/**** Write data to device ****/
ssize_t smpr_write_16 (smpr_t *self, size_t size_offs, uint64_t offs, const uint16_t *data)
    SMPR_FUNC_WRAPPER (proto_write_16, size_offs, offs, data)
ssize_t smpr_write_32 (smpr_t *self, size_t size_offs, uint64_t offs, const uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_write_32, size_offs, offs, data)
ssize_t smpr_write_64 (smpr_t *self, size_t size_offs, uint64_t offs, const uint64_t *data)
    SMPR_FUNC_WRAPPER (proto_write_64, size_offs, offs, data)

/**** Read data block from device function pointer, size in bytes ****/
ssize_t smpr_read_block (smpr_t *self, size_t size_offs, uint64_t offs, size_t size, uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_read_block, size_offs, offs, size, data)

/**** Write data block from device function pointer, size in bytes ****/
ssize_t smpr_write_block (smpr_t *self, size_t size_offs, uint64_t offs, size_t size, uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_write_block, size_offs, offs, size, data)

/**** Read data block from via DMA from protocol function pointer, size in bytes ****/
ssize_t smpr_read_dma (smpr_t *self, size_t size_offs, uint64_t offs, size_t size, uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_read_dma, size_offs, offs, size, data)

/**** Write data block via DMA from protocol function pointer, size in bytes ****/
ssize_t smpr_write_dma (smpr_t *self, size_t size_offs, uint64_t offs, size_t size, uint32_t *data)
    SMPR_FUNC_WRAPPER (proto_write_dma, size_offs, offs, size, data)
