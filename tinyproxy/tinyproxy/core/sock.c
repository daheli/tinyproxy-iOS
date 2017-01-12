/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1998 Steven Young <sdyoung@miranda.org>
 * Copyright (C) 1999, 2004 Robert James Kaes <rjkaes@users.sourceforge.net>
 * Copyright (C) 2000 Chris Lightfoot <chris@ex-parrot.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Sockets are created and destroyed here. When a new connection comes in from
 * a client, we need to copy the socket and the create a second socket to the
 * remote server the client is trying to connect to. Also, the listening
 * socket is created and destroyed here. Sounds more impressive than it
 * actually is.
 */

#include "main.h"

#include "log.h"
#include "heap.h"
#include "network.h"
#include "sock.h"
#include "text.h"
#include "conf.h"

/*
 * Bind the given socket to the supplied address.  The socket is
 * returned if the bind succeeded.  Otherwise, -1 is returned
 * to indicate an error.
 */
static int
bind_socket (int sockfd, const char *addr, int family)
{
        struct addrinfo hints, *res, *ressave;

        assert (sockfd >= 0);
        assert (addr != NULL && strlen (addr) != 0);

        memset (&hints, 0, sizeof (struct addrinfo));
        hints.ai_family = family;
        hints.ai_socktype = SOCK_STREAM;

        /* The local port it not important */
        if (getaddrinfo (addr, NULL, &hints, &res) != 0)
                return -1;

        ressave = res;

        /* Loop through the addresses and try to bind to each */
        do {
                if (bind (sockfd, res->ai_addr, res->ai_addrlen) == 0)
                        break;  /* success */
        } while ((res = res->ai_next) != NULL);

        freeaddrinfo (ressave);
        if (res == NULL)        /* was not able to bind to any address */
                return -1;

        return sockfd;
}

/*
 * Open a connection to a remote host.  It's been re-written to use
 * the getaddrinfo() library function, which allows for a protocol
 * independent implementation (mostly for IPv4 and IPv6 addresses.)
 */
int opensock (const char *host, int port, const char *bind_to)
{
        int sockfd, n;
        struct addrinfo hints, *res, *ressave;
        char portstr[6];

        assert (host != NULL);
        assert (port > 0);

        memset (&hints, 0, sizeof (struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        snprintf (portstr, sizeof (portstr), "%d", port);

        n = getaddrinfo (host, portstr, &hints, &res);
        if (n != 0) {
                log_message (LOG_ERR,
                             "opensock: Could not retrieve info for %s", host);
                return -1;
        }

        ressave = res;
        do {
                sockfd =
                    socket (res->ai_family, res->ai_socktype, res->ai_protocol);
                if (sockfd < 0)
                        continue;       /* ignore this one */

                /* Bind to the specified address */
                if (bind_to) {
                        if (bind_socket (sockfd, bind_to,
                                         res->ai_family) < 0) {
                                close (sockfd);
                                continue;       /* can't bind, so try again */
                        }
                } else if (config.bind_address) {
                        if (bind_socket (sockfd, config.bind_address,
                                         res->ai_family) < 0) {
                                close (sockfd);
                                continue;       /* can't bind, so try again */
                        }
                }

                if (connect (sockfd, res->ai_addr, res->ai_addrlen) == 0)
                        break;  /* success */

                close (sockfd);
        } while ((res = res->ai_next) != NULL);

        freeaddrinfo (ressave);
        if (res == NULL) {
                log_message (LOG_ERR,
                             "opensock: Could not establish a connection to %s",
                             host);
                return -1;
        }

        return sockfd;
}

/*
 * Set the socket to non blocking -rjkaes
 */
int socket_nonblocking (int sock)
{
        int flags;

        assert (sock >= 0);

        flags = fcntl (sock, F_GETFL, 0);
        return fcntl (sock, F_SETFL, flags | O_NONBLOCK);
}

/*
 * Set the socket to blocking -rjkaes
 */
int socket_blocking (int sock)
{
        int flags;

        assert (sock >= 0);

        flags = fcntl (sock, F_GETFL, 0);
        return fcntl (sock, F_SETFL, flags & ~O_NONBLOCK);
}


/**
 * Try to listen on one socket based on the addrinfo
 * as returned from getaddrinfo.
 *
 * Return the file descriptor upon success, -1 upon error.
 */
static int listen_on_one_socket(struct addrinfo *ad)
{
        int listenfd;
        int ret;
        const int on = 1;
        char numerichost[NI_MAXHOST];
        int flags = NI_NUMERICHOST;

        ret = getnameinfo(ad->ai_addr, ad->ai_addrlen,
                          numerichost, NI_MAXHOST, NULL, 0, flags);
        if (ret != 0) {
                log_message(LOG_ERR, "error calling getnameinfo: %s",
                            gai_strerror(errno));
                return -1;
        }

        log_message(LOG_INFO, "trying to listen on host[%s], family[%d], "
                    "socktype[%d], proto[%d]", numerichost,
                    ad->ai_family, ad->ai_socktype, ad->ai_protocol);

//        listenfd = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd == -1) {
                log_message(LOG_ERR, "socket() failed: %s", strerror(errno));
                return -1;
        }
    
    struct sockaddr_in si;
    si.sin_family = AF_INET;
    si.sin_port = htons(8888);
    si.sin_addr.s_addr = inet_addr("0.0.0.0");
    socklen_t sl = sizeof(si);
        ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (ret != 0) {
                log_message(LOG_ERR,
                            "setsockopt failed to set SO_REUSEADDR: %s",
                            strerror(errno));
                close(listenfd);
                return -1;
        }

//        if (ad->ai_family == AF_INET6) {
//                ret = setsockopt(listenfd, IPPROTO_IPV6, IPV6_V6ONLY, &on,
//                                 sizeof(on));
//                if (ret != 0) {
//                        log_message(LOG_ERR,
//                                    "setsockopt failed to set IPV6_V6ONLY: %s",
//                                    strerror(errno));
//                        close(listenfd);
//                        return -1;
//                }
//        }

    ret = bind(listenfd, (struct sockaddr*)&si, sl);
//        ret = bind(listenfd, ad->ai_addr, ad->ai_addrlen);
        if (ret != 0) {
               log_message(LOG_ERR, "bind failed: %s", strerror (errno));
            
               close(listenfd);
               return -1;
        }

        ret = listen(listenfd, MAXLISTEN);
        if (ret != 0) {
                log_message(LOG_ERR, "listen failed: %s", strerror(errno));
                close(listenfd);
                return -1;
        }

        log_message(LOG_INFO, "listening on fd [%d]", listenfd);

        return listenfd;
}

/*
 * Start listening on a socket. Create a socket with the selected port.
 * If the provided address is NULL, we may listen on multiple sockets,
 * e.g. the wildcard addresse for IPv4 and IPv6, depending on what is
 * supported. If the address is not NULL, we only listen on the first
 * address reported by getaddrinfo that works.
 *
 * Upon success, the listen-fds are added to the listen_fds list
 * and 0 is returned. Upon error,  -1 is returned.
 */
int listen_sock (const char *addr, uint16_t port, vector_t listen_fds)
{
        struct addrinfo hints, *result, *rp;
        char portstr[6];
        int ret = -1;

        assert (port > 0);
        assert (listen_fds != NULL);

        log_message(LOG_INFO, "listen_sock called with addr = '%s'",
                    addr == NULL ? "(NULL)" : addr);

        memset (&hints, 0, sizeof (struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        snprintf (portstr, sizeof (portstr), "%d", port);

        if (getaddrinfo (addr, portstr, &hints, &result) != 0) {
                log_message (LOG_ERR,
                             "Unable to getaddrinfo() because of %s",
                             strerror (errno));
                return -1;
        }

        for (rp = result; rp != NULL; rp = rp->ai_next) {
                int listenfd;

                listenfd = listen_on_one_socket(rp);
                if (listenfd == -1) {
                        continue;
                }

                vector_append (listen_fds, &listenfd, sizeof(int));

                /* success */
                ret = 0;

                if (addr != NULL) {
                        /*
                         * Unless wildcard is requested, only listen
                         * on the first result that works.
                         */
                        break;
                }
        }

        if (ret != 0) {
                log_message (LOG_ERR, "Unable to listen on any address.");
        }

        freeaddrinfo (result);

        return ret;
}

/*
 * Takes a socket descriptor and returns the socket's IP address.
 */
int getsock_ip (int fd, char *ipaddr)
{
        struct sockaddr_storage name;
        socklen_t namelen = sizeof (name);

        assert (fd >= 0);

        if (getsockname (fd, (struct sockaddr *) &name, &namelen) != 0) {
                log_message (LOG_ERR, "getsock_ip: getsockname() error: %s",
                             strerror (errno));
                return -1;
        }

        if (get_ip_string ((struct sockaddr *) &name, ipaddr, IP_LENGTH) ==
            NULL)
                return -1;

        return 0;
}

/*
 * Return the peer's socket information.
 */
int getpeer_information (int fd, char *ipaddr, char *string_addr)
{
        struct sockaddr_storage sa;
        socklen_t salen = sizeof sa;

        assert (fd >= 0);
        assert (ipaddr != NULL);
        assert (string_addr != NULL);

        /* Set the strings to default values */
        ipaddr[0] = '\0';
        strlcpy (string_addr, "[unknown]", HOSTNAME_LENGTH);

        /* Look up the IP address */
        if (getpeername (fd, (struct sockaddr *) &sa, &salen) != 0)
                return -1;

        if (get_ip_string ((struct sockaddr *) &sa, ipaddr, IP_LENGTH) == NULL)
                return -1;

        /* Get the full host name */
        return getnameinfo ((struct sockaddr *) &sa, salen,
                            string_addr, HOSTNAME_LENGTH, NULL, 0, 0);
}
