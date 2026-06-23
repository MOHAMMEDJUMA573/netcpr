#include "../headers.h"

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s hostname port username\n", argv[0]);
        return 1;
    }

    char username[32];
    strncpy(username, argv[3], sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';

    printf("\t\t\t --- Username: %s ---\n", username);

    /* Configure remote address */
    printf("--- Configuring remote address ---\n");
    struct addrinfo hints, *server_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], argv[2], &hints, &server_address) != 0) {
        fprintf(stderr, "getaddrinfo() failed\n");
        return 1;
    }

    /* Show resolved address */
    char addr_buf[100] = {0};
    char port_buf[20] = {0};
    getnameinfo(server_address->ai_addr, server_address->ai_addrlen,
                addr_buf, sizeof(addr_buf),
                port_buf, sizeof(port_buf),
                NI_NUMERICHOST | NI_NUMERICSERV);

    printf("--- Connecting to %s:%s ---\n", addr_buf, port_buf);

    /* Create socket */
    int client_socket = socket(server_address->ai_family,
                               server_address->ai_socktype,
                               server_address->ai_protocol);
    if (client_socket == -1) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }

    /* Connect */
    printf("--- Connecting...\n");
    if (connect(client_socket, server_address->ai_addr, server_address->ai_addrlen) == -1) {
        fprintf(stderr, "connect() failed. (%d)\n", errno);
        return 1;
    }

    freeaddrinfo(server_address);
    printf("--- Successfully connected! Type messages and press Enter ---\n\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval timeout = { .tv_sec = 0, .tv_usec = 100000 }; // 100ms

        if (select(client_socket + 1, &readfds, NULL, NULL, &timeout) == -1) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            break;
        }

        /* Data from server */
        if (FD_ISSET(client_socket, &readfds)) {
            char read_buffer[4096];
            int bytes_received = recv(client_socket, read_buffer, sizeof(read_buffer) - 1, 0);

            if (bytes_received < 1) {
                printf("--- Server disconnected ---\n");
                break;
            }

            read_buffer[bytes_received] = '\0';
            printf("%s", read_buffer);   // Server already sends formatted messages
        }

        /* Input from user (stdin) */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char input[4096];
            if (!fgets(input, sizeof(input), stdin)) {
                break;  // EOF or error
            }

            // Build message: "username : message"
            char message[4200];
            snprintf(message, sizeof(message), "%s : %s", username, input);

            int bytes_sent = send(client_socket, message, strlen(message), 0);
            if (bytes_sent == -1) {
                fprintf(stderr, "send() failed. (%d)\n", errno);
                break;
            }
        }
    }

    printf("Closing connection...\n");
    close(client_socket);
    printf("--- Goodbye! ---\n");
    return 0;
}