#include "headers.h"

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port.\n");
        return 1;
    }
    /*configure the remote address*/
    printf("---\t configuring the remote address \t ---\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *server_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &server_address) != 0) {
        fprintf(stderr, "getaddrinfo() failed.\n");
        return 1;
    }
    /*print the string adress on success*/
    char address_buffer[100];
    char service_buffer[100];
    if (getnameinfo(server_address->ai_addr, server_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST) != 0) {
        fprintf(stderr, "getnameinfo() failed.\n");
        return 1;
    }
    printf("---\t host:%s \tport:%s \t---\n",address_buffer, service_buffer);
    /*create an endpoint for connection*/
    printf("---\t creating a socket \t---\n");
    int client_socket;
    client_socket = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
    if (client_socket == -1) {
        fprintf(stderr, "socket() failed.\t(%d)\n", errno);
        return 1;
    }
    /*establish a connection to the remote server*/
    printf("---\t connecting ...\n");
    if (connect(client_socket, server_address->ai_addr, server_address->ai_addrlen) == -1) {
        fprintf(stderr, "connect() failed.\t(%d)\n", errno);
        return 1;
    }
    freeaddrinfo(server_address);
    /*past here? connected; show commands()*/
    printf("--- (connected!) TO SEND DATA ENTER TEXT FOLLOWED BY ENTER\n---");
    while(1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        FD_SET(0, &readfds);

        struct timeval timemout;
        timemout.tv_sec = 0;
        timemout.tv_usec = 100000;

        if (select(client_socket+1, &readfds, 0, 0, &timemout) == -1) {
            fprintf(stderr, "select() failed.\t(%d)\n", errno);
            return 1;
        }
        if (FD_ISSET(client_socket, &readfds)) {
            char read_buffer[4096];
            int bytes_recieved = recv(client_socket, read_buffer, sizeof(read_buffer), 0);
            if (bytes_recieved < 1){
                fprintf(stderr, "---peer connection shutdown---\n");
                break;
            }
            printf("\t---\tRequest\t---\n%.*s\n\t---\t***\t---\n", bytes_recieved, read_buffer);
        }
        if (FD_ISSET(0, &readfds)) {
            char read_buffer[4096];
            if (!fgets(read_buffer, sizeof(read_buffer), stdin)) break;
            printf("---\t sending \t---\n%s\n---\t sending \t---\n", read_buffer);
            int bytes_sent = send(client_socket, read_buffer, strlen(read_buffer), 0);
            if (bytes_sent == -1) {
                fprintf(stderr, "send() failed.\t(%d)\n", errno);
                return 1;
            }
            printf("---\t sent: %d (bytes) of (%d)\n", bytes_sent, (int)strlen(read_buffer));
        }
    }
    printf("closing socket ...\n");
    close(client_socket);
    printf("---***\t well done \t***---\n");


    return 0;
}