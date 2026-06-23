#include "../headers.h"

int main(void)
{
    printf("--- Configuring local address ---\n");
    struct addrinfo hints, *bind_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "8080", &hints, &bind_address) != 0) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return 1;
    }

    printf("--- Creating socket ---\n");
    int server_socket = socket(bind_address->ai_family,
                               bind_address->ai_socktype,
                               bind_address->ai_protocol);
    if (server_socket < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }

    // Allow quick restart
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    printf("--- Binding socket ---\n");
    if (bind(server_socket, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return 1;
    }
    freeaddrinfo(bind_address);

    printf("--- Listening ---\n");
    if (listen(server_socket, 10) != 0) {
        fprintf(stderr, "listen() failed. (%d)\n", errno);
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(server_socket, &master);
    int max_socket = server_socket;

    printf("Waiting for connections on port 8080...\n");

    while (1) {
        fd_set read_fds = master;

        if (select(max_socket + 1, &read_fds, NULL, NULL, NULL) == -1) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            return 1;
        }

        for (int i = 0; i <= max_socket; ++i) {
            if (!FD_ISSET(i, &read_fds)) continue;

            if (i == server_socket) {
                // New connection
                struct sockaddr_storage client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);

                if (client < 0) {
                    fprintf(stderr, "accept() failed. (%d)\n", errno);
                    continue;
                }

                FD_SET(client, &master);
                if (client > max_socket) max_socket = client;

                char addr_buf[100] = {0};
                getnameinfo((struct sockaddr*)&client_addr, addr_len,
                            addr_buf, sizeof(addr_buf), NULL, 0, NI_NUMERICHOST);

                printf("Client connected from %s (fd=%d)\n", addr_buf, client);

                // Optional welcome message
                const char *welcome = "Welcome to the chat!\n";
                send(client, welcome, strlen(welcome), 0);

            } else {
                // Existing client
                char buffer[1024];
                int bytes = recv(i, buffer, sizeof(buffer), 0);

                if (bytes <= 0) {
                    if (bytes == 0)
                        printf("Client %d disconnected\n", i);
                    else
                        fprintf(stderr, "recv() failed on %d. (%d)\n", i, errno);

                    close(i);
                    FD_CLR(i, &master);
                } else {
                    printf("Received from %d: %.*s", i, bytes, buffer);

                    // Broadcast to all other clients
                    for (int j = 0; j <= max_socket; ++j) {
                        if (FD_ISSET(j, &master) && j != server_socket && j != i) {
                            send(j, buffer, bytes, 0);
                        }
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}