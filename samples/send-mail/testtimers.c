/*
 * SSL connect sample
 *
 * by Jorgen Lundman <lundman@lundman.net> 
 *
 * Connects to an SSL ip/port and tries to authenticate.
 * 
 * 28/01/2003 - epoch
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "lion.h"

#define WITH_STDIN

static int master_switch = 0;

static lion_t *remote = NULL;

#ifdef WITH_STDIN
static lion_t *lstdin = NULL;
#endif

// Change the next define depending on which type of SSL you are testing
// this refers to logic flow, and in which order you receive the events.
//#define SSL_CONNECT_FIRST
//#define SSL_CONNECT_LATER
#define SSL_CONNECT_FTPD


int lion_userinput( lion_t *handle, 
					void *user_data, int status, int size, char *line)
{
	
		
	switch( status ) {
		
	case LION_CONNECTION_LOST:
		printf("[remote] connection lost: %d:%s\n", size, line);
		remote = NULL;
		break;

	case LION_CONNECTION_CLOSED:
		printf("[remote] connection closed\n");
		remote = NULL;
		master_switch = 1;
		break;
		
	case LION_CONNECTION_CONNECTED:
		printf("[remote] connection connected\n");
#ifdef SSL_CONNECT_FIRST
		lion_ssl_set(handle, LION_SSL_CLIENT);
#endif

		//		lion_printf(handle, "GET /onamae/navi/Login.jsp HTTP/1.0\r\n");
		//lion_printf(handle, "Host: ddns.onamae.com\r\n");
		//lion_printf(handle, "Accept: */*\r\n\r\n");

		

		break;

	case LION_CONNECTION_SECURE_ENABLED:
		printf("[remote] SSL successfully established\n");
		break;

	case LION_CONNECTION_SECURE_FAILED:
		printf("[remote] SSL failed\n");
		// Demonstrate how we can accept only SSL.
		//net_disconnect(handle);
		break;

	case LION_BUFFER_USED:
		break;
		
	case LION_BUFFER_EMPTY:
		break;
		
	case LION_BINARY:
		// Using text mode in this example
		break;
		
	case LION_INPUT:

		if (handle == remote)
			printf("%s\n", line);
#ifdef WITH_STDIN
		else
			lion_printf(remote,"%s\n", line);
#endif

#ifdef SSL_CONNECT_FTPD
		if ((size > 4) && !strncmp("234 ", line, 4)) {
			printf("STARTING SSL\n");
			lion_ssl_set(handle, LION_SSL_CLIENT);
		}
#endif


	}
	


	return 0;

}








void exit_interrupt(void)
{

	master_switch = 1;

}






int main(int argc, char **argv)
{

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

	printf("Parent starting...\n");


	if (argc != 3) {
		
		remote = lion_connect("127.0.0.1", 56688, NULL, 0,  
							  LION_FLAG_FULFILL, NULL);

	} else {

		remote = lion_connect(argv[1], atoi(argv[2]), NULL, 0,  
							  LION_FLAG_FULFILL, NULL);

	}


#ifdef SSL_CONNECT_LATER
	lion_ssl_set(remote, LION_SSL_CLIENT);
#endif

#ifdef WITH_STDIN
	lstdin = lion_adopt(fileno(stdin), LION_TYPE_FILE, NULL);
#endif


	while( !master_switch ) {

		lion_poll(0, 1);     // This blocks. (by choice, FYI).

	}
	printf("\n");

	// If we are parent, release child
	if (remote)
		lion_disconnect(remote);

	lion_free();

	printf("Network Released.\n");

	printf("Done\n");

	// For fun, lets return a random return code 
	return 0;

}
