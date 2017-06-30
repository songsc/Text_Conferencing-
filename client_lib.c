/*
 * client_lib.c
 *
 *  Created on: Mar 16, 2017
 *      Author: songshu3
 */

#include "client_lib.h"

int authenticate(int socket_id, char *source)
{
	int numbytes;
	char buf[MAX_DATA], input[MAX_DATA];
	struct processed_packet pr_pkt;
	struct packet pkt;
	printf("Login: ");
	scanf("%s %s %s", input, pr_pkt.parameter1, pr_pkt.parameter2);
	if(strcmp(input, "/login") == 0)
		pr_pkt.type = LOGIN;
	else
		exit(0);
	strcpy(pr_pkt.source, pr_pkt.parameter1);
	strcpy(source, pr_pkt.source);
	process_data(&pkt, &pr_pkt, 2);
	parse_packet(buf, &pkt, 2);
	send(socket_id, buf, strlen(buf), 0);
	assert((numbytes = recv(socket_id, buf, MAX_DATA - 1, 0)) >= 0);
	buf[numbytes] = '\0';
	parse_packet(buf, &pkt, 1);
	process_data(&pkt, &pr_pkt, 1);
	if(pr_pkt.type == LO_ACK)
	{
		printf("Login Successful. \n");
		return atoi(pr_pkt.source);
	}
	if(pr_pkt.type == LO_NAK)
	{
		printf("Login Failed. \n");
		return -1;
	}
	return -1;
}

int create_stream_socket(char *host, char *port)
{
	int socket_id;
	struct addrinfo new, *server_info, *p;
	char s[INET6_ADDRSTRLEN];

	memset(&new, 0, sizeof new);
	new.ai_family = AF_UNSPEC;
	new.ai_socktype = SOCK_STREAM;

	assert(getaddrinfo(host, port, &new, &server_info) == 0);

	for(p = server_info; p != NULL; p = p->ai_next)
	{
		if ((socket_id = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
		{
			printf("Client: socket failed, socket_id = %d, checking next one. \n", socket_id);
			continue;
		}
		if (connect(socket_id, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(socket_id);
			printf("Client: connect failed, checking next one \n");
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		printf("Client: connect failed, exiting. \n");
		return -1;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("Client: Connecting to %s. \n", s);
	freeaddrinfo(server_info); // all done with this structure

	return socket_id;
}

void receiveMessage(int socket_id) {
    int numbytes;
    char buf[MAX_DATA];
    struct processed_packet pr_pkt;
    struct packet pkt;
    while(1) {
        assert((numbytes = recv(socket_id, buf, MAX_DATA - 1, 0)) >= 0);
        buf[numbytes] = '\0';
        parse_packet(buf, &pkt, 1);
        process_data(&pkt, &pr_pkt, 1);
        if (pr_pkt.type == JN_ACK) {
        	printf("Joined Session: %s \n", pr_pkt.parameter1);
        } else if(pr_pkt.type == JN_NAK) {
            printf("Can not Join Session \n");
        } else if(pr_pkt.type == DATA) {
        	printf("Client %s: %s \n", pr_pkt.parameter1, pr_pkt.parameter2);
        } else if (pr_pkt.type == LS_ACK) {
            printf("Left Session. \n");
        } else if (pr_pkt.type == NS_ACK) {
            printf("Created Session: %s \n", pr_pkt.parameter1);
        } else if (pr_pkt.type == QUERY_R) {
        	printf("%s \n", pkt.data);
        } else if (pr_pkt.type == TOUT) {
        	printf("Time Out \n");
        	close(socket_id);
        	return;
        }
    }
}
