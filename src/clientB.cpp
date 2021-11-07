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

#define localhost "127.0.0.1"
#define CENTRAL_PORT_B "26499"  //TCP Port to connect to Central Server

void bootUpMsg(){
    printf("The client is up and running.\n");
}

int main(int argc, char *argv[]){
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
    // getaddrinfo used to get server address

    if ((status = getaddrinfo(localhost, CENTRAL_PORT_B, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket to communicate with server

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // error checking for socket creation

    if (sockfd == -1)
    {
        perror("Client B: socket");
        exit(1);
    }


    // Directly connecting to central server without binding
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        close(sockfd);
        perror("Client B: connect");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    // Established TCP connection, send user A name to Central server
    if (send(sockfd, argv[1], strlen(argv[1]), 0) == -1){
        perror("Client B: send");
        exit(1);
    }
    else
        printf("The client sent %s to the Central server.\n",argv[1]);

    // Upto this point client is setup and it has sent the username to the central server - Phase 1A
    int numbytes;
    char buf[20];
    if ((numbytes = recv(sockfd, buf, 19, 0)) == -1)
    {
        perror("Client B: recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("Client B: received '%s'\n", buf);

    close(sockfd);

}