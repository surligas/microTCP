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
 * You can use this file to write a test microTCP server.
 * This file is already inserted at the build system.
 */

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include "../lib/microtcp.h"

int main(int argc,char **argv) {
    microtcp_sock_t serverSocket;
    int client_connected,len;
    struct sockaddr_in client_addr,server_addr;
    struct hostent *ptrh;
    int n=0; 
    char message[100],received[100];

    //Check if port number is given
    if(argv[1] == NULL){
        perror("No port added!Execute the command with \"test_microtcp_server [port_number]\" ");
        exit( EXIT_FAILURE );
    }

    serverSocket = microtcp_socket(AF_INET, SOCK_DGRAM, 0);
    if(serverSocket.sd == -1){
        perror ( " SOCKET COULD NOT BE OPENED " );
        exit ( EXIT_FAILURE );
    }

    memset((char*)&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(microtcp_bind(&serverSocket,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1){
        printf("Bind Failure\n");
    }
    else{
        printf("Bind Success:<%u>\n", serverSocket.sd);
    }

    while(1){   
        listen(serverSocket.sd,5);
        len = sizeof(struct sockaddr_in);

        client_connected = accept(serverSocket.sd,
        (struct sockaddr*)&client_addr,&len);
        if (-1 != client_connected){
            printf("Connection accepted:<%u>\n", client_connected);
        }

        while(1){
            n = read(client_connected, received, sizeof(received));
            if( (strcmp(received,"q") == 0 ) || (strcmp(received,"Q") == 0 )){
                printf("Wrong place...Socket Closed of Client\n");
                close(client_connected);
                break;
            }
            else{
                printf("\nUser:-%s", received);
            }
            printf("\nServer:-");
            //  memset(message, '\0', 10);
            gets(message);             
            write(client_connected, message, sizeof(message));
            if( (strcmp(message,"q") == 0 ) || (strcmp(message,"Q") == 0 )){
                printf("Wrong place...Socket Closed of Client\n");
                close(client_connected);
                break;
            
            }  
        }
    }

    close(serverSocket.sd); printf("\nServer Socket Closed !!\n");

    return 0;
}