/*
    microtcp, a lightweight implementation of TCP for teaching,
    academic purpose

    Copyright (C) 2015  Manolis Surligas <surligas@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * bandwidth_test.c
 *
 *  Created on: Oct 25, 2015
 *      Author: surligas
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


#include "../lib/microtcp.h"

int
server_tcp(uint16_t listen_port, char *file)
{
	/*TODO: Write your code here */
	return 0;
}

int
server_microtcp(uint16_t listen_port, char *file)
{
	/*TODO: Write your code here */
	return 0;
}


int
client_tcp(uint16_t server_port, char *file)
{
	/*TODO: Write your code here */
	return 0;
}

int
client_microtcp(uint16_t server_port, char *file)
{
	/*TODO: Write your code here */
	return 0;
}


int
main(int argc, char **argv)
{
	int 		opt;
	int 		port;
	int		exit_code = 0;
	char 		*filestr;
	uint8_t		is_server = 0;
	uint8_t		use_microtcp = 0;

	/* A very easy way to parse command line arguments */
	while ((opt = getopt(argc, argv, "hsmf:p:")) != -1) {
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
			filestr = strdup(optarg);
			/* A few checks will be nice here...*/
			/* Convert the given file to absolute path */
			break;
		case 'p':
			port = atoi(optarg);
			/* To check or not to check? */
			break;

		default:
			printf("Usage: bandwidth_test [-s] [-m] -p port -f file"
			       "Options:\n"
			       "   -s                  If set, the program runs as server. Otherwise as client.\n"
			       "   -m                  If set, the program uses the microTCP implementation. Otherwise the normal TCP.\n"
			       "   -f <string>         If -s is set the -f option specifies the filename of the file that will be saved.\n"
			       "                       If not, is the source file at the client side that will be sent to the server.\n"
			       "   -p <int>            The listening port of the server\n"
			       "   -h                  prints this help\n");
			exit(EXIT_FAILURE);
			}
	}

	/*
	 * TODO: Some error cheking here???
	 */

	/*
	 * Depending the use arguements execute the appropriate functions
	 */
	if(is_server){
		if(use_microtcp){
			exit_code = server_microtcp(port, filestr);
		}
		else{
			exit_code = server_tcp(port, filestr);
		}
	}
	else{
		if(use_microtcp){
			exit_code = client_microtcp(port, filestr);
		}
		else{
			exit_code = client_tcp(port, filestr);
		}
	}
	return exit_code;
}

