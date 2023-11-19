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

/*
 * You can use this file to write a test microTCP client.
 * This file is already inserted at the build system.
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>  
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include "../lib/microtcp.h"

int main(int argc,char **argv){
    microtcp_sock_t clientSocket; /* Socket Decriptor for Client */
    struct sockaddr_in server_addr;
    struct hostent *ptrh;

    char message[100];
    char received[100];
    int n = 0;

    //Check if port number is given
    if(argv[1] == NULL){
        perror("No port added!Execute the command with \"test_microtcp_client [port_number]\" ");
        exit( EXIT_FAILURE );
    }
    clientSocket = microtcp_socket(AF_INET, SOCK_DGRAM, 0);
    if(clientSocket.sd == -1){
        perror ( " SOCKET COULD NOT BE OPENED " );
        exit ( EXIT_FAILURE );
    }
    
    // clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    memset((char*)&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));

    /*  bind(clientSocket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)); */

    ptrh = gethostbyname("127.0.0.1");
    memcpy(&server_addr.sin_addr,ptrh->h_addr,ptrh->h_length);

    if( -1 == connect(clientSocket.sd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        printf("\nServer Not Ready !!\n"); exit(1);
    }

    while(1)
    {
        printf("\nUser:-");
        // memset(message, '\0', 10);

        gets(message);

        n = write(clientSocket.sd, message, strlen(message)+1);
        if( (strcmp(message,"q") == 0 ) || (strcmp(message,"Q") == 0 )){
            printf("Wrong place...Socket Closed\n");
            close(clientSocket.sd);
            break;
        }
            //printf("Write:<%u>\n", n);

        read(clientSocket.sd, received, sizeof(received));
        if( (strcmp(received,"q") == 0 ) || (strcmp(received,"Q") == 0 )){
            printf("Wrong place...Socket Closed\n");
            close(clientSocket.sd);
            break;
        }
        else{
            printf("Server:- %s\n", received);
        }
    }

    return 0;
}