
/*- 
 * 
 * New BSD License 2006
 *
 * Copyright (c) 2006, Jorgen Lundman
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1 Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2 Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.  
 * 3 Neither the name of the stuff nor the names of its contributors 
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * send-file example
 *
 * by Jorgen Lundman <lundman@lundman.net> 
 *
 * Opens a listening socket and awaits connection. When one is received,
 * it opens a file (this source file) and sends it to the remote connection.
 * Closes the connection when the file is done.
 * 
 * 09/01/2003 - epoch
 * 22/01/2003 - converting to lib lion syntax
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#include "lion.h"


static int master_switch = 0;
static int server_port   = 57777;  // Set to 0 for any port.


int lion_userinput( lion_t *handle, 
				   void *user_data, int status, int size, char *line)
{
	lion_t *new_handle = NULL;
	lion_t *file = NULL;
	float out;

	switch( status ) {

	case LION_FILE_FAILED:
		printf("[send] file failed event!\n");
		if (user_data) {
			lion_printf(user_data, "%s\r\n", line);
			lion_close(user_data);
		}
		break;

	case LION_FILE_CLOSED:
		printf("[send] file close event!\n");
		// If a file finished, and we had a user_data (ie, socket) close it
		if (user_data) { 
			lion_close(user_data);
		}
		break;
	case LION_FILE_OPEN:
		printf("[send] file open event!\n");
		break;
		
	case LION_CONNECTION_LOST:
		printf("Connection '%p' was lost.\n", handle);
		break;
		
	case LION_CONNECTION_CLOSED:
		printf("Connection '%p' was gracefully closed.\n", handle);

		lion_get_cps(handle, NULL, &out);
		printf("Send took %lu seconds, at %7.2fKB/s\n",
			   lion_get_duration(handle), out);
		break;
		
	case LION_CONNECTION_NEW:
		printf("Connection '%p' has a new connection...\n", handle);

		// Accept the port, use LION_FLAGS_NONE instead of 0.
		new_handle = lion_accept(handle, 0, 0, NULL, NULL, NULL);

		break;

		// We have a new connection, send the greeting:
		// This isn't called here. We leave it for debugging reasons.
	case LION_CONNECTION_CONNECTED:

		break;

	case LION_BUFFER_USED:
		break;

	case LION_BUFFER_EMPTY:
		break;

	case LION_INPUT:
	  break;


	case LION_BINARY:
	  break;
	}



	return 0;

}








void exit_interrupt(void)
{

	master_switch = 1;

}






int main(int argc, char **argv)
{
	lion_t *file1 = NULL;
	lion_t *file2 = NULL;


	signal(SIGINT, exit_interrupt);
#ifndef WIN32
	signal(SIGHUP, exit_interrupt);
#endif



	printf("Initialising Network...\n");

	lion_init();
	lion_compress_level( 0 );

	printf("Network Initialised.\n");



	// Create an initial game
		
	printf("Initialising Socket...\n");


	file1 = lion_open("locktest.bin", O_WRONLY|O_CREAT,
			  0600, LION_FLAG_EXCLUSIVE,
			  NULL);
	if (file1) 
	  printf("File1 open successful (that's GOOD)\n");
	else
	  printf("File1 open failed     (that's BAD)\n");

	file2 = lion_open("locktest.bin", O_WRONLY|O_CREAT,
			  0600, LION_FLAG_EXCLUSIVE,
			  NULL);
	if (file2) 
	  printf("File2 open successful (that's BAD)\n");
	else
	  printf("File2 open failed     (that's GOOD)\n");

	sleep(30);

	return 0; // Avoid warning

}
