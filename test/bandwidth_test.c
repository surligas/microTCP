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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* CHANGES */
#include <limits.h>
/* END OF CHANGES */

#include "../lib/microtcp.h"

#define CHUNK_SIZE 4096

static inline void
print_statistics (ssize_t received, struct timespec start, struct timespec end)
{
  double elapsed = end.tv_sec - start.tv_sec
      + (end.tv_nsec - start.tv_nsec) * 1e-9;
  double megabytes = received / (1024.0 * 1024.0);
  printf ("Data received: %f MB\n", megabytes);
  printf ("Transfer time: %f seconds\n", elapsed);
  printf ("Throughput achieved: %f MB/s\n", megabytes / elapsed);
}

int
server_tcp (uint16_t listen_port, const char *file)
{
  uint8_t *buffer;
  FILE *fp;
  int sock;
  int accepted;
  int received;
  ssize_t written;
  ssize_t total_bytes = 0;
  socklen_t client_addr_len;

  struct sockaddr_in sin;
  struct sockaddr client_addr;
  struct timespec start_time;
  struct timespec end_time;

  /* Allocate memory for the application receive buffer */
  buffer = (uint8_t *) malloc (CHUNK_SIZE);
  if (!buffer) {
    perror ("Allocate application receive buffer");
    return -EXIT_FAILURE;
  }

  /* Open the file for writing the data from the network */
  fp = fopen (file, "w");
  if (!fp) {
    perror ("Open file for writing");
    free (buffer);
    return -EXIT_FAILURE;
  }

  if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    perror ("Opening TCP socket");
    free (buffer);
    fclose (fp);
    return -EXIT_FAILURE;
  }

  memset (&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (listen_port);
  /* Bind to all available network interfaces */
  sin.sin_addr.s_addr = INADDR_ANY;

  if (bind (sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) == -1) {
    perror ("TCP bind");
    free (buffer);
    fclose (fp);
    return -EXIT_FAILURE;
  }

  if (listen (sock, 1000) == -1) {
    perror ("TCP listen");
    free (buffer);
    fclose (fp);
    return -EXIT_FAILURE;
  }

  /* Accept a connection from the client */
  client_addr_len = sizeof(struct sockaddr);
  accepted = accept (sock, &client_addr, &client_addr_len);
  if (accepted < 0) {
    perror ("TCP accept");
    free (buffer);
    fclose (fp);
    return -EXIT_FAILURE;
  }

  /*
   * Start processing the received data.
   *
   * Also start measuring time. Not the most accurate measurement, but
   * it is a good starting point.
   *
   * At hy-435 we deal with bandwidth measurements software in a more
   * right and careful way :-)
   */

  clock_gettime (CLOCK_MONOTONIC_RAW, &start_time);
  while ((received = recv (accepted, buffer, CHUNK_SIZE, 0)) > 0) {
    written = fwrite (buffer, sizeof(uint8_t), received, fp);
    total_bytes += received;
    if (written * sizeof(uint8_t) != received) {
      printf ("Failed to write to the file the"
              " amount of data received from the network.\n");
      shutdown (accepted, SHUT_RDWR);
      shutdown (sock, SHUT_RDWR);
      close (accepted);
      close (sock);
      free (buffer);
      fclose (fp);
      return -EXIT_FAILURE;
    }
  }
  clock_gettime (CLOCK_MONOTONIC_RAW, &end_time);
  print_statistics (total_bytes, start_time, end_time);

  shutdown (accepted, SHUT_RDWR);
  shutdown (sock, SHUT_RDWR);
  close (accepted);
  close (sock);
  fclose (fp);
  free (buffer);

  return 0;
}

	int
server_microtcp (uint16_t listen_port, const char *file)
{
	microtcp_sock_t server; //= microtcp_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in sin;
	struct sockaddr client_addr;
	FILE *fp;
	socklen_t client_addr_len;
	struct timespec start_time;
	struct timespec end_time;
	int received;
	ssize_t written;
	ssize_t total_bytes=0;
	int flag;

	/*evala auta */
	char* tempbuf;
	microtcp_header_t header;

	fp=fopen(file,"w");
	if(!fp){
		perror("Open file for writing");
		return -EXIT_FAILURE;
	}

	server = microtcp_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset (&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (listen_port);
	sin.sin_addr.s_addr=INADDR_ANY;
	if(microtcp_bind(&server,(struct sockaddr*) &sin,sizeof(struct sockaddr_in))==-1){
		perror("TCP bind");

	}
	client_addr_len = sizeof(struct sockaddr);
	if(microtcp_accept(&server,&client_addr,client_addr_len)==-1){
		perror("TCP Accept");
		fclose(fp);
		return -EXIT_FAILURE;
	}

	clock_gettime (CLOCK_MONOTONIC_RAW, &start_time);
	while ((received = microtcp_recv(&server, server.recvbuf, CHUNK_SIZE + sizeof(microtcp_header_t), 0,&client_addr,client_addr_len)) > 0) {
 		written = fwrite (&(server.recvbuf)[sizeof(microtcp_header_t)-1], sizeof(uint8_t), received - sizeof(microtcp_header_t), fp);
		total_bytes += received;
 		if (written * sizeof(uint8_t) != (received-sizeof(microtcp_header_t))) {
   			printf ("Failed to write to the file the"
           		" amount of data received from the network.\n");
      			//shutdown (accepted, SHUT_RDWR)
      			//shutdown (sock, SHUT_RDWR);
      			close (server.sd);
      			free (server.recvbuf);
      			fclose (fp);
      			return -EXIT_FAILURE;
    		}
  	}
	flag=0;
	/*allaksa auto, uparxei ena flag to opoio elegxei an to teleutaio mhnuma einai mege8ous
	  oso enas header kai an to control einai FIN ACK (douleuei apo oso exw dei, grafei pia
	  sto arxeio apla teleiwnei me xarakthres @^@^@^@^@^@^ gia kapoio logo isws logo shutdown) */
	while ((flag==0)&&(received = microtcp_recv(&server, server.recvbuf, CHUNK_SIZE + sizeof(microtcp_header_t), 0,&client_addr,client_addr_len)) > 0) {
		tempbuf=(char*)malloc(CHUNK_SIZE);
                memcpy(tempbuf,&(server.recvbuf)[sizeof(microtcp_header_t)-1],received);
		if(server.recvbuf[sizeof(microtcp_header_t)]==NULL){
			memcpy(&header,server.recvbuf,sizeof(microtcp_header_t));
			if(ntohs(header.control)==(FIN|ACK)){
				flag=1;		//FIN ACK sent
			}
		}else{
 			written = fwrite (tempbuf, sizeof(uint8_t), received, fp);
			total_bytes += received;
 			if (written != received) {
   				printf ("Failed to write to the file the"
        	   		" amount of data received from the network.\n");
      				//shutdown (accepted, SHUT_RDWR);
      				//shutdown (sock, SHUT_RDWR);
      				close (server.sd);
      				free (server.recvbuf);
      				fclose (fp);
      				return -EXIT_FAILURE;
    			}
  		}
	}
	clock_gettime (CLOCK_MONOTONIC_RAW, &end_time);
  	print_statistics (total_bytes, start_time, end_time);
	fclose(fp);
	return 0;
}

int
client_tcp (const char *serverip, uint16_t server_port, const char *file)
{
  uint8_t *buffer;
  int sock;
  socklen_t client_addr_len;
  FILE *fp;
  size_t read_items = 0;
  ssize_t data_sent;

  struct sockaddr *client_addr;

  /* Allocate memory for the application receive buffer */
  buffer = (uint8_t *) malloc (CHUNK_SIZE);
  if (!buffer) {
    perror ("Allocate application receive buffer");
    return -EXIT_FAILURE;
  }


  /* Open the file for writing the data from the network */
  fp = fopen (file, "r");
  if (!fp) {
    perror ("Open file for reading");
    free (buffer);
    return -EXIT_FAILURE;
  }
  if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    perror ("Opening TCP socket");
    free (buffer);
    fclose (fp);
    return -EXIT_FAILURE;
  }

  struct sockaddr_in sin;
  memset (&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  /*Port that server listens at */
  sin.sin_port = htons (server_port);
  /* The server's IP*/
  sin.sin_addr.s_addr = inet_addr (serverip);

  if (connect (sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in))
      == -1) {
    perror ("TCP connect");
    exit (EXIT_FAILURE);
  }

  printf ("Starting sending data...\n");
  /* Start sending the data */
  while (!feof (fp)) {
    read_items = fread (buffer, sizeof(uint8_t), CHUNK_SIZE, fp);
    if (read_items < 1) {
      perror ("Failed read from file");
      shutdown (sock, SHUT_RDWR);
      close (sock);
      free (buffer);
      fclose (fp);
      return -EXIT_FAILURE;
    }

    data_sent = send (sock, buffer, read_items * sizeof(uint8_t), 0);
    if (data_sent != read_items * sizeof(uint8_t)) {
      printf ("Failed to send the"
              " amount of data read from the file.\n");
      shutdown (sock, SHUT_RDWR);
      close (sock);
      free (buffer);
      fclose (fp);
      return -EXIT_FAILURE;
    }

  }

  printf ("Data sent. Terminating...\n");
  shutdown (sock, SHUT_RDWR);
  close (sock);
  free (buffer);
  fclose (fp);
  return 0;
}

int client_microtcp (const char *serverip, uint16_t server_port, const char *file)
{
	microtcp_sock_t client = microtcp_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	FILE *fp;
	struct sockaddr_in* server_address;
	struct sockaddr_in sin;
	size_t read_items=0;
	ssize_t data_sent;
	microtcp_header_t header;
	int checksum;
	uint8_t* buffer;

	fp=fopen(file,"r");
	
	if(!fp){
		perror("Open file for reading");
		return -EXIT_FAILURE;
	}
	memset(&sin,0,sizeof(struct sockaddr_in));
	sin.sin_family=AF_INET;
	sin.sin_port=htons(server_port);
	sin.sin_addr.s_addr=inet_addr(serverip);
	
	if(microtcp_connect(&client,(struct sockaddr*)&sin,sizeof(struct sockaddr_in))==-1){
		return -EXIT_FAILURE;
	}
	printf("Starting sending data...\n");


	buffer=(uint8_t*)malloc(CHUNK_SIZE);
	/* Start sending the data */
  	while (!feof (fp)) {
    		read_items = fread (buffer, sizeof(uint8_t), CHUNK_SIZE, fp);
    		if (read_items < 1) {
    			perror ("Failed read from file");
      			//shutdown (sock, SHUT_RDWR);
      			close (client.sd);
      			free (buffer);
      			fclose (fp);
      			return -EXIT_FAILURE;
   		}
		
		checksum=0;		//calculate checksum


		//Initialising header
		header=initialize(client.seq_number,client.ack_number,ACK,0,0,0,client.curr_win_size,sizeof(microtcp_header_t) + CHUNK_SIZE, sin.sin_family, sin.sin_port, sin.sin_addr.s_addr, checksum);	
	
		//Making buffer
		memcpy(client.recvbuf,&header,sizeof(microtcp_header_t));
		memcpy(&(client.recvbuf)[sizeof(microtcp_header_t)-1],buffer,CHUNK_SIZE);

    		data_sent = microtcp_send (&client, client.recvbuf, sizeof(microtcp_header_t)+read_items * sizeof(uint8_t),0, (struct sockaddr*)&sin,sizeof(struct sockaddr));
    		if ((data_sent-sizeof(microtcp_header_t)) != read_items * sizeof(uint8_t)) {
      			printf ("Failed to send the"
        	      " amount of data read from the file.\n");
      			//shutdown (sock, SHUT_RDWR);
      			close (client.sd);
      			free (buffer);
      			fclose (fp);
     			return -EXIT_FAILURE;
    		}

  	}

  	printf ("Data sent. Terminating...\n");

	/*pros8esa allh mia send sto telos h opoia stelnei mono ena header FIN ACK, auth eleipe kai
	  paremene anoixtos o server */

	//Initialising header
        header=initialize(client.seq_number,client.ack_number,ACK,0,0,FIN,client.curr_win_size,sizeof(microtcp_header_t) + CHUNK_SIZE, 0, 0, 0, checksum);
        //Making buffer
        memcpy(client.recvbuf,&header,sizeof(microtcp_header_t));
	
	data_sent=microtcp_send(&client,client.recvbuf,sizeof(microtcp_header_t),0, (struct sockaddr*)&sin,sizeof(struct sockaddr));
	
}

int
main (int argc, char **argv)
{
  int opt;
  int port;
  int exit_code = 0;
  char *filestr = NULL;
  char *ipstr = NULL;
  uint8_t is_server = 0;
  uint8_t use_microtcp = 0;
  /* CHANGES */
  char* p;
  char buf[PATH_MAX];
  /* END OF CHANGES */  

  /* A very easy way to parse command line arguments */
  while ((opt = getopt (argc, argv, "hsmf:p:a:")) != -1) {
    switch (opt)
      {
      /* If -s is set, program runs on server mode */
      case 's':
        is_server = 1;
        break;
        /* if -m is set the program should use the microTCP implementation */
      case 'm':
        use_microtcp = 1;
        break;
      case 'f':
        filestr = strdup (optarg);
        /* A few checks will be nice here...*/
        /* Convert the given file to absolute path */
	p=realpath(filestr,buf);
	if(p){
		printf("%s\n",p);
	}

        break;
      case 'p':
        port = atoi (optarg);
        /* To check or not to check? */
        break;
      case 'a':
        ipstr = strdup (optarg);
        break;

      default:
        printf (
            "Usage: bandwidth_test [-s] [-m] -p port -f file"
            "Options:\n"
            "   -s                  If set, the program runs as server. Otherwise as client.\n"
            "   -m                  If set, the program uses the microTCP implementation. Otherwise the normal TCP.\n"
            "   -f <string>         If -s is set the -f option specifies the filename of the file that will be saved.\n"
            "                       If not, is the source file at the client side that will be sent to the server.\n"
            "   -p <int>            The listening port of the server\n"
            "   -a <string>         The IP address of the server. This option is ignored if the tool runs in server mode.\n"
            "   -h                  prints this help\n");
        exit (EXIT_FAILURE);
      }
  }

  /*
   * TODO: Some error checking here???
   */

  /*
   * Depending the use arguments execute the appropriate functions
   */
  if (is_server) {

    if (use_microtcp) {
      exit_code = server_microtcp (port, p);
    }
    else {
      exit_code = server_tcp (port, p);
    }
  }
  else {
    if (use_microtcp) {
      exit_code = client_microtcp (ipstr, port, p);
    }
    else {
      exit_code = client_tcp (ipstr, port, p);
    }
  }

  free(filestr);
  free (p);
  free (ipstr);
  return exit_code;
}

