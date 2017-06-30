/*
 * utils.h
 *
 *  Created on: Mar 15, 2017
 *      Author: songshu3
 */

#ifndef UTILS_H_
#define UTILS_H_



#endif /* UTILS_H_ */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT "20028"
#define MAX_DATA 1000
#define MAX_NAME 100

/*
 * Convention:
 * No parameter: Less than 1000
 * One parameter: Between 1000 and 2000
 * Two parameters: Between 2000 and 3000
 */
#define LOGIN 2001
#define LO_ACK 902
#define LO_NAK 1003
#define EXIT 904
#define JOIN 1005
#define JN_ACK 1006
#define JN_NAK 2007
#define LEAVE_SESS 908
#define NEW_SESS 909
#define NS_ACK 1010
#define MESSAGE 1011
#define QUERY 912
#define QU_ACK 1013
#define LS_ACK 1014
#define DATA 2015
#define INVITE 2016
#define INVI_ACK 917
#define INVI_NAK 918
#define QUERY_R 2019
#define TOUT 920

#define TIME_OUT 30


struct packet {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
};

struct processed_packet {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char parameter1[MAX_DATA];
	unsigned char parameter2[MAX_DATA];
};

struct tm *current_time();

void *get_in_addr(struct sockaddr *sa);

void parse_packet(char *buffer, struct packet *pkt, int direction);

void process_data(struct packet *pkt, struct processed_packet *pr_pkt, int direction);

void sigchld_handler(int s);
