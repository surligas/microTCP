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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <random>
#include <chrono>
#include <thread>

extern "C" {
#include "../lib/microtcp.h"
#include "../utils/log.h"
}

#define BUF_LEN 2048

static bool stop_traffic = false;


void
sig_handler(int signal)
{
  if(signal == SIGINT) {
    LOG_INFO("Stopping traffic generator...");
    stop_traffic = true;
  }
}

int
main (int argc, char **argv)
{
  int                   opt;
  int                   ret;
  int                   port;
  int                   mean_inter;
  microtcp_sock_t       sock;
  struct sockaddr_in    sin;
  struct sockaddr       client_addr;
  socklen_t             client_addr_len;
  struct sockaddr_in    *addr_in;
  char                  ip_addr[INET_ADDRSTRLEN];
  char                  buffer[BUF_LEN];

  /* Create the random generator */
  std::random_device rd;
  std::mt19937 gen(rd());

  /* A very easy way to parse command line arguments */
  while ((opt = getopt (argc, argv, "hp:i:")) != -1) {
    switch (opt)
      {
      case 'p':
        port = atoi (optarg);
        /* To check or not to check? */
        break;
      case 'i':
        /*
         * Set the mean of the poisson distribution for the interarrivals
         * in milliseconds (ms)
         */
        mean_inter = atoi (optarg);
        break;
      default:
        printf (
            "Usage: bandwidth_test -p port -i packet inter-arrival ms"
            "Options:\n"
            "   -p <int>            the port to wait for a peer"
            "   -i <int>            the mean inter-arrival time in milliseconds of the poisson distribution"
            "   -h                  prints this help\n");
        exit (EXIT_FAILURE);
      }
  }
  std::poisson_distribution<int> dpoisson(mean_inter);
  LOG_INFO("Creating traffic generator on port %d", port);
  LOG_INFO("Poisson distribution inter-arrivals with mean %u ms", mean_inter);

  /*
   * Register a signal handler so we can terminate the generator with
   * Ctrl+C
   */
  signal(SIGINT, sig_handler);

  /* Create a microtcp socket */
  sock = microtcp_socket (AF_INET, 0, 0);
 /* TODO: some error checking here ??? */

  memset (&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (port);
  /* Bind to all available network interfaces */
  sin.sin_addr.s_addr = INADDR_ANY;

  if (microtcp_bind (&sock, (struct sockaddr *) &sin,
                     sizeof(struct sockaddr_in)) == -1) {
    LOG_ERROR("Failed to bind");
    return -EXIT_FAILURE;
  }

  /*
   * Normally, using the original TCP, we would have to set the socket
   * in listening mode with listen(). MicroTCP does not provide such function
   * so we proceed using the equivalent TCP accept()
   */

  /* Block waiting for a connection */
  client_addr_len = sizeof(struct sockaddr);
  ret = microtcp_accept(&sock, &client_addr, client_addr_len);
  if(ret != 0) {
    LOG_ERROR("Failed to accept connection");
    return -EXIT_FAILURE;
  }

  addr_in = (struct sockaddr_in *) &client_addr;
  inet_ntop(AF_INET, &(addr_in->sin_addr), ip_addr, INET_ADDRSTRLEN);
  LOG_INFO("Peer %s connected.", ip_addr);
  std::this_thread::sleep_for (std::chrono::seconds(1));
  LOG_INFO("Start generating traffic...");

  while(stop_traffic == false) {
    std::this_thread::sleep_for(std::chrono::milliseconds(dpoisson(gen)));
    microtcp_send(&sock, buffer, BUF_LEN, 0);
  }

  LOG_INFO("Going to terminate microtcp connection...");

  /* SHUT_RDWR can be omitted internally */
  microtcp_shutdown(&sock, SHUT_RDWR);

}
