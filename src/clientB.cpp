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
#define CENTRAL_PORT_B "26499" //TCP Port to connect to Central Server

#include <string>
#include <iostream>

#define MAX_BUF_LEN 30000

void BootUpMsg()
{
    printf("The client is up and running.\n");
}

int main(int argc, char *argv[])
{
    BootUpMsg();

    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;
    int status;
    int numbytes;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_STREAM; // use datagram sockets

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
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0)
    {
        close(sockfd);
        perror("Client B: connect");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    int *num_usernames = new int(argc - 1);

    if ((numbytes = send(sockfd, num_usernames, sizeof(int), 0)) == -1)
    {
        perror("ClientB: send");
        exit(1);
    }

    // Main case when only single username provided by client B
    if (argc == 2)
    {
        // Established TCP connection, send user A name to Central server
        if ((numbytes = send(sockfd, argv[1], strlen(argv[1]), 0)) == -1)
        {
            perror("Client B: send");
            exit(1);
        }
        else
            printf("The client sent %s to the Central server.\n", argv[1]);
    }

    // Optional case when two userames provided by client B
    else
    {
        // Established TCP connection, send user A name to Central server
        if ((numbytes = send(sockfd, argv[1], strlen(argv[1]), 0)) == -1)
        {
            perror("Client B: send");
            exit(1);
        }
        if ((numbytes = send(sockfd, argv[2], strlen(argv[2]), 0)) == -1)
        {
            perror("Client B: send");
            exit(1);
        }
        else
            printf("The client sent %s and %s to the Central server.\n", argv[1],argv[2]);
    }

    // Upto this point client is setup and it has sent the username to the central server - Phase 1A

    std::string clientB_output;
    int *string_length = new int(0);

    if ((numbytes = recv(sockfd, string_length, sizeof(int), 0)) == -1)
    {
        perror("ClientA: recv string length");
        exit(1);
    }

    int string_size = *string_length;

    if (string_size < MAX_BUF_LEN)
    {
        char clientB_str[MAX_BUF_LEN];
        if ((numbytes = recv(sockfd, clientB_str, sizeof clientB_str, 0)) == -1)
        {
            perror("ClientA: recv clientA str");
            exit(1);
        }
        clientB_output.append(clientB_str);
    }
    else
    {
        char clientB_str[MAX_BUF_LEN + 10];
        while (string_size > 0)
        {

            if ((numbytes = recv(sockfd, clientB_str, sizeof clientB_str, 0)) == -1)
            {
                perror("Central: ServerP recvfrom clientA str");
                exit(1);
            }
            clientB_output.append(clientB_str);
            string_size -= MAX_BUF_LEN;
        }
    }

    std::cout << clientB_output;

    // Freeing allocated dynamic memory
    delete string_length;
    delete num_usernames;

    close(sockfd);
}