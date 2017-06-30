/*
 * server_lib.c
 *
 *  Created on: Mar 16, 2017
 *      Author: songshu3
 */

#include "server_lib.h"

void create_client_list(struct client_list *list, int size) {
	int i;
	char name[MAX_NAME];
	for (i = 0; i < size; i++) {
		sprintf(name, "%c", i + 'a');
		strcpy(list[i].username, name);
		list[i].state = 0;
		list[i].session = 0;
		list[i].socket = 0;
		pthread_mutex_init(&list[i].lock, NULL);
	}
}

int create_stream_socket(char *port) {
	int socket_id;
	struct addrinfo new, *server_info, *p;
	int yes = 1;

	memset(&new, 0, sizeof new);
	new.ai_family = AF_UNSPEC;
	new.ai_socktype = SOCK_STREAM;
	new.ai_flags = AI_PASSIVE;

	assert(getaddrinfo(NULL, port, &new, &server_info) == 0);

	// loop through all the results and bind to the first we can
	for (p = server_info; p != NULL; p = p->ai_next) {
		if ((socket_id = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			printf(
					"Server: socket failed, socket_id = %d, checking next one. \n",
					socket_id);
			continue;
		}
		if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			printf("Server: setsocket failed, exiting. \n");
			exit(1);
		}
		if (bind(socket_id, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_id);
			printf("Server: Binding socket failed, checking next one. \n");
			continue;
		}
		break;
	}
	freeaddrinfo(server_info); // all done with this structure
	if (p == NULL) {
		printf("Server: Binding socket failed, exiting \n");
		exit(1);
	}

	return socket_id;
}

int handle_auth(int socket_id, struct client_list *list) {
	int numbytes;
	char buf[MAX_DATA];
	char temp[MAX_NAME];
	struct packet pkt;
	struct processed_packet pr_pkt;

	numbytes = recv(socket_id, buf, MAX_DATA - 1, 0);
	if (numbytes < 0)
		return -1;
	buf[numbytes] = '\0';
	parse_packet(buf, &pkt, 1);
	process_data(&pkt, &pr_pkt, 1);

	int i;
	if (pr_pkt.type == LOGIN) {
		if (strcmp(pr_pkt.parameter1, pr_pkt.parameter2) == 0) {
			for (i = 0; i < 10; i++)
				if (strcmp(list[i].username, pr_pkt.source) == 0) {
					printf("handle auth lock \n");
					pthread_mutex_lock(&list[i].lock);
					list[i].state = 1;
					list[i].socket = socket_id;
					list[i].last_active = clock();
					printf("Last Active: %d \n", list[i].last_active);
					pthread_mutex_unlock(&list[i].lock);
					printf("handle auth unlock \n");
					break;
				}
			pr_pkt.type = LO_ACK;

			sprintf(temp, "%d", socket_id);
			strcpy(pr_pkt.source, temp);
			process_data(&pkt, &pr_pkt, 2);
			parse_packet(buf, &pkt, 2);
			send(socket_id, buf, strlen(buf), 0);
			return 0;
		}
	}
	pr_pkt.type = LO_NAK;
	strcpy(pr_pkt.source, temp);
	process_data(&pkt, &pr_pkt, 2);
	parse_packet(buf, &pkt, 2);
	send(socket_id, buf, strlen(buf), 0);
	return 0;
}

int handle_request(int socket_id, struct client_list *list, char *buf) {
	struct packet pkt;
	struct processed_packet pr_pkt;

	char temp[MAX_NAME], temp2[MAX_NAME];
	int j, k;

	parse_packet(buf, &pkt, 1);
	process_data(&pkt, &pr_pkt, 1);

	for (k = 0; k < 10; k++) {
		if (strcmp(list[k].username, pr_pkt.source) == 0) {
			break;
		}
	}
	printf("last active lock \n");
	pthread_mutex_lock(&list[k].lock);
	printf("last active locked \n");
	list[k].last_active = clock();
	printf("Last Active: %d \n", list[k].last_active);
	pthread_mutex_unlock(&list[k].lock);
	printf("last active unlock \n");

	if (pr_pkt.type == EXIT) {
		for (j = 0; j < 10; j++)
			if (strcmp(list[j].username, pr_pkt.source) == 0) {
				list[j].state = 0;
				printf("state: %d \n", list[j].state);
			}
	} else if (pr_pkt.type == NEW_SESS) {
		for (j = 0; j < 10; j++) {
			if (strcmp(list[j].username, pr_pkt.source) == 0) {
				pthread_mutex_lock(&list[j].lock);
				list[j].session = j + 1;
				printf("session: %d \n", list[j].session);
				pthread_mutex_unlock(&list[j].lock);
				break;
			}
		}
		sprintf(temp, "%d", socket_id);
		strcpy(pr_pkt.source, temp);
		pr_pkt.type = NS_ACK;
		sprintf(temp, "%d", list[j].session);
		strcpy(pr_pkt.parameter1, temp);
		process_data(&pkt, &pr_pkt, 2);
		parse_packet(buf, &pkt, 2);
		send(list[j].socket, buf, strlen(buf), 0);
	} else if (pr_pkt.type == JOIN) {
		for (j = 0; j < 10; j++) {
			pthread_mutex_lock(&list[j].lock);
			if (list[j].session == atoi(pr_pkt.parameter1)) {
				pthread_mutex_unlock(&list[j].lock);
				break;
			} else {
				pthread_mutex_unlock(&list[j].lock);
			}
		}
		if (j != 10) {
			for (j = 0; j < 10; j++) {
				pthread_mutex_lock(&list[j].lock);
				if (strcmp(list[j].username, pr_pkt.source) == 0) {
					list[j].session = atoi(pr_pkt.parameter1);
					pthread_mutex_unlock(&list[j].lock);
					break;
				} else {
					pthread_mutex_unlock(&list[j].lock);
				}

			}
			sprintf(temp, "%d", socket_id);
			strcpy(pr_pkt.source, temp);
			pr_pkt.type = JN_ACK;
			pthread_mutex_lock(&list[j].lock);
			sprintf(temp, "%d", list[j].session);
			pthread_mutex_unlock(&list[j].lock);
			strcpy(pr_pkt.parameter1, temp);
		} else {
			for (j = 0; j < 10; j++) {
				if (strcmp(list[j].username, pr_pkt.source) == 0)
					break;
			}
			sprintf(temp, "%d", socket_id);
			strcpy(pr_pkt.source, temp);
			pr_pkt.type = JN_NAK;
			sprintf(temp, "%d", j);
			strcpy(pr_pkt.parameter1, temp);
		}
		process_data(&pkt, &pr_pkt, 2);
		parse_packet(buf, &pkt, 2);
		send(list[j].socket, buf, strlen(buf), 0);
	} else if (pr_pkt.type == LEAVE_SESS) {
		for (j = 0; j < 10; j++) {
			pthread_mutex_lock(&list[j].lock);
			if (strcmp(list[j].username, pr_pkt.source) == 0) {
				list[j].session = 0;
			}
			pthread_mutex_unlock(&list[j].lock);
		}
		printf("%d, %d \n", j, list[j].session);
		sprintf(temp, "%d", socket_id);
		strcpy(pr_pkt.source, temp);
		pr_pkt.type = LS_ACK;
		sprintf(temp, "%d", j);
		strcpy(pr_pkt.parameter1, temp);
		process_data(&pkt, &pr_pkt, 2);
		parse_packet(buf, &pkt, 2);
		send(list[j].socket, buf, strlen(buf), 0);
	} else if (pr_pkt.type == QUERY) {
		pr_pkt.type = QU_ACK;
		sprintf(temp, "Users:");
		for (j = 0; j < 10; j++) {
			pthread_mutex_lock(&list[j].lock);
			if (list[j].state == 1) {
				sprintf(temp2, "%s,", list[j].username);
				strcat(temp, temp2);
			}
			pthread_mutex_unlock(&list[j].lock);
		}
		sprintf(temp2, ".Sessions:");
		strcat(temp, temp2);
		for (j = 0; j < 10; j++) {
			pthread_mutex_lock(&list[j].lock);
			if ((list[j].session > 0) && (list[j].session < 11)) {
				sprintf(temp2, "%d,", list[j].session);
				strcat(temp, temp2);
			}
			pthread_mutex_unlock(&list[j].lock);
		}
		for (j = 0; j < 10; j++) {
			if (strcmp(list[j].username, pr_pkt.source) == 0)
				break;
		}
		strcpy(pr_pkt.parameter1, temp);
		sprintf(temp, "%d", socket_id);
		strcpy(pr_pkt.source, temp);
		pr_pkt.type = QUERY_R;
		process_data(&pkt, &pr_pkt, 2);
		parse_packet(buf, &pkt, 2);
		send(list[j].socket, buf, strlen(buf), 0);
		printf("%s \n", buf);
	} else if (pr_pkt.type == DATA) {
		for (k = 0; k < 10; k++) {
			if (strcmp(list[k].username, pr_pkt.source) == 0) {
				break;
			}
		}
		sprintf(temp, "%d", socket_id);
		strcpy(pr_pkt.source, temp);
		pr_pkt.type = DATA;
		process_data(&pkt, &pr_pkt, 2);
		parse_packet(buf, &pkt, 2);
		for (j = 0; j < 10; j++) {
			pthread_mutex_lock(&list[j].lock);
			if ((j != k) && (list[j].session == list[k].session))
				send(list[j].socket, buf, strlen(buf), 0);
			pthread_mutex_unlock(&list[j].lock);
		}
	} else if(pr_pkt.type == INVITE) {

	}
	return 0;
}

void kick_out(struct client_list *list) {

	struct packet pkt;
	struct processed_packet pr_pkt;

	char buf[MAX_DATA];
	int i;
	while(1) {
		for (i = 0; i < 10; i++ ) {
			//printf("kick out lock \n");
			pthread_mutex_lock(&list[i].lock);
			//printf("kick out locked \n");
			if((list[i].state == 1) && ((double)(clock() - list[i].last_active) / CLOCKS_PER_SEC) > TIME_OUT) {
				printf("%f \n", (double)(clock() - list[i].last_active) / CLOCKS_PER_SEC);
				list[i].state = 0;
				printf("kicking out %d \n", i);
				pr_pkt.type = TOUT;
				process_data(&pkt, &pr_pkt, 2);
				parse_packet(buf, &pkt, 2);
				send(list[i].socket, buf, strlen(buf), 0);
			}
			pthread_mutex_unlock(&list[i].lock);
			//printf("kick out unlock \n");
		}
	}
}

