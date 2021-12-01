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

microtcp_sock_t
microtcp_socket (int domain, int type, int protocol)
{

 	microtcp_sock_t s1;

	if ( ( s1.sd = socket(domain ,type,protocol ) ) == -1){
		perror("opening TCP listening socket\n");
		s1.state = INVALID;
		return s1;
	}

	s1.state = UNKNOWN;
	s1.init_win_size = MICROTCP_WIN_SIZE;
	s1.curr_win_size = MICROTCP_WIN_SIZE;
	s1.ssthresh = MICROTCP_INIT_SSTHRESH;
	s1.cwnd = MICROTCP_INIT_CWND;

	return s1;

}

int
microtcp_bind (microtcp_sock_t *socket, const struct sockaddr *address,
               socklen_t address_len)
{
	int b;
	if(b=bind(socket->sd,address,address_len)==-1){
		perror("TCP bind\n");
		socket->state=INVALID;
	}else{
		socket->state=LISTEN;
	}
	return b;
}

int
microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address,
                  socklen_t address_len)
{
        microtcp_header_t send;
        struct sockaddr *restrict adres=(struct sockaddr* restrict)address;
        microtcp_header_t *receive=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
        socket->recvbuf=(uint8_t*)malloc(MICROTCP_RECVBUF_LEN*sizeof(uint8_t)); 
        //initializing the header (to be sent to client)of microtcp to start the 3-way handshake
        send.seq_number=htonl(rand());
        send.ack_number=0;
        send.control=htons(SYN);
        send.window=0;
        send.data_len=0;
        send.future_use0=0;
        send.future_use1=0;
        send.future_use2=0;
        send.checksum=0;
        
        for(int i=0;i<MICROTCP_RECVBUF_LEN;i++){
                socket->recvbuf[i]=0;
        }
        socket->recvbuf[8]=send.control;

        if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)==-1){
                perror("Send first packet of 3-way handshake error:");
                socket->state=INVALID;
                return -1;
        }
        
        if(recvfrom(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,adres,&address_len)==-1){
                perror("Receive from server error:");
                socket->state=INVALID;
                return -1;
        }
        if(
	
       	return 1;
}

int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
                 socklen_t address_len)
{
	microtcp_header_t send;
	microtcp_header_t receive;

	struct sockaddr* restrict addr=address;
	socklen_t* restrict addr_len=&address_len;
	int flags=0;
	int rec;
	socket->recvbuf=(uint8_t*)malloc(MICROTCP_RECVBUF_LEN*sizeof(uint8_t));
	if(rec=recvfrom(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,flags,addr,addr_len)==-1){
		perror("TCP Accept first receival\n");
		socket->state=INVALID;
		return -1;
	}
	if(socket->recvbuf[8]==htons((uint8_t)(0*ACK+1*SYN+0*FIN))){
		printf("Client did not send SYN for the handshake\n");
		return -1;
	}
	int r=rand()%999+1;
	
	

	receive.seq_number=htonl((uint32_t)r);
        receive.ack_number=
        receive.control=htons(1*ACK+0*SYN+0*FIN);
        receive.window=0;
        receive.data_len=0;
        receive.future_use0=0;
        receive.future_use1=0;
        receive.future_use2=0;
        receive.checksum=0;

	return 0;
}

int

microtcp_shutdown (microtcp_sock_t *socket, int how)
{

 //*** client sends FIN pocket ***//
 //*** server receivs FIN - sends ACK - state = CLOSING_BY_PEER ***//
 //*** client receivs ACK - state = CLOSING_BY_HOST ***//
 //*** server sends FIN pocket ***//
 //*** client receivs FIN pocket - sends ACK pocket - state = CLOSED ***//
 //*** server receivs ACK pocket - state = CLOSED ***//

struct sockaddr *address;
socklen_t address_len;

ssize_t tmp_recvfrom;
uint8_t buffer[MICROTCP_RECVBUF_LEN];
microtcp_header_t *recv_head_pack=(microtcp_header_t *)malloc(sizeof(microtcp_header_t));
microtcp_header_t send_head_pack;
microtcp_header_t check_head_pack;
uint32_t	checkSum1;
uint32_t	TMPcheckSum;
int i;

	if(socket->state==CLOSING_BY_PEER){

		send_head_pack.seq_number=0;
		send_head_pack.ack_number=htonl(socket->seq_number+1);
		send_head_pack.control=htons(ACK);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;


		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&send_head_pack,sizeof(send_head_pack));
		TMPcheckSum=crc32(buffer,sizeof(buffer));
		send_head_pack.checksum=htonl(TMPcheckSum);

		  if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
			socket->state=INVALID;
			perror("microTCP Shutdown connection error");
			return socket;
		}



		srand(time(NULL));
		send_head_pack.seq_number=rand()%1000+1;
		send_head_pack.ack_number=htonl(socket->seq_number+1);
		send_head_pack.control=htons(FIN_ACK);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;

		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&send_head_pack,sizeof(send_head_pack));
		TMPcheckSum=crc32(buffer,sizeof(buffer));
		send_head_pack.checksum=htonl(TMPcheckSum);
		  if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
			socket->state=INVALID;
			perror("microTCP Shutdown connection error - While 3rd packet send");
			return socket;
		}


		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,address_len);
		if(tmp_recvfrom == -1){

			perror("microTCP shutdown connection fail");
			exit(EXIT_FAILURE);
		}

		TMPcheckSum=ntohl(recv_head_pack->checksum);
		check_head_pack.seq_number = recv_head_pack->seq_number;
		check_head_pack.ack_number = recv_head_pack->ack_number;
		check_head_pack.control = recv_head_pack->control;
		check_head_pack.window = recv_head_pack->window;
		check_head_pack.data_len = 0;
		check_head_pack.future_use0 = 0;
		check_head_pack.future_use1 = 0;
		check_head_pack.future_use2 = 0;
		check_head_pack.checksum = 0;

		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&check_head_pack,sizeof(microtcp_header_t));
		checkSum1=crc32(buffer,sizeof(buffer));

		if(checkSum1!=TMPcheckSum){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return socket;
		}
		recv_head_pack->control=ntohs(recv_head_pack->control);
		if(recv_head_pack->control!=ACK){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return socket;
		}

		if(ntohl(recv_head_pack->seq_number)!=ntohl(send_head_pack.ack_number) ||
			ntohl(recv_head_pack->ack_number)!=ntohl(send_head_pack.seq_number)+1){
				socket->state=INVALID;
				perror("microTCP shutdown connection error");
				return socket;
			}
	}
	else{

		send_head_pack.seq_number=htonl(socket->seq_number+1);
		send_head_pack.ack_number=0;
		send_head_pack.control=htons(FIN_ACK);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;


		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&send_head_pack,sizeof(send_head_pack));
		TMPcheckSum=crc32(buffer,sizeof(buffer));
		send_head_pack.checksum=htonl(TMPcheckSum);
		  if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len) <0){
			socket->state=INVALID;
			perror("microTCP Shutdown connection error");
			return socket;
		}


		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,address_len);
		if(tmp_recvfrom == -1){

			perror("microTCP shutdown connection fail (2nd packet recv)");
			exit(EXIT_FAILURE);
		}


		TMPcheckSum=ntohl(recv_head_pack->checksum);
		check_head_pack.seq_number = recv_head_pack->seq_number;
		check_head_pack.ack_number = recv_head_pack->ack_number;
		check_head_pack.control = recv_head_pack->control;
		check_head_pack.window = recv_head_pack->window;
		check_head_pack.data_len = 0;
		check_head_pack.future_use0 = 0;
		check_head_pack.future_use1 = 0;
		check_head_pack.future_use2 = 0;
		check_head_pack.checksum = 0;
		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&check_head_pack,sizeof(check_head_pack));
		checkSum1=crc32(buffer,sizeof(buffer));
		if(checkSum1!=TMPcheckSum){
			socket->state=INVALID;
			perror("microTCP shutdown connection error - 2nd packet - error checksum");
			return socket;
		}


		recv_head_pack->control=ntohs(recv_head_pack->control);
		if(recv_head_pack->control!=ACK){
			socket->state=INVALID;
			perror("microTCP shutdown connection error - 2nd packet is not ACK");
			return socket;
		}

		recv_head_pack->ack_number=ntohl(recv_head_pack->ack_number);
		send_head_pack.seq_number=ntohl(send_head_pack.seq_number);
		if(recv_head_pack->ack_number!=send_head_pack.seq_number+1){
				perror("microTCP shutdown connection error - 2nd packet -error ack numbers");
				printf("I was waiting for \n");
				//printf("%" PRIu32 "\n",send_head_pack.seq_number+1);
				printf("and I got \n");
				//printf("%" PRIu32 "\n",recv_head_pack->ack_number);
				socket->state = INVALID;
				return socket;
			}


		socket->state = CLOSING_BY_HOST;


		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,address_len);
		if(tmp_recvfrom == -1){

			perror("microTCP shutdown connection fail (3rd packet recv)");
			exit(EXIT_FAILURE);
		}


		TMPcheckSum=ntohl(recv_head_pack->checksum);
		check_head_pack.seq_number = recv_head_pack->seq_number;
		check_head_pack.ack_number = recv_head_pack->ack_number;
		check_head_pack.control = recv_head_pack->control;
		check_head_pack.window = recv_head_pack->window;
		check_head_pack.data_len = 0;
		check_head_pack.future_use0 = 0;
		check_head_pack.future_use1 = 0;
		check_head_pack.future_use2 = 0;
		check_head_pack.checksum = 0;
		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&check_head_pack,sizeof(check_head_pack));
		checkSum1=crc32(buffer,sizeof(buffer));
		if(checkSum1!=TMPcheckSum){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return socket;
		}


		recv_head_pack->control=ntohs(recv_head_pack->control);
		if(recv_head_pack->control!=FIN_ACK){
			socket->state=INVALID;
			perror("microTCP shutdown connection error - 3rd packet is not FIN ACK");
			return socket;
		}
		recv_head_pack->seq_number=ntohl(recv_head_pack->seq_number);
		recv_head_pack->ack_number=ntohl(recv_head_pack->ack_number);


		send_head_pack.seq_number=htonl(recv_head_pack->ack_number);
		send_head_pack.ack_number=htonl(recv_head_pack->seq_number+1);
		send_head_pack.control=htons(ACK);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;


		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
				buffer[i]=0;
		memcpy(buffer,&send_head_pack,sizeof(send_head_pack));
		TMPcheckSum=crc32(buffer,sizeof(buffer));
		send_head_pack.checksum=htonl(TMPcheckSum);
		 if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len) <0){
			socket->state=INVALID;
			perror("microTCP Shutdown connection error - While 4th packet send");
		}
	}


 microtcp_sock_t s2;


 if(how == 0){
	 s2.state = CLOSING_BY_PEER;
 }
 else if(how == 1){
	s2.state = CLOSING_BY_HOST;
 }

 free(socket);
 s2.state = CLOSED;





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


