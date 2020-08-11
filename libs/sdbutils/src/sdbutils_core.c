/*
 * Copyright (C) 2016 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "sdbutils.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...)  \
    ASSERT_HAL_TEST(test_boolean, HAL_UTILS, "[sdbutils:utils]",                \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...)                   \
    ASSERT_HAL_ALLOC(ptr, HAL_UTILS, "[sdbutils:utils]",                        \
            sdbutils_err_str(SDBUTILS_ERR_ALLOC),                               \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                                                \
    CHECK_HAL_ERR(err, HAL_UTILS, "[sdbutils:utils]",                           \
            sdbutils_err_str (err_type))

#define __SDBUTILS_PRINT(stream, format, ...)                                   \
    ({                                                                          \
        if (stream) {                                                           \
            fprintf (stream, format, ## __VA_ARGS__);                           \
        }                                                                       \
        else {                                                                  \
            DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_INFO | DBG_LVL_SIMPLE, format,   \
                ## __VA_ARGS__);                                                \
        }                                                                       \
    })

/*******************************************************************/
/*********************  SDB Utilities functions ********************/
/*******************************************************************/

/* Copied from libsdbfs/tools/sdb-read, located at:
 * git://ohwr.org/hdl-core-lib/fpga-config-space.git */

/* Boring ascii representation of a device */
sdbutils_err_e sdbutils_list_device_file (struct sdb_device *d, int depth, int base,
        int opt_long, FILE *stream)
{
    sdbutils_err_e err = SDBUTILS_SUCCESS;
    struct sdb_product *p;
    struct sdb_component *c;
    struct sdb_synthesis *s;

    unsigned char *data;
    int i;

    c = &d->sdb_component;
    p = &c->product;
    s = (void *)d;

    /* Different sdb items are listed in different ways */
    switch(p->record_type) {

        /* The following items are components, and are listed as such */
        case sdb_type_interconnect:
        case sdb_type_device:
        case sdb_type_bridge:
        case sdb_type_msi:
            if (!opt_long) {
                __SDBUTILS_PRINT(stream, "%.19s\n", p->name);
                return SDBUTILS_SUCCESS;
            }

            /* hack: show directory level looking at the internals */
            __SDBUTILS_PRINT(stream, "%016llx:%08x @ %08llx-%08llx ",
                    (long long)ntohll(p->vendor_id), ntohl(p->device_id),
                    (long long)base + ntohll(c->addr_first),
                    (long long)base + ntohll(c->addr_last));
            for (i = 0; i < depth; i++) {
                __SDBUTILS_PRINT(stream, "    ");
            }
            __SDBUTILS_PRINT(stream, "%.19s\n", p->name);

            return SDBUTILS_SUCCESS;

            /* A product, but not a component (no address range) */
        case sdb_type_integration:
            if (!opt_long) {
                __SDBUTILS_PRINT(stream, "%.19s\n", p->name);
                return SDBUTILS_SUCCESS;
            }

            __SDBUTILS_PRINT(stream, "%016llx:%08x                     ",
                    (long long)ntohll(p->vendor_id), ntohl(p->device_id));
            /* like above, show directory level */
            for (i = 0; i < depth; i++) {
                __SDBUTILS_PRINT(stream, "    ");
            }
            __SDBUTILS_PRINT(stream, "%.19s\n", p->name);

            return SDBUTILS_SUCCESS;

            /* Just a string */
        case sdb_type_repo_url:
            if (opt_long) {
                __SDBUTILS_PRINT(stream, "repo-url: %.63s\n",
                        ((struct sdb_repo_url *)d)->repo_url);
            }
            return SDBUTILS_SUCCESS;

            /* Some metadata */
        case sdb_type_synthesis:
            if (!opt_long) {
                return SDBUTILS_SUCCESS;
            }

            __SDBUTILS_PRINT(stream, "synthesis-name: %.16s\n", s->syn_name);
            __SDBUTILS_PRINT(stream, "    commit-id: ");
            for (i = 0; i < (int) sizeof(s->commit_id); i++) {
                __SDBUTILS_PRINT(stream, "%02x", s->commit_id[i]);
            }
            __SDBUTILS_PRINT(stream, "\n");

            /* Some of the following fields are sometimes empty */
            if (s->tool_name[0] && s->tool_name[0] != ' ') {
                __SDBUTILS_PRINT(stream, "    tool-name: %.8s\n", s->tool_name);
            }
            if (s->tool_version) {
                __SDBUTILS_PRINT(stream, "    tool-version: 0x%08x\n",
                        ntohl(s->tool_version));
            }
            if (s->date) {
                __SDBUTILS_PRINT(stream, "    build-date: %08x\n", ntohl(s->date));
            }
            if (s->user_name[0] && s->tool_name[0] != ' ') {
                __SDBUTILS_PRINT(stream, "    build-user: %.15s\n", s->user_name);
            }
            return SDBUTILS_SUCCESS;

        case sdb_type_empty:
            return SDBUTILS_SUCCESS;

        default:
            break;
    }

    /* Unknown record type */
    if (p->record_type & 0x80) {
        err = SDBUTILS_SUCCESS;
    } else {
        err = SDBUTILS_ERR_UNK_REC;
    }

    DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_WARN, "Unknown record type 0x%02x\n",
        p->record_type);
    if (!opt_long) {
        __SDBUTILS_PRINT(stream, "Unknown-record\n");
        return err;
    }

    /* long listing of unknown record */
    __SDBUTILS_PRINT(stream, "Unknown-record:\n");
    data = (void *)d;

    for (i = 0; i < (int) sizeof(struct sdb_empty); i++) {
        __SDBUTILS_PRINT(stream, "%s%02x%c",
                (i & 0xf) == 0 ? "   " : "",
                data[i],
                (i & 0xf) == 0xf ? '\n' : ' ');
    }

    return err;
}

sdbutils_err_e sdbutils_list_device (struct sdb_device *d, int depth, int base,
        int opt_long)
{
    return sdbutils_list_device_file (d, depth, base, opt_long, NULL);
}

sdbutils_err_e sdbutils_do_list_file (struct sdbfs *fs, int opt_long, FILE *stream)
{
    sdbutils_err_e err = SDBUTILS_SUCCESS;
    struct sdb_device *d;
    int new = 1;

    while ( (d = sdbfs_scan(fs, new)) != NULL) {
        err = sdbutils_list_device_file (d, fs->depth, fs->base[fs->depth], opt_long,
                stream);
        ASSERT_TEST(err == SDBUTILS_SUCCESS, "Could not list device",
                err_list_device, SDBUTILS_ERR_ALLOC);
        new = 0;
    }

err_list_device:
    return err;
}

sdbutils_err_e sdbutils_do_list (struct sdbfs *fs, int opt_long)
{
    return sdbutils_do_list_file (fs, opt_long, NULL);
}

sdbutils_err_e sdbutils_do_cat_name_file (struct sdbfs *fs, char *name, FILE *stream)
{
    sdbutils_err_e err = SDBUTILS_SUCCESS;
    char buf[4096];
    int i;

    i = sdbfs_open_name (fs, name);
    if (i < 0) {
        DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_WARN, "%s: %s\n",
                name, strerror(-i));
        err = SDBUTILS_ERR_OPEN;
        goto err_open_name;
    }

    while ( (i = sdbfs_fread (fs, -1, buf, sizeof(buf))) > 0) {
        /* Convert bytes to C-string */
        const int namesize = i;
        char name[namesize + 1];

        memcpy (name, buf, i);
        name [namesize] = '\0';
        __SDBUTILS_PRINT(stream, "%s\n", name);
    }

    sdbfs_close(fs);

err_open_name:
    return err;
}

sdbutils_err_e sdbutils_do_cat_name (struct sdbfs *fs, char *name)
{
    return sdbutils_do_cat_name_file (fs, name, NULL);
}

sdbutils_err_e sdbutils_do_cat_id_file (struct sdbfs *fs, uint64_t vendor, uint32_t dev,
        FILE *stream)
{
    sdbutils_err_e err = SDBUTILS_SUCCESS;
    char buf[4096];
    int i;

    i = sdbfs_open_id (fs, htonll(vendor), htonl(dev));
    if (i < 0) {
        DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_WARN, "%016"PRIx64"-%08x: %s\n",
                vendor, dev, strerror (-i));
        err = SDBUTILS_ERR_OPEN;
        goto err_open_id;
    }

    while ( (i = sdbfs_fread (fs, -1, buf, sizeof(buf))) > 0) {
        /* Convert bytes to C-string */
        const int namesize = i;
        char name [namesize + 1];

        memcpy (name, buf, i);
        name [namesize] = '\0';
        __SDBUTILS_PRINT(stream, "%s\n", name);
    }

    sdbfs_close (fs);

err_open_id:
    return err;
}

sdbutils_err_e sdbutils_do_cat_id (struct sdbfs *fs, uint64_t vendor, uint32_t dev)
{
    return sdbutils_do_cat_id_file (fs, vendor, dev, NULL);
}

struct sdb_device *sdbutils_next_device (struct sdbfs *fs)
{
    struct sdb_device *d;
    struct sdb_product *p;
    struct sdb_component *c;

    /* Search for the next device and return it */
    while ((d = sdbfs_scan (fs, 0)) != NULL) {
        c = &d->sdb_component;
        p = &c->product;

        if (p->record_type == sdb_type_device) {
            int base = fs->base[fs->depth];
            /* hack: show directory level looking at the internals */
            DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_INFO, "[sdbutils] Device \"%.19s\" found:\n"
                    "\t%016llx:%08x @ %08llx-%08llx ",
                    p->name,
                    (long long)ntohll(p->vendor_id), ntohl(p->device_id),
                    (long long)base + ntohll(c->addr_first),
                    (long long)base + ntohll(c->addr_last));
            return d;
        }
    }

    return NULL;
}

struct sdb_device *sdbutils_get_sdb_device (struct sdbfs *fs, uint64_t vendor,
        uint32_t dev, int num_device_skip)
{
    int device_count = 0;
    struct sdb_device *d;
    struct sdb_product *p;
    struct sdb_component *c;

    sdbfs_scan(fs, 1); /* new scan: get the interconnect and igore it */
    while ((d = sdbutils_next_device (fs)) != NULL) {
        c = &d->sdb_component;
        p = &c->product;

        /* Device found */
        if ((unsigned long long)ntohll(p->vendor_id) == vendor &&
                ntohl(p->device_id) == dev) {
            device_count++;

            /* And we have skipped num_device_skip devices */
            if (device_count > num_device_skip) {
                DBE_DEBUG (DBG_HAL_UTILS | DBG_LVL_TRACE, "[sdbutils] Device "
                        "vid = 0x%"PRIx64", did = 0x%08x, instance = %d found\n"
                        "\tname = %.19s, abi_class = %02X, abi_major = %01X, abi_minor = %01X\n",
                        ntohll(p->vendor_id), ntohl(p->device_id), num_device_skip, p->name,
                        d->abi_class, d->abi_ver_major, d->abi_ver_minor);
                return d;
            }
        }
    }

    return NULL;
}
