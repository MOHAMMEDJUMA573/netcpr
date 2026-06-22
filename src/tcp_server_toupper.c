#include "headers.h"

int main()
{
    /*creating a bind address (local)*/
    printf("--- Configuring the local addresses ---\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    if (getaddrinfo(0, "8080", &hints, &bind_address) != 0) {
        fprintf(stderr, "getaddrinfo() failed.\t(%d)\n", errno);
        return 1;
    }
    /*creating a socket for the protocol type*/
    printf("--- Creating a socket ---\n");
    int server_socket;
    server_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (server_socket < 0) {
        fprintf(stderr, "socket() failed.\t(%d)\n", errno);
        return 1;
    }
    /*binding the socket to the address*/
    printf("Binding a socket to *bind_address...\n");
    if (bind(server_socket, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
        fprintf(stderr, "bind() failed.\t(%d)\n", errno);
        return 1;
    }
    freeaddrinfo(bind_address);
    /*listen for connections*/
    printf("Listening for connections...\n");
    if (listen(server_socket, 10) != 0) {
        fprintf(stderr, "listen() failed.\t(%d)\n", errno);
        return 1;
    }
    /*a set for the sockets file descriptor*/
    fd_set master;
    FD_ZERO(&master);
    FD_SET(server_socket, &master);
    int max_socket = server_socket;
    /*accept incoming connections*/
    printf("Waiting for connections...\n");
    while(1) {
        fd_set read_fds;
        read_fds = master;
        if (select(max_socket+1, &read_fds, 0, 0, 0) == -1) {
            fprintf(stderr, "select() failed.\t(%d)\n", errno);
            return 1;
        }
        int i;
        for (i = 0; i <= max_socket; i++) {
            if (FD_ISSET(i, &read_fds)) {
                //handle the socket
                if (i == server_socket) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    int new_connected_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
                    if (new_connected_socket < 0) {
                        fprintf(stderr, "accept() failed.\t(%d)\n", errno);
                        return 1;
                    }
                    FD_SET(new_connected_socket, &master);
                    if (new_connected_socket > max_socket) {
                        max_socket = new_connected_socket;
                    }
                    /*past here means a connection was accepted*/
                    printf("---\tclient connected successfully\n");
                    char address_buffer[100];
                    if (getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST) != 0) {
                        fprintf(stderr, "getnameinfo() failed.\t(%d)\n", errno);
                        return 1;
                    }
                    printf("---\t(new connecction from)--- %s --- ...(connected)\n",address_buffer);
                    /*read the request...parse function in the future*/
                }else {
                    printf("---\t IN THE READ SECTION \n");
                    char read_buffer[1024];
                    int bytes_recieved = recv(i, read_buffer, sizeof(read_buffer), 0);
                    if (bytes_recieved == 0) {
                        fprintf(stderr, "---peer socket shutdown---\n");
                        FD_CLR(i, &master);
                        close(i);
                        continue;
                    }else if(bytes_recieved == -1) {
                        fprintf(stderr, "recv() failed.\t(%d)\n", errno);
                        return 1;
                    }
                    printf("\t---\tServer Recieved\t---\n%.*s\n\t---\t***\t---\n", bytes_recieved, read_buffer);
                    //perform the operation and send back a response
                    int j;
                    for (j=0; j<bytes_recieved; ++j) {
                        read_buffer[j] = toupper(read_buffer[j]);
                    }
                    send(i, read_buffer, bytes_recieved, 0);
                }
            }
        }
    }
    printf("Closing server's listening socket...\n");
    close(server_socket);
    printf("---***\t well done \t***---\n");

    return 0;
}