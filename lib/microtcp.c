/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "microtcp.h"
#include "../utils/crc32.h"

microtcp_sock_t
microtcp_socket (int domain, int type, int protocol)
{
  /* Your code here */
}

int
microtcp_bind (microtcp_sock_t *socket, const struct sockaddr *address,
               socklen_t address_len)
{
  /* Your code here */
}

int
microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address,
                  socklen_t address_len)
{
  /* Your code here */
}

int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
                 socklen_t address_len)
{
  /* Your code here */
}

int
microtcp_shutdown (microtcp_sock_t *socket, int how)
{
  /* Your code here */
}

ssize_t
microtcp_send (microtcp_sock_t *socket, const void *buffer, size_t length,
               int flags)
{
  /* Your code here */
}

ssize_t
microtcp_recv (microtcp_sock_t *socket, void *buffer, size_t length, int flags)
{
  /* Your code here */
}
