
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


//#define SENDOK "%c0"   // OK and DELETE
#define SENDOK "%c1"   // OK and KEEP


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
		printf("< sending 0900 greeting\n");
		lion_printf(handle, "%c0900%c", 0x04, 0x05);
        state++;
		break;

	case 1:
        // 01 "0001" 02 "09000021800000000240000000              " 03 07
        if (cmdlen != 48) printf("Unexpected length state %d (%d)\n",
                                 state, cmdlen);

        if (*cmd == 0x01) { // Valid reply
            unsigned char keep;

            // Find how many entries the unit has
            keep = cmd[25];
            cmd[25] = 0;
            num_entries = strtol(&cmd[22], NULL, 10); // 22 is 3 digits.
            cmd[25] = keep;

            printf("Unit has %d entries for retrieval\n", num_entries);


        }


        printf("========================================================================\n");

        // We send 1030 here to clear it.
        printf("< sending 1030 query\n");
        lion_printf(handle, SENDOK, 0x10);
        state++;
        break;


    case 2:
        // Should be 0x04 to 1030

        printf("========================================================================\n");

        printf("< sending 0700 query\n");
        lion_printf(handle, "%c0700%c", 0x04, 0x05);
        state++;
        break;

    case 3:

        if (*cmd != 0x01)
            printf("Empty 0700 reply.\n");

        printf("========================================================================\n");

        printf("< sending 0600 query\n");
        lion_printf(handle, "%c0600%c", 0x04, 0x05);
        state++;
        break;

    case 4: // called until all data downloaded

        printf("========================================================================\n");

        if (*cmd == 0x01) {

            parse_entry(cmd, cmdlen);

            // DATA, send ACK
            printf("< sending 1030 query\n");
            lion_printf(handle, SENDOK, 0x10);
            break;
        }

        if (*cmd == 0x04) {
            // End-of-data
            printf("Finished retreiving packets\n");
        } else {
            printf("Empty 0600 reply.\n");
        }

        master_switch = 1;
        break;

        printf("< sending 1901 query (enter receive mode?)\n");
        lion_printf(handle, "%c1901%c", 0x04, 0x05);
        state++;
        break;


    case 5:
        printf("1901 status %s (%02X)\n",
               cmd[1] == 0x30 ? "OK" : "NG", cmd[1]);

        printf("========================================================================\n");

        printf("< sending 1911 clear command\n");
        // 01 30303032 02 3139 3131 202020202020202020202020 03 0b
        // " 0002 1911____________  "  (12 spaces)

        snprintf(buf, sizeof(buf), "%c0002%c1911            %c",
                 0x01,0x02,0x03);
        len = strlen(buf);
        buf[len] = getCRC(&buf[6], len - 6);
        lion_output(handle, buf, len+1);
        state++;
        break;

    case 6:
        printf("1911 status %s (%02X) Expecting 0x31 \n",
               cmd[1] == 0x31 ? "OK" : "NG", cmd[1]);

        printf("========================================================================\n");

        printf("< sending final 0600 query\n");
        // 0000 = empty
        // 0100 = 0001 01 2008 12 30 1351 19    Time?
        // 0200 = '000102050100000000020001    030002    040002    0502560008060000    0700030003080000    090001    100000000011000000001200000000    '
        // 0300 = empty
        // 0400 = empty
        // 0500 = empty
        // 0600 = get time-entries
        // 0700 = get time, empty
        // 0800 = empty
        // 0900 = get serial and number of entries
        // 1000 = empty
        // 1100 = empty
        // 1200 = empty
        // ...
        // 2100 = empty

        lion_printf(handle, "%c0600%c", 0x04, 0x05);
        state++;
        break;

    case 7:
        if (*cmd != 0x01)
            printf("Empty 0600 reply.\n");
        else
            printf("Strange, we expected no data here!\n");

        printf("========================================================================\n");

        master_switch = 1;
        break;

#if 0  // SET TIME
        printf("< sending 1101 query\n");
        // 0101 = ERROR
        // ...
        // 0801 = ERROR
        // 0901 = ERROR
        // 1001 = 04
        // 1101 = 04
        // 1201 = 04
        // 1301 = 04
        // 1401 = ERROR
        // 1501 = ERROR
        // 1601 = 04
        // 1701 = ERROR
        // 1801 = ERROR
        // 1901 = 04     // used for clearing num records
        // 3101 = ERROR

        lion_printf(handle, "%c1101%c", 0x04, 0x05);
        state++;
        break;


    case 8:

        printf("1101 status %s (%02X)\n",
               cmd[1] == 0x30 ? "OK" : "NG", cmd[1]);

        printf("========================================================================\n");

        printf("Attempting write-back\n");
        snprintf(buf, sizeof(buf), "%c1120090107090900%c",
                 0x02, 0x03);

        // "05" is wrong length. Too short, too long.
        // 15 bad CRC

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

    case 9:
        if (*cmd == 0x10) {
            printf("Operation returned 10%02x! \n", cmd[1]);
            // Send ACK
            lion_printf(handle, "%c", 0x04);
        }

        master_switch = 1;
#endif

        n = 4;
    stuff:
        // case 7
        printf("< sending 1601 query\n");
        lion_printf(handle, "%c1901%c", 0x04, 0x05);
        state++;
        break;

        // 1001  "08"      10* -> "05" expects n=16
        // 1201  "08"      12* -> "05"
        // 1301  "08"      13* -> "05" expects n=24
        // 1401  15 error
        // 1501  15 error
        // 1601  "08"      16* -> "05"
        // 1701  15 error
        // 1801  15 error
        // 1901  "08"      1931 -> "05" expects n=16

    case 8:
        printf("1601 status %s (%02X)\n",
               cmd[1] == 0x30 ? "OK" : "NG", cmd[1]);

        printf("========================================================================\n");

        printf("Attempting write-back: %d\n", n);

        //lion_printf(handle, "%c0001", 0x01);

        snprintf(buf, sizeof(buf), "%c%*.*s%c",
                 0x02,
                 n,n,"19013120081230181500010000001222000131200812301815000100000012220001312008123018150001000000122200013120081230181500010000001222000131200812301815000100000012220001312008123018150001000000122200013120081230181500010000001222000131200812301815000100000012220001                                                                                                                                                                                                                                                                                                                                                                          ",
                 0x03);

        // "05" is wrong length. Too short, too long.
        // 15 bad CRC
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


    case 9:
        if (*cmd == 0x10) {
            printf("Operation returned 10%02x! \n", cmd[1]);
            // Send ACK
            lion_printf(handle, "%c", 0x04);
        }

        if (*cmd == '0' &&
            cmd[1] == '5') {
            n++;
            state = 7;
            goto stuff;
        }

        master_switch = 1;
        break;


    default:
        printf("Processing complete, disconnecting.\n");
        master_switch = 1;
        break;
    }

}





// ========================================================================
//       ID # | KeyID |    Date/Time |     Status |    Special | Flag
// 0000001222 |    31 | 200812311611 |    0002    |      00    |  01
//
// Punch-IN : 0001 00 01
// Punch-OUT: 0002 00 01
// Lunch-OUT: 0003 89 01
// Lunch-IN : 0004 00 01
//
// Or perhaps Lunch-OUT should be considered "punched out from work temporarily
// with default reason set to 89, which we consider to be lunch".
//
void parse_entry(char *cmd, int cmdlen)
{
    int i;

    if (cmdlen != 264) {
        printf("Expected cmdlen of 264, not %d.\n", cmdlen);
        return;
    }

    printf("%10.10s | %5.5s | %12.12s | %10.10s | %10.10s | %4.4s\n",
           "ID #", "KeyID", "Date/Time", "Status  ", "Special", "Flag");

    for (i = 6; i < 262; i += 32) {

        if (cmd[i] == 0x20) continue; // empty record, skip it.

        //      6 8           20  24        34
        // 0001 31200812311611000200000012220001
        printf("%10.10s |    %2.2s | %12.12s |    %4.4s    |      %2.2s    |  %2.2s \n",
               &cmd[i+18],
               &cmd[i+0],
               &cmd[i+2],
               &cmd[i+14],
               &cmd[i+28],
               &cmd[i+30]);


    } // for all records (up to 8)
    sleep(5);
}


