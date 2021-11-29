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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "microtcp.h"
#include "../utils/crc32.h"
#include <unistd.h>

#define listening_port 8081

microtcp_sock_t
microtcp_socket (int domain, int type, int protocol)
{

 	microtcp_sock_t s1;
	//s1.sd = socket(domain, SOCK_DGRAM, IPPROTO_UDP);

	if ( ( s1.sd = socket(domain,type,protocol) ) == -1){
		perror("opening TCP listening socket\n");
		exit(EXIT_FAILURE);
	}

	s1.state = UKNOWN;
	return s1;

  /* Your code here */
}

int
microtcp_bind (microtcp_sock_t *socket, const struct sockaddr *address,
               socklen_t address_len)
{
	struct sockaddr_in* sin=(struct sockaddr_in*)address;
	printf("PrOgRaS\n");
	//memset(&sin,0,sizeof(struct sockaddr_in));
	 printf("PrOgRaS\n");
	sin=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(listening_port);
	sin->sin_addr.s_addr = htonl( INADDR_ANY);
	 printf("PrOgRaS\n");

	
	if(bind(socket->sd,(struct sockaddr*)sin,address_len)==-1){
		perror("TCP bind\n");
		exit(EXIT_FAILURE);
	}
    	if(listen(socket->sd,0)==-1){
    	        perror("TCP listen\n");
    	        exit(EXIT_FAILURE);
    	}
}

int
microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address,
                  socklen_t address_len)
{
       if(connect(socket->sd,address,address_len)<0){
               perror("TCP Connect\n");
               return -1;
       }
       socket->state=ESTABLISHED;
       socket->seq_number=rand();
       return 1;
}

int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
                 socklen_t address_len)
{
	int sock_accept;
	if(sock_accept=accept(socket->sd,address,&address_len)==-1){
		perror("TCP Accept\n");
		return -1;
	}
	socket->state=ESTABLISHED;
	socket->ack_number=0;		
	return 0;
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

}

ssize_t
microtcp_recv (microtcp_sock_t *socket, void *buffer, size_t length, int flags)
{
	
}


int main(){
	int pid;

	/* Creating sockets of server and client */
	microtcp_sock_t server=microtcp_socket( AF_INET , SOCK_STREAM, IPPROTO_TCP);
	microtcp_sock_t client=microtcp_socket( AF_INET , SOCK_STREAM, IPPROTO_TCP);
	
	/* binding server socket */
	struct sockaddr server_address;
	socklen_t server_address_len=sizeof(struct sockaddr);
	struct sockaddr client_address;
        socklen_t client_address_len=sizeof(struct sockaddr);

	microtcp_bind(&server,&server_address,server_address_len);
	if(pid=fork()){
		printf("GIWRGHS\n");
		microtcp_accept(&server,&server_address,server_address_len);
		sleep(2);	
	}else{
		printf("O TSAKALOS\n");
		//microtcp_connect(&client,&client_address,client_address_len);
		microtcp_connect(&server,&server_address,server_address_len);
		exit(0);
	}
	return 0;
}
