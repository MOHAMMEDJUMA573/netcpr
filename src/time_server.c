#include "headers.h"
/*App level libraries*/
#include <stdio.h>
#include <string.h>
#include <time.h>

int main()
{
    /*creating a bind address (local)*/
    printf("--- Configuring the local addresses ---\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    if ((getaddrinfo(0, "8080", &hints, &bind_address)) != 0) {
        fprintf(stderr, "getaddrinfo() failed.\t(%d)\n", errno);
        return 1;
    }
    /*creating a socket for the protocol type*/
    printf("--- Creating a socket ---\n");
    int server_socket;
    server_socket  = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (server_socket < 0) {
        fprintf(stderr, "socket() failed.\t(%d)\n", errno);
        return 1;
    }
    /* dual stack socket setup by removing ipv6_only default: i suspect its no longer default*/
    int option = 0;
    if((setsockopt(server_socket,IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option))) != 0) {
        fprintf(stderr, "setsockopt() failed.\t(%d)", errno);
        return 1;
    }
    /*binding the socket to the address*/
    printf("Binding a socket to *bind_address...\n");
    if ((bind(server_socket, bind_address->ai_addr, bind_address->ai_addrlen)) != 0) {
        fprintf(stderr, "bind() failed.\t(%d)\n", errno);
        return 1;
    }
    freeaddrinfo(bind_address);
    /*listen for connections*/
    printf("Listening for connections...\n");
    if ((listen(server_socket, 10)) != 0) {
        fprintf(stderr, "listen failed.\t(%d)\n", errno);
        return 1;
    }
    /*accept incoming connections*/
    printf("Waiting for connections...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int new_connected_socket =  accept(server_socket, (struct sockaddr*)&client_address, &client_len);
    if (new_connected_socket < 0) {
        fprintf(stderr, "accept() failed.\t(%d)\n", errno);
        return 1;
    }
    /*past here means a connection was accepted*/
    printf("---\tclient connected successfully\n");
    char address_buffer[100];
    if ((getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST)) != 0) {
        fprintf(stderr, "getnameinfo() failed.\t(%d)\n", errno);
        return 1;
    }
    printf("---\t%s(connected)\n",address_buffer);
    /*read the request...parse function in the future*/
    printf("---\tReading (%s)'s request\n",address_buffer);
    char request_buffer[1024];
    int bytes_recieved = recv(new_connected_socket, request_buffer, sizeof(request_buffer),0);
    if (bytes_recieved == -1){
        fprintf(stderr, "recv() failed.\t(%d)\n", errno);
        return 1;
    }else if (bytes_recieved == 0 ){
        fprintf(stderr, "---peer socket shutdown---\n");
        return 1;
    }else{
        printf("\t---\tRequest\t---\n%.*s\n\t---\t***\t---\n", bytes_recieved, request_buffer);
    }
    /*sending response*/
    printf("Sending response...\n");
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(new_connected_socket, response, strlen(response), 0);
    printf("---\t sent: %d (bytes) of (%d)\n", bytes_sent, (int)strlen(response));
    /*timer intergration send*/
    time_t timer;
    if ((time(&timer)) == (time_t)-1) {
        fprintf(stderr, "time() failed.\t(%d)\n", errno);
        return 1;
    }
    char *time_string = ctime(&timer);
    bytes_sent = send(new_connected_socket, time_string, strlen(time_string), 0);
    printf("---\t sent: %d (bytes) of (%d)\n", bytes_sent, (int)strlen(time_string));
    /*close the sockets both client and server*/
    printf("\nClosing client socket connection...\n");
    close(new_connected_socket);
    printf("Closing server's listening socket...\n");
    close(server_socket);

    printf("---***\t well done \t***---\n");

    return 0;
}