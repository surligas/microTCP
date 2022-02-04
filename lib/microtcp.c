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

/* our own functions*/
microtcp_header_t initialize(int seq,int ack,int Ack,int Rst,int Syn,int Fin,uint16_t window,uint32_t data_len,uint32_t future_use0,uint32_t future_use1, uint32_t future_use2,uint32_t checksum );

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
    s1.init_win_size = MICROTCP_INIT_CWND;
    s1.curr_win_size = MICROTCP_INIT_CWND;
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

    printf("Connecting before first send...\n");
    int success_counter=0;
    microtcp_header_t send;
    struct sockaddr* adres=(struct sockaddr*)address;
    microtcp_header_t *receive=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
    socket->recvbuf=(uint8_t*)malloc(MICROTCP_RECVBUF_LEN); 
    //initializing the header (to be sent to client)of microtcp to start the 3-way handshake
    send=initialize(rand()%999+1,0,0,0,SYN,0,MICROTCP_INIT_CWND,0,0,0,0,0);

    for(int i=0;i<MICROTCP_RECVBUF_LEN;i++){
        socket->recvbuf[i]=0;
    }

    memcpy(socket->recvbuf,&send,sizeof(microtcp_header_t));
    if(sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,address,address_len)==-1){
        perror("Send first packet of 3-way handshake error:");
        socket->state=INVALID;
    }else{
        success_counter++;
    }

    printf("Initiating 3-way handshake...\n");	
    if(recvfrom(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,adres,&address_len)==-1){
        perror("Receive from server error:");
        socket->state=INVALID;
    }else{
        receive=(microtcp_header_t*)socket->recvbuf;
        //check if receive was: a SYNACK,acknum=send.seqnumber+1
        if((ntohs(receive->control)==(SYN|ACK))&&(receive->ack_number==send.seq_number+1)){	  	
            send=initialize(send.seq_number+1,ntohl(receive->seq_number+1),ACK,0,0,0,MICROTCP_WIN_SIZE,0,0,0,0,0);
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
    socket->seq_number=1;
    socket->ack_number=1;
    socket->init_win_size=MICROTCP_WIN_SIZE;
    printf("\n3-way handshake achieved!\nConnection established! Seq : %zu  Ack : %zu\n\n",socket->seq_number,socket->ack_number);

    return socket->sd;


}

    int
microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,
        socklen_t address_len)
{

    microtcp_header_t* recv_header;
    microtcp_header_t send_header;
    int isSYNReceived=0;
    int isSYNACKSent=0;
    int isACKReceived=0;

    if(socket->state!=LISTEN)
        return -1;

    recv_header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));

    srand(time(0));


    /* HERE WE RECEIVE THE SYN SIGNAL FROM THE CLIENT */
    printf("Accepting before first receive...\n");
    /*if recvfrom return -1 error */
    if(recvfrom(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,&address_len)==-1){
        perror("Error receiving SYN from client\n");			
    }else{
        memcpy(recv_header,socket->recvbuf,sizeof(microtcp_header_t));
        /*if control is not SYN error */
        if(ntohs(recv_header->control)!=SYN){
            perror("Connection did not start with SYN from client\n");
        }else{
            /* That means that we received the SYN from the client
               so we change the send_header */
            send_header=initialize(rand()%999+1,ntohl(recv_header->seq_number+1),ACK,0,SYN,0,MICROTCP_WIN_SIZE,0,0,0,0,0);
            isSYNReceived=1;
        }
    }
    /* HERE WE SEND THE SYN,ACK SIGNAL BACK TO THE CLIENT */
    memcpy(socket->recvbuf,&send_header,sizeof(microtcp_header_t));
    /*if sendto return -1 error*/
    if(sendto(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,address_len)==-1){
        if(send_header.ack_number!=0)perror("Error sending SYN,ACK to client\n");
        else perror("Error sending ACK=0 to client\n");
    }else{
        isSYNACKSent=1;
    }

    /* if SYN was not received or/and SYN,ACK was not sent */
    if(isSYNReceived*isSYNACKSent==0) return -1;

    /* HERE WE RECEIVE THE ACK SIGNAL FROM THE CLIENT */

    /*if recvfrom return -1 error */
    if(recvfrom(socket->sd,socket->recvbuf,MICROTCP_RECVBUF_LEN,0,address,&address_len)==-1){
        perror("Error receiving ACK from client\n");
    }else{
        memcpy(recv_header,socket->recvbuf,sizeof(microtcp_header_t));
        /*if control is not ACK error */
        if(htons(recv_header->control)!=ACK){
            perror("Did not receive ACK from client\n");
        }else{
            isACKReceived=1;
        }
    }
    /* if ACK was not received */
    if(isACKReceived==0)
        return -1;
    /* if ACK was received, SUCCESS*/
    free(recv_header);
    socket->state=ESTABLISHED;
    socket->seq_number=1;
    socket->ack_number=1;
    socket->init_win_size=MICROTCP_WIN_SIZE;
    printf("\n3-way handshake achieved!\nConnection established! Seq : %zu,  Ack : %zu\n\n",socket->seq_number,socket->ack_number);

    return 0;

}

int

microtcp_shutdown (microtcp_sock_t *socket, int how)
{
	microtcp_header_t header;
	struct sockaddr_in* sin;
	socklen_t len=sizeof(struct sockaddr);
	int data_sent;

	/* Extracting address from socket buffer */
	sin=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	memcpy(sin,socket->recvbuf,sizeof(struct sockaddr_in));

	header=initialize(socket->seq_number,socket->ack_number,ACK,0,0,FIN,socket->curr_win_size,0, sin->sin_family, sin->sin_port, sin->sin_addr.s_addr, 0);
	memcpy(socket->recvbuf,&header,sizeof(microtcp_header_t));

    	printf("Sending FIN ACK\n\n");
   	data_sent=sendto(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,(struct sockaddr*)sin,len);
	    /* Error checking */
	if(data_sent==-1){
		perror("Error sending FIN ACK");
        	return -1;
	}
	
}

    ssize_t
microtcp_send (microtcp_sock_t *socket, const void *buffer, size_t length,int flags)
{
    ssize_t bytes_send,bytes_received;
    struct sockaddr_in sin;
    socklen_t len=sizeof(struct sockaddr);
    microtcp_header_t head;
    uint8_t* newbuf,*newbuf2;	
    size_t last_seq_num;
    int times=1;
    int flag;
    uint32_t size_of_data;
    long int checksum;


	last_seq_num=socket->seq_number;

    /* Extracting address from buffer */
    memset(&sin,0,sizeof(struct sockaddr_in));
    memcpy(&head,buffer,sizeof(microtcp_header_t));
    sin.sin_family=ntohs(head.future_use0);
    sin.sin_port=ntohs(head.future_use1);
    sin.sin_addr.s_addr=ntohl(head.future_use2);

    /* Extracting the data segment to newbuf */
    newbuf2=(uint8_t*)buffer;
    newbuf=(uint8_t*)malloc(length-sizeof(microtcp_header_t));
    memcpy(newbuf,&(newbuf2)[sizeof(microtcp_header_t)],length-sizeof(microtcp_header_t));

    /* Calculating checksum if there is data in the buffer to send */
    if(length!=sizeof(microtcp_header_t)){
        checksum=crc32(newbuf,length-sizeof(microtcp_header_t));              //calculate checksum
        head.checksum=htonl(checksum);
    }
    /* Copying both the header and data segment to newbuf2 to send */
    newbuf2=(uint8_t*)malloc(length);
    memcpy(newbuf2,&head,sizeof(microtcp_header_t));
    memcpy(&(newbuf2)[sizeof(microtcp_header_t)],newbuf,length-sizeof(microtcp_header_t));	

	/* Sending */
    size_of_data=length-sizeof(microtcp_header_t);
    bytes_send=sendto(socket->sd,newbuf2,length,flags,(struct sockaddr*)&sin,len);

	/* Error checking */
    if(bytes_send==-1){
        perror("Error sending the data");
        return -1;
    }
    printf("\nSending data...\n");

    /* receiving ACK if the data are sent correctly */

	/* Timeout function */
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = MICROTCP_ACK_TIMEOUT_US;
    if (setsockopt(socket->sd , SOL_SOCKET ,SO_RCVTIMEO , &timeout ,sizeof(struct timeval)) < 0) {
        perror("setsockopt");
    }

	/* Repeatedly check whether to retransmit or not */
    flag=0;
    do{
	/* Receive the ACK */
        bytes_received=recvfrom(socket->sd,socket->recvbuf,sizeof(microtcp_header_t),0,(struct sockaddr*)&sin,&len);
	
	/* Extract the header */
        memcpy(&head,socket->recvbuf,sizeof(microtcp_header_t));
        if((ntohs(head.control)==ACK)){	/* If ACK was received succesfully */  
            if((ntohl(head.seq_number)!=socket->ack_number)&&(ntohl(head.ack_number)!=socket->seq_number+size_of_data)){ 	/* If packet was not received in order */
                /* Retransmission */
		bytes_send=sendto(socket->sd,newbuf2,length,flags,(struct sockaddr*)&sin,len);
            }else{	/* If packet was received in order */
		socket->seq_number=ntohl(head.ack_number);
                socket->ack_number=ntohl(head.seq_number);
                printf("Packet received with correct order! Seq : %zu,  Ack : %zu\n",socket->seq_number,socket->ack_number);
	            if(socket->seq_number==last_seq_num){ /* If we received a duplicate ACK */
			/* Retransmission */
			bytes_send=sendto(socket->sd,newbuf2,length,flags,(struct sockaddr*)&sin,len);
	                times++;
	                if(times==3){
	                    printf("Triple duplicate ACK received, retransmitting...\n");
	                    times=0;
	                    //fast retransmit (whats the difference between this and normal retransmit)
	                }
	            }else{	/* If the ACK was not duplicate */
	            	printf("ACK received succesfully!\n");
	            	last_seq_num=socket->seq_number;
	            	times=0;
	            	flag=1;		/* flag=1 to end the repetition */
		    }
	    }
        }else{
            if(bytes_received==-1){ /* In case of timeout */
                printf("Error receiving ACK because of timeout, retransmitting the packet...\n");
		/* retransmission */
                bytes_send=sendto(socket->sd,newbuf2,length,flags,(struct sockaddr*)&sin,len);
            }
            printf("Error receiving ACK from server!\n");
        }
    }while(flag==0);

	/* Return the size of the data segment that was sent */
    return bytes_send-sizeof(microtcp_header_t);



}

    ssize_t
microtcp_recv (microtcp_sock_t *socket, void *buffer, size_t length, int flags)
{
    ssize_t bytes_received,bytes_sent;
    struct sockaddr_in sin;
    socklen_t len=sizeof(struct sockaddr);
    microtcp_header_t *header;
    uint8_t *newbuf;
    uint8_t *newbuf2;
    size_t acknum=socket->ack_number;
    long int checksum;
    ssize_t sent;

    printf("\nWaiting to receive data\n");

	/* Extracting address from buffer */
    newbuf=(uint8_t*)buffer;
    header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
    memcpy(header,newbuf,sizeof(microtcp_header_t));
    sin.sin_family=ntohs(header->future_use0);
    sin.sin_port=ntohs(header->future_use1);
    sin.sin_addr.s_addr=ntohl(header->future_use2);

	/* receiving the data from the sender */
    bytes_received=recvfrom(socket->sd,buffer,length,flags,(struct sockaddr*)&sin,&len);

	/* Error checking */
    if(bytes_received<0){
        perror("Error receiving the data");

        return -1;
    }

	/* Extracting the header from the buffer received */
    newbuf=(uint8_t*)buffer;
    header=(microtcp_header_t*)malloc(sizeof(microtcp_header_t));
    memcpy(header,newbuf,sizeof(microtcp_header_t));
	
	/* Changing the seq and ack numbers accordingly */
    socket->ack_number=ntohl(header->seq_number)+(bytes_received -sizeof(microtcp_header_t));//ntohl(header->data_len);
    socket->seq_number=ntohl(header->ack_number);

    if(bytes_received!=sizeof(microtcp_header_t)){		/* if data were sent */

        /* Extracting the data segment from the buffer received */
        newbuf2=(uint8_t*)malloc(bytes_received-sizeof(microtcp_header_t));
        memcpy(newbuf2,&(newbuf)[sizeof(microtcp_header_t)],ntohl(header->data_len));

        /* Calculating checksum */
        checksum=crc32(newbuf2,ntohl(header->data_len));
        if(ntohl(header->checksum)!=checksum){		/* If checksum is not correct */
            printf("Unsuccesful deliver of data: header:%u checked:%ld\n",ntohl(header->checksum),checksum);
            socket->ack_number=acknum;
            return -1;
        }else{			/* If checksum is correct */
            printf("Data delivered succesfully! Seq : %zu,  Ack : %zu\nWriting on file...\n",socket->seq_number,socket->ack_number);
		
		/* Copying the data segment to the returning buffer */
            memcpy(buffer,newbuf2,ntohl(header->data_len));	
        }
    }else{
        /* If no data sent, check for FIN ACK */
        if(htons(header->control)==(FIN|ACK)){
            socket->state=CLOSING_BY_PEER;
            return -1;
        }
    }   

    /* Initialising the header to send ACK if the receiving data were not corrupted */
    newbuf=(uint8_t*)malloc(sizeof(microtcp_header_t));
    *header=initialize(socket->seq_number,socket->ack_number,ACK,0,0,0,socket->curr_win_size,0,0,0,0,0);
	/* Sending ACK back to the sender */
    memcpy(newbuf,header,sizeof(microtcp_header_t));
    sent=sendto(socket->sd,newbuf,sizeof(microtcp_header_t),0,(struct sockaddr*)&sin,len);

	/* Checking if ACK was sent succesfully */
    if(sent==sizeof(microtcp_header_t)&&(ntohs(header->control)==ACK)){
        printf("Sent ACK succesfully!\n\n");
    }else{
        printf("Error sending ACK to client!\n\n");
    }

	/* return the size of the data segment */
    return bytes_received-sizeof(microtcp_header_t);
}


/* Our own Function(s) */

microtcp_header_t initialize(int seq,int ack,int Ack,int Rst,int Syn,int Fin,uint16_t window,uint32_t data_len,uint32_t future_use0,uint32_t future_use1, uint32_t future_use2,uint32_t checksum ){
    microtcp_header_t sock;

    sock.seq_number=htonl(seq);
    sock.ack_number=htonl(ack);
    sock.control=htons(Ack|Rst|Syn|Fin);
    sock.window=htons(window);
    sock.data_len=htonl(data_len);
    sock.future_use0=htons(future_use0);
    sock.future_use1=htons(future_use1);
    sock.future_use2=(int)htonl(future_use2);
    sock.checksum=htonl(checksum);

    return sock;
}
