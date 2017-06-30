/*
 * server.c
 *
 *  Created on: Mar 15, 2017
 *      Author: songshu3
 */

#include "server_lib.h"

#define BACKLOG 10 // how many pending connections queue will hold

int main(void) {
    fd_set master, read_fds;
    int fd_max;
    int socket_id, new_socket_id; // listen on sock_fd, new connection on new_fd

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;

    struct packet pkt;
    struct processed_packet pr_pkt;

    char s[INET6_ADDRSTRLEN], buf[MAX_DATA];
    int i, numbytes;

    struct client_list list[10];
    create_client_list(list, 10);

    pthread_t time_out;
    pthread_create(&time_out, NULL, kick_out, list);

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    socket_id = create_stream_socket("5580");
    assert(socket_id != 0);


    assert(listen(socket_id, BACKLOG) >= 0);
    FD_SET(socket_id, &master);
    fd_max = socket_id;

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    assert(sigaction(SIGCHLD, &sa, NULL) >= 0);

    printf("Server: Waiting for connections...\n");
    while (1) {
        read_fds = master; // copy it
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for (i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == socket_id) {
                    sin_size = sizeof their_addr;
                    new_socket_id = accept(socket_id, (struct sockaddr *) &their_addr, &sin_size);
                    assert(new_socket_id != -1);
                    FD_SET(new_socket_id, &master);
                    if (new_socket_id > fd_max)
                        fd_max = new_socket_id;
                    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
                    printf("Server: Got connection from %s\n", s);
                    handle_auth(new_socket_id, list);
                } else {
                    numbytes = recv(i, buf, sizeof buf, 0);
                    printf("%d, %s \n", i, buf);
                    if (numbytes <= 0) {
                        if (numbytes == 0)
                            printf("selectserver: socket %d hung up\n", i);
                        else
                            perror("recv");
                        close(i);
                        FD_CLR(i, &master); // remove from master set
                    } else {
                    	handle_request(socket_id, list, buf);
                    }
                }
            }
        }
    }

    return 0;
}
