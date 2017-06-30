/*
 * server_lib.h
 *
 *  Created on: Mar 16, 2017
 *      Author: songshu3
 */

#include "utils.h"

struct client_list{
	char username[MAX_NAME];
	int state;
	int padding1;
	int session;
	int padding2;
    int socket;
    int padding3;
    clock_t last_active;
    int padding4;
    pthread_mutex_t lock;
};

void create_client_list(struct client_list *list, int size);

int create_stream_socket(char *port);

int handle_auth(int socket_id, struct client_list *list);

int handle_request(int socket_id, struct client_list *list, char *buf);

void kick_out(struct client_list *list);
