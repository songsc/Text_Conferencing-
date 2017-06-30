/*
 * client.c
 *
 *  Created on: Mar 15, 2017
 *      Author: songshu3
 */

#include <pthread.h>
#include "client_lib.h"

int main(int argc, char *argv[]) {
	int socket_id, numbytes;
	char buf[MAX_DATA], input[MAX_DATA], source[MAX_NAME];
	struct processed_packet pr_pkt;
	struct packet pkt;
	pthread_t read_t;

	assert(argc == 2);
	socket_id = create_stream_socket(argv[1], "5580");
	assert(socket_id != 0);

	int new_socket_id = authenticate(socket_id, source);
	pthread_create(&read_t, NULL, receiveMessage, socket_id);
	while (new_socket_id > 0) {
		scanf("%s", input);
		if (strcmp(input, "/logout") == 0) {
			pr_pkt.type = EXIT;
			strcpy(pr_pkt.source, source);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
			printf("%d %s \n", socket_id, buf);
			close(socket_id);
			return 0;
		} else if (strcmp(input, "/joinsession") == 0) {
			printf("Session ID: ");
			scanf("%s", buf);
			pr_pkt.type = JOIN;
			strcpy(pr_pkt.source, source);
			strcpy(pr_pkt.parameter1, buf);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		} else if (strcmp(input, "/leavesession") == 0) {
			pr_pkt.type = LEAVE_SESS;
			strcpy(pr_pkt.source, source);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		} else if (strcmp(input, "/createsession") == 0) {
			pr_pkt.type = NEW_SESS;
			strcpy(pr_pkt.source, source);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		} else if (strcmp(input, "/list") == 0) {
			pr_pkt.type = QUERY;
			strcpy(pr_pkt.source, source);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		} else if (strcmp(input, "/quit") == 0) {
			close(socket_id);
			return 0;
		} else if (strcmp(input, "/invite") == 0) {
			printf("User Name: ");
			scanf("%s", buf);
			pr_pkt.type = INVITE;
			strcpy(pr_pkt.source, source);
			strcpy(pr_pkt.parameter1, buf);
			printf("Session: ");
			scanf("%s", buf);
			strcpy(pr_pkt.parameter2, buf);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		} else {
			pr_pkt.type = DATA;
			strcpy(pr_pkt.source, source);
			strcpy(pr_pkt.parameter1, source);
			strcpy(pr_pkt.parameter2, input);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
		}
	}

	close(socket_id);
	return 0;
}
