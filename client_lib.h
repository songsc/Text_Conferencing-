/*
 * client_lib.h
 *
 *  Created on: Mar 16, 2017
 *      Author: songshu3
 */

#ifndef CLIENT_LIB_H_
#define CLIENT_LIB_H_



#endif /* CLIENT_LIB_H_ */

#include "utils.h"

int authenticate(int socket_id, char *source);

int create_stream_socket(char *host, char *port);

void receiveMessage(int socket_id);