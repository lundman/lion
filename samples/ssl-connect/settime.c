
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
#include <sys/time.h>
#include "lion.h"

#define WITH_STDIN

static int master_switch = 0;

static lion_t *remote = NULL;

#ifdef WITH_STDIN
static lion_t *lstdin = NULL;
#endif


#define SENDOK "%c0"   // OK and DELETE
//#define SENDOK "%c1"   // OK and KEEP

char settime[15];



// Change the next define depending on which type of SSL you are testing
// this refers to logic flow, and in which order you receive the events.
// SSL_CONNECT_FIRST - Connect, wait for success, then issue SSL
// SSL_CONNECT_LATER - Connect and issue SSL
//#define SSL_CONNECT_FIRST
//#define SSL_CONNECT_LATER
//#define SSL_CONNECT_FTPD     // Talk a little FTP protocol
//#define SSL_CONNECT_STARTTLS   // Talk STARTTLS (sendmail, pop)

void binary_engine(lion_t *handle, char *line, int size);
void parse_entry(char *cmd, int cmdlen);


int lion_userinput( lion_t *handle,
					void *user_data, int status, int size, char *line)
{
	int i;
	static int state = 0;

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
		lion_printf(handle, "SSL\r\n");
#endif
		printf("Connected, sending challenge\n");
		printf("<04\"0900\"05\n");
		lion_enable_binary(handle);
		//lion_printf(handle, "%c0900%c\n", 0x04, 0x05);
		binary_engine(handle, line, size);
		break;
#if 0
	case LION_CONNECTION_SECURE_ENABLED:
		printf("[remote] SSL successfully established\n");
		break;

	case LION_CONNECTION_SECURE_FAILED:
		printf("[remote] SSL failed\n");
		// If plain is not acceptable, call lion_disconnect() here
		break;
#endif
	case LION_BUFFER_USED:
		break;

	case LION_BUFFER_EMPTY:
		break;

		// Using text mode in this example
		break;

	case LION_BINARY:
	case LION_INPUT:
		if (handle == remote) {

			binary_engine(handle, line, size);

		}
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

    // Expect IP port [date]
    if (argc < 3) {
        printf("%s ip port [date]\n", argv[0]);
        printf("If no time specified, it uses current time.\n");
        exit(0);
    }
    if (argc == 4) {
        if (strlen(argv[3]) != 14) {
                printf("Specify date string as yyyymmddHHMMSS. Ie '20090107090900'\n");
                exit(0);
            }
            strcpy(settime, argv[3]);
    } else {
        time_t tim;
        struct tm *tm;
        // Build time string.
        time(&tim);
        tm = localtime(&tim);
        strftime(settime, sizeof(settime),
                 "%G%m%d%H%M%S", tm);
    }

    printf("Time to be set to '%s'\n", settime);

    remote = lion_connect(argv[1], atoi(argv[2]), NULL, 0,
                          LION_FLAG_FULFILL, NULL);

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

	return 0;

}


unsigned char getCRC(char *buf, int len)
{
    int i;
    unsigned char crc = 0xa0;

    for (crc = 0, i = 0;
         i < len;
         i++) {
        crc ^= buf[i];
    }
    printf("Computed CRC as %02X\n", crc);
    return crc;
}


int binary_input(char *line, int size, char **cmd, int *cmdlen)
{
    static char inbuffer[1024] = {0};
    static int bufferlen = 0;
    static int msgtype = 0;
    int found = 0, i;
    unsigned char crc = 0xfe;

    *cmd = inbuffer;
    *cmdlen = 0;

    if (!line || !size) return 0;

    // If we are at len 0, check the type of reply.
    if (!bufferlen) {
        switch(line[0]) {
        case 0x01:  // 0x01-0x02-0x03 packet
            msgtype = 1;
            break;
        case 0x04:  // Ready message.
            msgtype = 4;
            break;
        case 0x05:  // idle? ayt msg
            msgtype = 5;
            break;
        case 0x10:  // status msg
            msgtype = 0x10;
            break;
        case 0x15:  // error
            msgtype = 0x15;
            break;
        default:
            // We can receive "0600"0x04. Which means empty 0600.
            msgtype = 0xff; // data, read until 0x04.
            break;
        }
        inbuffer[0] = line[0];
        bufferlen = 1;
        line++;
        size--;
    } // !bufferlen

    *cmd = inbuffer;
    *cmdlen = bufferlen;


    switch(msgtype) {
    case 0x04: // OK, no more bytes
    case 0x05: //
    case 0x15: //
        if (size > 0) printf("Warning, dropping %d bytes from %02X msgtype\n",
                             size, msgtype);
        bufferlen = 0;
        return 1;

    case 0xff: // data, read until 0x04.
        while(size > 0) {
            inbuffer[ bufferlen ] = *line;
            bufferlen++;

            if (*line == 0x04) { // ok, we've finished.

                *cmdlen = bufferlen;
                size--;
                line++;
                if (size > 0) printf("Warning, dropping %d bytes from %02X msgtype\n",
                                     size, msgtype);
                bufferlen = 0;
                return 1;

            }

            size--;
            line++;
        } // while size
        return 0; // not found yet.

    case 0x10: // one byte status code
        if (size <= 0) return 0; // wait for it.

        inbuffer[ bufferlen ] = *line;
        bufferlen++;
        *cmdlen = bufferlen;
        size--;
        line++;
        if (size > 0) printf("Warning, dropping %d bytes from %02X msgtype\n",
                             size, msgtype);
        bufferlen = 0;
        return 1;

    case 0x01: // read until 0x03. Perhaps, this should be split into 3 types
        while(size > 0) {
            inbuffer[ bufferlen ] = *line;
            bufferlen++;

            if (*line == 0x17) *line = 0x03;

            if (*line == 0x03) { // Just one more byte
                if (size <= 1) printf("Warning, no checksum byte after 0x03\n");
                if (size >  2) printf("Warning, dropping extra %d bytes after 0x030x?? \n", size);
                size = 2; // Just read one more
                found = 1;
            }

            size--;
            line++;
        } // while size

        if (!found) return 0;

        if (bufferlen > 6)
            crc = getCRC(&inbuffer[6], bufferlen -6 -1 ); // 0x03 inclusive

        if (crc == inbuffer[bufferlen - 1])
            printf("** CRC good (%02X)\n", crc);
        else
            printf("** CRC bad (%02X != %02X)\n", crc,inbuffer[bufferlen - 1]);

        // Finished decoding.
        *cmdlen = bufferlen;
        bufferlen = 0;
        return 1;

    default:
        printf("Can't decode packet type %d\n", msgtype);
        bufferlen = 0;
        return 0;
    }

    bufferlen = 0;
    return 0;
}


void binary_engine(lion_t *handle, char *line, int size)
{
	static int state = 0, n;
    static int num_entries = 0;
    int cmdlen;
    char *cmd;
    char buf[1024];
    int len;
	int i, process;
    unsigned char crc;


	// If we received valid input, print it
#if 1
	if ( size > 0 && line && *line) {
		for (i = 0; i < size; i++) {
			printf("%s%02X ", !(i%16) ? "\n" : "", line[i]);
		}

		printf("\n>'%*.*s'\n", size, size, line);
	}
#endif

    // Processed a command?
    if (!line && !size)
        process = 1; // Send initial greeting
    else {
        process = binary_input(line, size, &cmd, &cmdlen);

        if (process) {
            printf("\nDecoded (%d bytes) :", cmdlen);
            for (i = 0; i < cmdlen; i++) {
                printf("%s%02X ", !(i%16) ? "\n:" : "", cmd[i]);
            }

		printf("\n>'%*.*s'\n", cmdlen, cmdlen, cmd);
        }
    }

    if (!process) return;

    //printf("binary input says %d : '%*.*s'\n", cmdlen, cmdlen, cmdlen, cmd);
    printf("\n");

	switch(state) {

	case 0: // send 0900 greeting
        printf("========================================================================\n");
		printf("< sending 0400 gettime\n");
		lion_printf(handle, "%c0400%c", 0x04, 0x05);
        state++;
		break;

	case 1:

        printf("========================================================================\n");

        printf("< sending 1101 query\n");
        lion_printf(handle, "%c1101%c", 0x04, 0x05);
        state++;
        break;


    case 2:

        printf("1101 status %s (%02X)\n",
               cmd[1] == 0x30 ? "OK" : "NG", cmd[1]);

        printf("========================================================================\n");

        printf("Attempting to settime\n");
        snprintf(buf, sizeof(buf), "%c11%s%c",
                 0x02, settime, 0x03);
        len = strlen(buf);
        buf[len] = getCRC(&buf[1], len - 1) ;
        len++;

        for (i = 0; i < len; i++) {
            printf("%s%02X ", !(i%16) ? "\n:" : "", buf[i]);
        }
        printf("\nSending %d bytes:\n'%*.*s'\n", len, len, len, buf);
        lion_output(handle, buf, len);
        state++;
        break;

    case 3:
        if (*cmd == 0x10) {
            printf("Operation returned 10%02x! \n", cmd[1]);
            // Send ACK
            lion_printf(handle, "%c", 0x04);
        }

        master_switch = 1;
        break;


    default:
        printf("Processing complete, disconnecting.\n");
        master_switch = 1;
        break;
    }

}


