/*
 * utils.c
 *
 *  Created on: Mar 15, 2017
 *      Author: songshu3
 */

#include "utils.h"

struct tm *current_time() {
	time_t rawtime;
	struct tm *realtime;
	time( &rawtime );
	realtime = localtime (&rawtime);

	return realtime;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*) sa)->sin_addr);
    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void parse_packet(char *buffer, struct packet *pkt, int direction) {
    int index = 0;
    int i = 0;
    char work[1024];
    if (direction == 1) {
        while (buffer[index] != ':') {
            work[i] = buffer[index];
            i++;
            index++;
        }
        work[i] = '\0';
        pkt->type = atoi(work);
        i = 0;
        index++;

        while (buffer[index] != ':') {
            work[i] = buffer[index];
            i++;
            index++;
        }
        work[i] = '\0';
        pkt->size = atoi(work);
        i = 0;
        index++;

        while (buffer[index] != ':') {
            pkt->source[i] = buffer[index];
            i++;
            index++;
        }
        pkt->source[i] = '\0';

        i = 0;
        index++;
        while (i < pkt->size) {
            pkt->data[i] = buffer[index];
            i++;
            index++;
        }
        pkt->data[i] = '\0';
    } else if (direction == 2) {
        sprintf(buffer, "%d:%d:%s:%s%s", pkt->type, pkt->size, pkt->source, pkt->data, "\0");
    }
}

void process_data(struct packet *pkt, struct processed_packet *pr_pkt, int direction) {
    int index = 0;
    int i = 0;
    if (direction == 1) {
        pr_pkt->type = pkt->type;
        pr_pkt->size = pkt->size;
        strcpy(pr_pkt->source, pkt->source);
        if (pr_pkt->type < 1000) {
        	strcpy(pr_pkt->parameter1, "\0");
        	strcpy(pr_pkt->parameter2, "\0");
        	return;
        }
        if (pr_pkt->type > 1000) {
            while (pkt->data[i] != ':') {
                pr_pkt->parameter1[i] = pkt->data[index];
                i++;
                index++;
            }
            pr_pkt->parameter1[i] = '\0';
            strcpy(pr_pkt->parameter2, "\0");
        }
        if (pr_pkt->type > 2000) {
            i = 0;
            index++;
            while (pkt->data[i] != '\0') {
                pr_pkt->parameter2[i] = pkt->data[index];
                i++;
                index++;
            }
            pr_pkt->parameter2[i] = '\0';
        }
    } else if (direction == 2) {
        pkt->type = pr_pkt->type;
        strcpy(pkt->source, pr_pkt->source);
        if(pkt->type > 2000) {
        	sprintf(pkt->data, "%s:%s%s", pr_pkt->parameter1, pr_pkt->parameter2, "\0");
        } else if (pkt->type > 1000) {
        	sprintf(pkt->data, "%s:%s", pr_pkt->parameter1, "\0");
        } else {
        	sprintf(pkt->data, "::%s", "\0");
        }
        pkt->size = strlen(pkt->data);
    }
}

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
