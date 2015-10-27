/*
 * microtcp.c
 *
 *  Created on: Oct 25, 2015
 *      Author: surligas
 */

#include "microtcp.h"
#include "../utils/crc32.h"

microtcp_sock_t
microtcp_socket(int domain, int type, int protocol)
{
	/* Your code here */
}

int
microtcp_bind(microtcp_sock_t socket, const struct sockaddr *address,
              socklen_t address_len)
{
	/* Your code here */
}

int
microtcp_connect(microtcp_sock_t socket, const struct sockaddr *address,
                 socklen_t address_len)
{
	/* Your code here */
}

int
microtcp_shutdown(microtcp_sock_t socket, int how)
{
	/* Your code here */
}
