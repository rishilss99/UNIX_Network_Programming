#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BACKLOG 5
#define localhost "127.0.0.1"
#define TCP_PORT_A "25499"
#define TCP_PORT_B "26499"

void bootUpMsg(){
    printf("The Central server is up and running.\n");
}

int main(){
    bootUpMsg();

    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;
    int status;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_STREAM;  // use datagram sockets

    // error checking for getaddrinfo

    if ((status = getaddrinfo(localhost, TCP_PORT_A, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // error checking for socket creation

    if (sockfd == -1)
    {
        perror("ServerT: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("ServerT: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure


    //-----------------Literally just for trial-----------------------

    if (listen(sockfd, BACKLOG) == -1) {
        close(sockfd);
        perror("listen");
        exit(1);
    }

    printf("Central: waiting for connections...\n");
    int child_fd;
    socklen_t addr_len;
    struct sockaddr_in client_addr;
    char ip4[INET_ADDRSTRLEN];
    while(1) { // main accept() loop
        addr_len = sizeof client_addr;
        child_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (child_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(AF_INET, &client_addr, ip4, INET_ADDRSTRLEN);
        printf("server: got connection from %s\n", ip4);
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(child_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(child_fd);
            exit(0);
        }
        close(child_fd);
    }
    close(sockfd);
}