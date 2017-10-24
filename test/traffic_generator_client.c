/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2017  Manolis Surligas <surligas@gmail.com>
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../lib/microtcp.h"
#include "../utils/log.h"

static char running = 1;

static void
sig_handler(int signal)
{
  if(signal == SIGINT) {
    LOG_INFO("Stopping traffic generator client...");
    running = 0;
  }
}

int
main(int argc, char **argv) {
  uint16_t port;

  /*
   * Register a signal handler so we can terminate the client with
   * Ctrl+C
   */
  signal(SIGINT, sig_handler);

  LOG_INFO("Start receiving traffic from port %u", port);
  /*TODO: Connect using microtcp_connect() */
  while(running) {
    /* TODO: Measure time */
    /* TODO: Receive using microtcp_recv()*/
    /* TODO: Measure time */
    /* TODO: Do other stuff... */
  }

  /* Ctrl+C pressed! Store properly time measurements for plotting */
}

