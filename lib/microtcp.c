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
#include <time.h>

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
	s1.recvbuf = (uint8_t*)malloc(MICROTCP_RECVBUF_LEN*sizeof(uint8_t));

	return s1;

}

int
microtcp_bind (microtcp_sock_t *socket, const struct sockaddr *address,
               socklen_t address_len)
{
	int b;
	if(socket->state==INVALID) return -1;
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
        
        if(socket->state!=LISTEN)
                return -1;
        int success_counter=0;
        microtcp_header_t send;
        struct sockaddr* adres=(struct sockaddr*)address;
        microtcp_header_t *receive=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
        socket->recvbuf=(uint8_t*)malloc(MICROTCP_RECVBUF_LEN*sizeof(uint8_t)); 
        //initializing the header (to be sent to client)of microtcp to start the 3-way handshake
        srand(time(0));
        send.seq_number=htonl(rand()%999+1);
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
	}else{
            success_counter++;
    }
		
        
        if(recvfrom(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,adres,&address_len)==-1){
                perror("Receive from server error:");
                socket->state=INVALID;
        }else{
		        receive=(microtcp_header_t*)socket->recvbuf;
		//check if receive was: a SYNACK,acknum=send.seqnumber+1
                if((receive->control==htons((uint16_t)(1*ACK+1*SYN+0*FIN)))&&(receive->ack_number==send.seq_number+1)){
                                send.control=htons((uint16_t)(1*ACK + 0*SYN+0*FIN));
                                send.seq_number=send.seq_number+1;
                                send.ack_number=receive->seq_number+1;
                                success_counter++;
                }else{
                        socket->state=INVALID;
                        perror("Receiving SYN_ACK signal error");
                }
        }
        if(success_counter!=2){
                printf("Error at the SYN-ACK signal transmission\n");
                socket->state=INVALID;
                return -1;
        }
        memcpy(socket->recvbuf,&send,sizeof(microtcp_header_t));
        if(sendto(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,address_len)==-1){
                perror("TCP error sending ACK to server");
        }else{
                success_counter++;

        }
        if(success_counter!=3){
                socket->state=INVALID;
                return -1;
        }
        socket->state=ESTABLISHED;
        return socket->sd;
                   
	
}

int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
                 socklen_t address_len)
{

	microtcp_header_t *recv_header;
	microtcp_header_t *send_header;
	int isSYNReceived=0;
	int isSYNACKSent=0;
	int isACKReceived=0;
	
	recv_header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
	send_header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));

    srand(time(0));
	send_header->seq_number=rand()%999+1;
	send_header->ack_number=0;
	send_header->control=htons((uint16_t)(0*ACK + 0*SYN + 0*FIN));
	send_header->window=0;
	send_header->data_len=0;
	send_header->future_use0=0;
	send_header->future_use1=0;
	send_header->future_use2=0;
	send_header->checksum=0;

	/* HERE WE RECEIVE THE SYN SIGNAL FROM THE CLIENT */
	
	/*if recvfrom return -1 error */
	if(recvfrom(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,&address_len)==-1){
		perror("Error receiving SYN from client\n");			
	}else{
		recv_header=(microtcp_header_t*)socket->recvbuf;
		/*if control is not SYN error */
		if(recv_header->control!=htons((uint16_t)(0*ACK + 1*SYN + 0*FIN))){
			perror("Connection did not start with SYN from client\n");
		}else{
			/* That means that we received the SYN from the client
			   so we change the send_header */
			send_header->ack_number=recv_header->seq_number+1;
			send_header->control=htons((uint16_t)(1*ACK + 1*SYN + 0*FIN));
			isSYNReceived=1;
		}
	}
	
	/* HERE WE SEND THE SYN,ACK SIGNAL BACK TO THE CLIENT */

	memcpy(socket->recvbuf,send_header,sizeof(microtcp_header_t));
	/*if sendto return -1 error*/
	if(sendto(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,address_len==-1)){
		if(send_header->ack_number!=0)perror("Error sending SYN,ACK to client\n");
		else perror("Error sending ACK=0 to client\n");
	}else{
		isSYNACKSent=1;
	}
	socket->recvbuf[8]=htons((uint8_t)(0*ACK+0*SYN+0*FIN));
	
	/* if SYN was not received or/and SYN,ACK was not sent */
	if(isSYNReceived*isSYNACKSent==0) return -1;
	
	/* HERE WE RECEIVE THE ACK SIGNAL FROM THE CLIENT */

	/*if recvfrom return -1 error */
	if(recvfrom(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,&address_len)==-1){
	        perror("Error receiving ACK from client\n");
        }else{
		/* reinitialising recv_header*/
		free(recv_header);
		recv_header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
		recv_header=(microtcp_header_t*)socket->recvbuf;
        	/*if control is not ACK error */
        	if(recv_header->control!=htons((uint16_t)(1*ACK + 0*SYN + 0*FIN))){
        	        perror("Did not receive ACK from client\n");
	        }else{
			isACKReceived=1;
		}
	}
	/* if ACK was not received */
	if(isACKReceived==0){
            return -1;
	/* if ACK was received, SUCCESS*/
    }else{ 
            socket->state=ESTABLISHED;
            return socket->sd;
    }
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
//uint8_t buffer[MICROTCP_RECVBUF_LEN];
microtcp_header_t *recv_head_pack=(microtcp_header_t *)malloc(sizeof(microtcp_header_t));
microtcp_header_t send_head_pack;
int i;
    
	if(socket->isServer==0){	/* if the client calls shutdown */
	
		/* Client sends FIN,ACK to server */
 		srand(time(NULL));
		send_head_pack.seq_number=rand()%999+1;
		send_head_pack.ack_number=htonl(socket->seq_number+1);
		send_head_pack.control=htons(1*ACK+0*SYN+1*FIN);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;  

		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
			socket->recvbuf[i]=0;
		memcpy(socket->recvbuf,&send_head_pack,sizeof(send_head_pack));
		if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
			socket->state=INVALID;
			perror("microTCP Shutdown connection error");
			return -1;
             	}

		/* Client receives ACK from server */
		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,&address_len);
		if(tmp_recvfrom == -1){

			perror("microTCP shutdown connection fail");
		}


		recv_head_pack->control=htons(recv_head_pack->control);
		if(recv_head_pack->control!=htons(1*ACK+0*SYN+0*FIN)){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return -1;
		}

		if(htonl(recv_head_pack->seq_number)!=htonl(send_head_pack.ack_number) || htonl(recv_head_pack->ack_number)!=htonl(send_head_pack.seq_number)+1){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return -1;
		}
        	socket->state=CLOSING_BY_HOST;
        	
		/* Client receives FIN,ACK from server */
		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,&address_len);
		if(tmp_recvfrom == -1){

			perror("microTCP shutdown connection fail");
		}

		recv_head_pack->control=ntohs(recv_head_pack->control);
		if(recv_head_pack->control!=htons(1*ACK+0*SYN+1*FIN)){
			socket->state=INVALID;
			perror("microTCP shutdown connection error");
			return -1;
		}
        	
		/* Client sends ACK to server */
		send_head_pack.seq_number=htonl(socket->seq_number+1);
        	send_head_pack.ack_number=htonl(recv_head_pack->ack_number+1);
        	send_head_pack.control=htons(1*ACK+0*SYN+0*FIN);
		send_head_pack.window=htons(socket->curr_win_size);
		send_head_pack.data_len=0;
		send_head_pack.future_use0=0;
		send_head_pack.future_use1=0;
		send_head_pack.future_use2=0;
		send_head_pack.checksum=0;

		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
                        socket->recvbuf[i]=0;
                memcpy(socket->recvbuf,&send_head_pack,sizeof(send_head_pack));
                if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
                        socket->state=INVALID;
                        perror("microTCP Shutdown connection error");
                        return -1;
                }
	
		/* Closing client */
		
		free(socket->recvbuf);
		close(socket->sd);
    	
	}else{		/* If the server calls shutdown */

		/* Server receives Fin,ACK from client */
		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,&address_len);
		if(tmp_recvfrom == -1){
			socket->state=INVALID;
			perror("microTCP shutdown connection fail (2nd packet recv)");
			return -1;
		}

		recv_head_pack->control=htons(recv_head_pack->control);
		if(recv_head_pack->control!=htons(1*ACK+0*SYN+1*FIN)){
			socket->state=INVALID;
			perror("microTCP shutdown connection error - 2nd packet is not ACK");
			return -1;
		}
		
		/* Server sends ACK to client */
		send_head_pack.seq_number=0;
                send_head_pack.ack_number=htonl(recv_head_pack->ack_number+1);
                send_head_pack.control=htons(1*ACK+0*SYN+0*FIN);
                send_head_pack.window=htons(socket->curr_win_size);
                send_head_pack.data_len=0;
                send_head_pack.future_use0=0;
                send_head_pack.future_use1=0;
                send_head_pack.future_use2=0;
                send_head_pack.checksum=0;

		for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
                        socket->recvbuf[i]=0;
                memcpy(socket->recvbuf,&send_head_pack,sizeof(send_head_pack));
                if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
                        socket->state=INVALID;
                        perror("microTCP Shutdown connection error");
                        return -1;
                }
		
		socket->state = CLOSING_BY_PEER;

		/* Server sends FIN,ACK to client */
		send_head_pack.seq_number=0;
                send_head_pack.ack_number=htonl(recv_head_pack->ack_number+1);
                send_head_pack.control=htons(1*ACK+0*SYN+1*FIN);
                send_head_pack.window=htons(socket->curr_win_size);
                send_head_pack.data_len=0;
                send_head_pack.future_use0=0;
                send_head_pack.future_use1=0;
                send_head_pack.future_use2=0;
                send_head_pack.checksum=0;

                for(i=0;i<MICROTCP_RECVBUF_LEN;i++)
                        socket->recvbuf[i]=0;
                memcpy(socket->recvbuf,&send_head_pack,sizeof(send_head_pack));
                if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)<0){
                        socket->state=INVALID;
                        perror("microTCP Shutdown connection error");
                        return -1;
                }

		/* Server receives ACK from client */
		tmp_recvfrom=recvfrom(socket->sd,recv_head_pack,sizeof(microtcp_header_t),0,address,&address_len);
		if(tmp_recvfrom == -1){
			socket->state=INVALID;
			perror("microTCP shutdown connection fail (3rd packet recv)");
			return -1;
		}

		recv_head_pack->control=htons(recv_head_pack->control);
		if(recv_head_pack->control!=htons(1*ACK + 0*SYN + 0*FIN)){
			socket->state=INVALID;
			perror("microTCP shutdown connection error - 3rd packet is not FIN ACK");
			return -1;
		}

		/* Server closing */
		free(socket->recvbuf);
		close(socket->sd);
	}
 	socket->state = CLOSED;

	return 1;



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


/* Our own Function(s) */

//
microtcp_header_t initialize(int seq,int ack,int who,int Ack,int Rst,int Syn,int Fin,uint16_t window,uint32_t data_len,uint32_t future_use0,uint32_t future_use1, uint32_t future_use2,uint32_t checksum ){
	//0==sender htons
	//1== receiver ntohs
	microtcp_header_t sock;
	if(who==1){
		sock->seq_number=htonl(seq);
		sock->ack_number=htonl(ack);
		sock->control=htons(Ack|Rst|Syn|Fin);
		sock->window=htons(window);
		sock->data_len=hton(data_len);
		sock->future_use0=htonl(future_use0);
		sock->future_use1=htonl(future_use1);
		sock->future_use2=htonl(future_use2);
		sock->checksum=htonl(checksum);
	}else{
		sock->seq_number=ntohl(seq);
		sock->ack_number=ntohl(ack);
		sock->control=ntohs(Ack|Rst|Syn|Fin);
		sock->window=ntohs(window);
		sock->data_len=ntoh(data_len);
		sock->future_use0=ntohl(future_use0);
		sock->future_use1=ntohl(future_use1);
		sock->future_use2=ntohl(future_use2);
		sock->checksum=ntohl(checksum);	
	}


	return sock;
}

