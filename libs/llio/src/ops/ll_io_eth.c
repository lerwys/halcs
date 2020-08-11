/*
 * Copyright (C) 2014 LNLS (www.lnls.br)
 * Author: Lucas Russo <lucas.russo@lnls.br>
 *
 * Released according to the GNU GPL, version 3 or any later version.
 */

#include "ll_io.h"

/* Undef ASSERT_ALLOC to avoid conflicting with other ASSERT_ALLOC */
#ifdef ASSERT_TEST
#undef ASSERT_TEST
#endif
#define ASSERT_TEST(test_boolean, err_str, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_TEST(test_boolean, LL_IO, "[ll_io:eth]",    \
            err_str, err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef ASSERT_ALLOC
#undef ASSERT_ALLOC
#endif
#define ASSERT_ALLOC(ptr, err_goto_label, /* err_core */ ...) \
    ASSERT_HAL_ALLOC(ptr, LL_IO, "[ll_io:eth]",             \
            llio_err_str(LLIO_ERR_ALLOC),                   \
            err_goto_label, /* err_core */ __VA_ARGS__)

#ifdef CHECK_ERR
#undef CHECK_ERR
#endif
#define CHECK_ERR(err, err_type)                            \
    CHECK_HAL_ERR(err, LL_IO, "[ll_io:eth]",                \
            llio_err_str (err_type))

/* Our expected endpoint looks like the following:
 * tcp://10.0.0.0:8888
 * udp://10.0.0.0:8888
 * */
#define LLIO_ETH_REGEX                                      \
    "^(tcp|udp)://(\\d+\\.\\d+\\.\\d+\\.\\d+):(\\d+)$"

/* Number of expected hits (socket type, address, port number) +
 * whole pattern */
#define LLIO_ETH_REGEX_HITS                 4
/* Hit indexes */
#define LLIO_ETH_REGEX_SOCK_TYPE_HIT        1
#define LLIO_ETH_REGEX_ADDR_HIT             2
#define LLIO_ETH_REGEX_PORT_HIT             3

#define LLIO_ETH_MAX_RECONNECT_TRIES         1
#define LLIO_ETH_SECS_BEFORE_RECONNECT       1
#define LLIO_ETH_SECS_BEFORE_RECV_TIMEOUT    1

/* Max retries when opening the ETH connection */
#define LLIO_ETH_MAX_OPEN_RECONNECT_TRIES    -1
#define LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT  1

/* Device endpoint */
typedef struct {
    llio_eth_type_e type;
    int fd;
    char *hostname;
    char *port;
} llio_dev_eth_t;

static int _llio_eth_conn (int *fd, llio_eth_type_e type, char *hostname,
        char* port, int secs_before_reconnect);
static void *_get_in_addr(struct sockaddr *sa);
static ssize_t _eth_sendall (int fd, uint8_t *buf, size_t len);
static ssize_t _eth_recvall (int fd, uint8_t *buf, size_t len);
static ssize_t _eth_read_generic (llio_t *self, uint64_t offs, uint32_t *data,
        size_t size, bool auto_reconnect);
static ssize_t _eth_write_generic (llio_t *self, uint64_t offs, const uint32_t *data,
        size_t size, bool auto_reconnect);

/************ Our methods implementation **********/

/* Creates a new instance of the dev_eth */
static llio_dev_eth_t * llio_dev_eth_new (const char *sock_type, const char *hostname,
        const char *port)
{
    assert (sock_type);
    assert (hostname);
    assert (port);

    /* Convert socke type into enum */
    llio_eth_type_e type = llio_str_to_eth_type (sock_type);
    ASSERT_TEST (type != INVALID_ETH_SOCK, "Invalid ethernet socket",
            err_llio_sock_type);

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Sock type is %d\n",
            type);

    llio_dev_eth_t *self = (llio_dev_eth_t *) zmalloc (sizeof *self);
    ASSERT_ALLOC (self, err_llio_dev_eth_alloc);

    /* *Initialize socket type */
    self->type = type;

    self->hostname = strdup (hostname);
    ASSERT_ALLOC(self->hostname, err_hostname_alloc);
    self->port = strdup (port);
    ASSERT_ALLOC(self->port, err_port_alloc);

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Created instance of llio_dev_eth\n");

    return self;

err_port_alloc:
    free (self->hostname);
err_hostname_alloc:
    free (self);
err_llio_dev_eth_alloc:
err_llio_sock_type:
    return NULL;
}

/* Destroy an instance of the Endpoint */
static llio_err_e llio_dev_eth_destroy (llio_dev_eth_t **self_p)
{
    if (*self_p) {
        llio_dev_eth_t *self = *self_p;

        free (self->hostname);
        free (self->port);
        free (self);

        self_p = NULL;
    }

    return LLIO_SUCCESS;
}

/************ llio_ops_eth Implementation **********/

/* Open ETH device */
static int eth_open (llio_t *self, llio_endpoint_t *endpoint)
{
    if (llio_get_endpoint_open (self)) {
        /* Device is already opened. So, we return success */
        return 0;
    }

    llio_err_e lerr = LLIO_SUCCESS;
    int err = 0;
    if (endpoint != NULL) {
        lerr = llio_set_endpoint (self, endpoint);
        ASSERT_TEST(lerr == LLIO_SUCCESS, "Could not set endpoint on ETH device",
                err_endpoint_set, -1);
    }

    /* Parse the endpoint name */
    zrex_t *endp_regex = zrex_new (LLIO_ETH_REGEX);
    ASSERT_ALLOC(endp_regex, err_endp_regex_alloc, -1);

    bool valid = zrex_valid (endp_regex);
    /* Verify possible error on regex expression */
    ASSERT_TEST(valid, "Regex expression is not valid", err_inv_regex_exp, -1);

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Endpoint is %s\n", llio_get_endpoint_name (self));

    const char *endpoint_name = llio_get_endpoint_name (self);
    /* Extract the socket type, host address and port number from the endpoint
     * name */
    bool endp_matches = zrex_matches (endp_regex, endpoint_name);
    ASSERT_TEST(endp_matches == true,
            "Could not match endpoint string to the expected pattern",
            err_endp_match, -1);

    int hits = zrex_hits (endp_regex);
    ASSERT_TEST(hits == LLIO_ETH_REGEX_HITS,
            "Number of LLIO endpoint hits was unexpected",
            err_endp_hits, -1);

    const char *endp_sock_type = zrex_hit (endp_regex, LLIO_ETH_REGEX_SOCK_TYPE_HIT);
    ASSERT_TEST(endp_sock_type != NULL, "Could not retrieve socket type string",
            err_endp_sock_type_retrieve, -1);
    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Endpoint socket type is %s\n", endp_sock_type);

    const char *endp_addr = zrex_hit (endp_regex, LLIO_ETH_REGEX_ADDR_HIT);
    ASSERT_TEST(endp_addr != NULL, "Could not retrieve address string",
            err_endp_addr_retrieve, -1);
    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Endpoint address is %s\n", endp_addr);

    const char *endp_port = zrex_hit (endp_regex, LLIO_ETH_REGEX_PORT_HIT);
    ASSERT_TEST(endp_port != NULL, "Could not retrieve port string",
            err_endp_port_retrieve, -1);
    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Endpoint port is %s\n", endp_port);

    /* Create new private ETH handler */
    llio_dev_eth_t *dev_eth = llio_dev_eth_new (endp_sock_type, endp_addr,
            endp_port);
    ASSERT_TEST(dev_eth != NULL, "Could not allocate dev_handler",
            err_dev_handler_alloc, -1);

    ssize_t retries = LLIO_ETH_MAX_OPEN_RECONNECT_TRIES;
    bool retry_forever_first_time = (retries < 0)? true : false;
    err = _llio_eth_conn (&dev_eth->fd, dev_eth->type, dev_eth->hostname,
        dev_eth->port, LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT);

    while (err < 0) {
        ASSERT_TEST(retries != 0, "No more retries in connecting to endpoint", err_retries_eth_conn);

        if (retry_forever_first_time) {
            DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                    "[ll_io_eth] Error on connection to endpoint the first time, retrying forever\n");
            retry_forever_first_time = false;
        }
        else if (retries >= 0) {
            DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                    "[ll_io_eth] Error on connection to endpoint the first time, retrying (%zu/%u)\n", LLIO_ETH_MAX_OPEN_RECONNECT_TRIES-retries,
                    LLIO_ETH_MAX_OPEN_RECONNECT_TRIES);
        }

        /* Only decrement if we are in finite waiting mode */
        if (retries > 0) {
            retries--;
        }

        /* cleanup and try again */
        close (dev_eth->fd);
        dev_eth->fd = -1;
        err = _llio_eth_conn (&dev_eth->fd, dev_eth->type,
                dev_eth->hostname, dev_eth->port, LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT);
    }

    ASSERT_TEST(err == 0, "Could not connect to endpoint", err_eth_conn);

    llio_set_dev_handler (self, dev_eth);
    /* Signal that the endpoint is opened and ready to work */
    llio_set_endpoint_open (self, true);

    /* Set SDB prefix adress */
    llio_set_sdb_prefix_addr (self, 0x0);

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Opened ETH device located at %s\n",
            llio_get_endpoint_name (self));

err_eth_conn:
err_retries_eth_conn:
err_dev_handler_alloc:
err_endp_port_retrieve:
err_endp_addr_retrieve:
err_endp_sock_type_retrieve:
err_endp_hits:
err_endp_match:
err_inv_regex_exp:
    zrex_destroy (&endp_regex);
err_endp_regex_alloc:
err_endpoint_set:
    return err;
}

/* Release ETH device */
static int eth_release (llio_t *self, llio_endpoint_t *endpoint)
{
    UNUSED(endpoint);

    if (!llio_get_endpoint_open (self)) {
        /* Nothing to close */
        return 0;
    }

    llio_err_e lerr = LLIO_SUCCESS;
    int err = 0;
    llio_dev_eth_t *dev_eth = llio_get_dev_handler (self);
    ASSERT_TEST(dev_eth != NULL, "Could not get ETH handler",
            err_dev_eth_handler, -1);

    /* First destroy the FD handling the socket. This FD
     * is initialized on eth_open (), so the proper place to
     * destroy it is here, not on llio_dev_eth_destroy () */
    close (dev_eth->fd);
    dev_eth->fd = -1;

    /* Deattach specific device handler to generic one */
    lerr = llio_dev_eth_destroy (&dev_eth);
    ASSERT_TEST (lerr==LLIO_SUCCESS, "Could not close device appropriately",
            err_dealloc, -1);

    llio_set_dev_handler (self, NULL);
    llio_set_endpoint_open (self, false);

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Closed ETH device located at %s\n",
            llio_get_endpoint_name (self));

    return err;

err_dealloc:
err_dev_eth_handler:
    return err;
}

/* Read data from Eth device */
static ssize_t eth_read_16 (llio_t *self, uint64_t offs, uint16_t *data)
{
    return _eth_read_generic (self, offs, (uint32_t *) data,
            sizeof (*data), true);
}

static ssize_t eth_read_32 (llio_t *self, uint64_t offs, uint32_t *data)
{
    return _eth_read_generic (self, offs, (uint32_t *) data,
            sizeof (*data), true);
}

static ssize_t eth_read_64 (llio_t *self, uint64_t offs, uint64_t *data)
{
    return _eth_read_generic (self, offs, (uint32_t *) data,
            sizeof (*data), true);
}

/* Write data to Eth device */
static ssize_t eth_write_16 (llio_t *self, uint64_t offs, const uint16_t *data)
{
    return _eth_write_generic (self, offs, (const uint32_t *) data,
            sizeof (*data), true);
}

static ssize_t eth_write_32 (llio_t *self, uint64_t offs, const uint32_t *data)
{
    return _eth_write_generic (self, offs, (const uint32_t *) data,
            sizeof (*data), true);
}

static ssize_t eth_write_64 (llio_t *self, uint64_t offs, const uint64_t *data)
{
    return _eth_write_generic (self, offs, (const uint32_t *) data,
            sizeof (*data), true);
}

/* Read data block from Eth device, size in bytes */
static ssize_t eth_read_block (llio_t *self, uint64_t offs, size_t size, uint32_t *data)
{
    return _eth_read_generic (self, offs, data, size, true);
}

/* Write data block to Eth device, size in bytes */
ssize_t eth_write_block (llio_t *self, uint64_t offs, size_t size, uint32_t *data)
{
    return _eth_write_generic (self, offs, data, size, true);
}

/******************************* Static Functions *****************************/

static ssize_t _eth_read_generic (llio_t *self, uint64_t offs, uint32_t *data,
        size_t size, bool auto_reconnect)
{
    UNUSED(offs);

    ssize_t err = 0;
    size_t retries = (auto_reconnect) ? LLIO_ETH_MAX_RECONNECT_TRIES : 0;
    llio_dev_eth_t *dev_eth = llio_get_dev_handler (self);
    ASSERT_TEST(dev_eth != NULL, "Could not get ETH handler",
            err_dev_eth_handler, -1);

    err = _eth_recvall (dev_eth->fd, (uint8_t *) data, size);
    /* If we got an error, try reconnecting and try up to
     * LLIO_ETH_MAX_RECONNECT_TRIES or forever if retries == -1*/
    while (err < 0) {
        if (retries == 0) {
            break;
        }

        /* Only decrement if we are in finite waiting mode */
        if (retries > 0) {
            retries--;
        }

        /* cleanup and try again */
        close (dev_eth->fd);
        dev_eth->fd = -1;
        err = _llio_eth_conn (&dev_eth->fd, dev_eth->type,
                dev_eth->hostname, dev_eth->port, LLIO_ETH_SECS_BEFORE_RECONNECT);
        /* keep retrying */
        if (err < 0) {
            DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE,
                    "[ll_io_eth] Error connecting to endpoint during recv (). "
                    "Retrying in %u seconds\n",
                    LLIO_ETH_SECS_BEFORE_RECONNECT);
            continue;
        }

        err = _eth_recvall (dev_eth->fd, (uint8_t *) data, size);
    }

err_dev_eth_handler:
    return err;
}

static ssize_t _eth_write_generic (llio_t *self, uint64_t offs, const uint32_t *data,
        size_t size, bool auto_reconnect)
{
    UNUSED(offs);

    ssize_t err = 0;
    size_t retries = (auto_reconnect) ? LLIO_ETH_MAX_RECONNECT_TRIES : 0;
    llio_dev_eth_t *dev_eth = llio_get_dev_handler (self);
    ASSERT_TEST(dev_eth != NULL, "Could not get ETH handler",
            err_dev_eth_handler, -1);

    err = _eth_sendall (dev_eth->fd, (uint8_t *) data, size);
    /* If we got an error, try reconnecting and try up to
     * LLIO_ETH_MAX_RECONNECT_TRIES or forever if retries == -1*/
    while (err < 0) {
        if (retries == 0) {
            break;
        }

        /* Only decrement if we are in finite waiting mode */
        if (retries > 0) {
            retries--;
        }

        /* cleanup and try again */
        close (dev_eth->fd);
        dev_eth->fd = -1;
        err = _llio_eth_conn (&dev_eth->fd, dev_eth->type,
                dev_eth->hostname, dev_eth->port, LLIO_ETH_SECS_BEFORE_RECONNECT);
        /* keep retrying */
        if (err < 0) {
            DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE,
                    "[ll_io_eth] Error connecting to endpoint during send (). "
                    "Retrying in %u seconds\n",
                    LLIO_ETH_SECS_BEFORE_RECONNECT);
            continue;
        }

        err = _eth_sendall (dev_eth->fd, (uint8_t *) data, size);
    }

err_dev_eth_handler:
    return err;
}

/******************************* Helper Functions *****************************/

static int _llio_eth_conn (int *fd, llio_eth_type_e type, char *hostname,
        char* port, int secs_before_reconnect)
{
    int err = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    long arg;
    fd_set myset;
    struct timeval tv;
    int valopt;
    socklen_t lon;
    int yes = 1;

    /* Ignored for now */
    ASSERT_TEST (type == TCP_ETH_SOCK, "Unsupported socket type",
            err_unsup, -1);

    // Socket specific part
    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo (hostname, port, &hints, &servinfo);
    /* DBE_DEBUG (DBG_LL_IO | DBG_LVL_ERR,
       "[ll_io_eth] Error executing getaddrinfo: %s\n", gai_strerror(rv)); */
    ASSERT_TEST (rv == 0, "Could not get address information",
            err_getaddrinfo, -1);

    /* loop through all the results and connect to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((*fd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            DBE_DEBUG (DBG_LL_IO | DBG_LVL_ERR,
                    "[ll_io_eth] Error executing socket: %s\n", strerror(errno));
            continue;
        }

        /* This is important for correct behaviour */
        rv = setsockopt(*fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
        /* DBE_DEBUG (DBG_LL_IO | DBG_LVL_ERR,
           "[ll_io_eth] Error executing setsockpot: %s\n", strerror(errno));*/
        ASSERT_TEST (rv == 0, "Could not set endpoint options",
                err_setsockopt, -1);

        /* Set non-blocking connection. Based on
         * http://developerweb.net/viewtopic.php?id=3196 */
        arg = fcntl (*fd, F_GETFL, NULL);
        ASSERT_TEST (arg >= 0, "Could not get socket FD options",
            err_get_fcntl, -1);
        arg |= O_NONBLOCK;
        rv = fcntl (*fd, F_SETFL, arg);
        ASSERT_TEST (rv >= 0, "Could not set socket FD options",
            err_set_fcntl, -1);

        /* Trying to connect with timeout */
        rv = connect(*fd, p->ai_addr, p->ai_addrlen);
        if (rv < 0) {
            if (errno != EINPROGRESS) {
                DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                        "[ll_io_eth] Error connecting is not EINPROGRESS: %d - %s\n", errno, strerror(errno));
                sleep (LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT);
                close (*fd);
                *fd = -1;
                continue;
            }

            DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE,
                    "[ll_io_eth] EINPROGRESS in connect() - selecting\n");
            tv.tv_sec = secs_before_reconnect;
            tv.tv_usec = 0;
            FD_ZERO (&myset);
            FD_SET (*fd, &myset);

            rv = select (*fd+1, NULL, &myset, NULL, &tv);
            if (rv < 0) {
                DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                        "[ll_io_eth] Error connecting: %d - %s\n", errno, strerror(errno));
                sleep (LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT);
                err = -1;
                goto err_select1;
            }

            if (rv == 0) {
                DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE,
                        "[ll_io_eth] Timeout in connecting\n");
                err = -1;
                goto err_select2;
            }

            /* Check if socket is opened succesfully */
            lon = sizeof(int);
            rv = getsockopt (*fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
            ASSERT_TEST (rv >= 0, "Could not get socket options",
                    err_getsockopt, -1);
            if (valopt) {
                DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                        "[ll_io_eth] Error in delayed connection:%d - %s\n", valopt, strerror(valopt));
                sleep (LLIO_ETH_SECS_OPEN_BEFORE_RECONNECT);
                err = -1;
                goto err_valopt;
            }
        }

        break;
    }

    ASSERT_TEST (p != NULL, "Could not connect", err_connect, -1);

    /* Set sock to blocking-mode again */
    arg = fcntl (*fd, F_GETFL, NULL);
    ASSERT_TEST (arg >= 0, "Could not get socket FD options",
        err_get_fcntl2, -1);
    arg &= (~O_NONBLOCK);
    rv = fcntl (*fd, F_SETFL, arg);
    ASSERT_TEST (rv >= 0, "Could not set socket FD options",
        err_set_fcntl2, -1);

    inet_ntop(p->ai_family, _get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    DBE_DEBUG (DBG_LL_IO | DBG_LVL_INFO,
            "[ll_io_eth] Connection established with endpoint: %s\n", s);

    freeaddrinfo(servinfo); /* all done with this structure */

    return err;

err_set_fcntl2:
err_get_fcntl2:
err_connect:
err_valopt:
err_getsockopt:
err_select2:
err_select1:
err_set_fcntl:
err_get_fcntl:
err_setsockopt:
    close (*fd);
    *fd = -1;
err_getaddrinfo:
err_unsup:
    return err;
}

/* get sockaddr, IPv4 or IPv6: */
static void *_get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

static ssize_t _eth_sendall (int fd, uint8_t *buf, size_t len)
{
    size_t total = 0;        /* how many bytes we've sent */
    size_t bytesleft = len;  /* how many we have left to send */
    ssize_t n;

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Sending %zu bytes\n", len);

    while (total < len) {
        n = send (fd, (char *) buf+total, bytesleft, 0);
        DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Sent %zd bytes\n", n);

        /* On error, don't try to recover, just inform it to the caller*/
        if (n == -1) {
            return -1;
        }

        total += n;
        bytesleft -= n;
    }

    return total; /* return number actually sent here */
}

static ssize_t _eth_recvall (int fd, uint8_t *buf, size_t len)
{
    size_t total = 0;        /* how many bytes we've recv */
    size_t bytesleft = len; /* how many we have left to recv */
    ssize_t n;
    fd_set myset;
    struct timeval tv;
    int rv = 0;

    DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Receiving %zu bytes\n", len);

    while (total < len) {
        tv.tv_sec = LLIO_ETH_SECS_BEFORE_RECV_TIMEOUT;
        tv.tv_usec = 0;
        FD_ZERO (&myset);
        FD_SET (fd, &myset);

        rv = select (fd+1, &myset, NULL, NULL, &tv);
        if (rv < 0) {
           DBE_DEBUG (DBG_LL_IO | DBG_LVL_FATAL,
                   "[ll_io_eth] Error on recv select (): %d - %s\n", errno, strerror(errno));
           return -1;
        }

        if (rv == 0) {
           DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE,
                   "[ll_io_eth] Timeout on recv (). Aborting...\n");
           return -1;
        }

        n = recv (fd, (char *) buf+total, bytesleft, 0);
        DBE_DEBUG (DBG_LL_IO | DBG_LVL_TRACE, "[ll_io_eth] Received %zd bytes\n", n);

        /* On error, don't try to recover, just inform it to the caller*/
        if (n == -1) {
            return -1;
        }

        /* Disconnected endpoint */
        if (n == 0) {
            return -1;
        }

        total += n;
        bytesleft -= n;
    }

    return total; /* return actual number of bytes sent here */
}

const llio_ops_t llio_ops_eth = {
    .name           = "ETH",            /* Operations name */
    .open           = eth_open,         /* Open device */
    .release        = eth_release,      /* Release device */
    .read_16        = eth_read_16,      /* Read 16-bit data */
    .read_32        = eth_read_32,      /* Read 32-bit data */
    .read_64        = eth_read_64,      /* Read 64-bit data */
    .write_16       = eth_write_16,     /* Write 16-bit data */
    .write_32       = eth_write_32,     /* Write 32-bit data */
    .write_64       = eth_write_64,     /* Write 64-bit data */
    .read_block     = eth_read_block,   /* Read arbitrary block size data,
                                           parameter size in bytes */
    .write_block    = eth_write_block,  /* Write arbitrary block size data,
                                           parameter size in bytes */
    .read_dma       = NULL,             /* Read arbitrary block size data via DMA,
                                           parameter size in bytes */
    .write_dma      = NULL,             /* Write arbitrary block size data via DMA,
                                           parameter size in bytes */
    /*.read_info      = pcie_read_info */   /* Read device information data */
};
