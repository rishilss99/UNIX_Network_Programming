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

// More than what is specified in project details
#include <poll.h>

#define BACKLOG 5
#define localhost "127.0.0.1"
#define TCP_PORT_A "25499"
#define TCP_PORT_B "26499"

void bootUpMsg()
{
    printf("The Central server is up and running.\n");
}

// Socket setup, bind and listen
int setupSocket(const char *port)
{
    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;
    int status;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_STREAM; // use datagram sockets

    // error checking for getaddrinfo

    if ((status = getaddrinfo(localhost, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // error checking for socket creation

    if (sockfd == -1)
    {
        perror("Central: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("Central: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1)
    {
        close(sockfd);
        perror("Central: listen");
        exit(1);
    }

    return sockfd;
}

int getPortNumber(int socket_fd){
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr*)&sin, &sinlen);
    return ntohs(sin.sin_port);
}

int main()
{
    bootUpMsg();

    int sockfd_A = setupSocket(TCP_PORT_A);
    int sockfd_B = setupSocket(TCP_PORT_B);

    // Polling for multiple sockets

    int fd_count = 2;
    struct pollfd pfds[fd_count];

    pfds[0].fd = sockfd_A;
    pfds[0].events = POLLIN | POLLOUT; // Report ready to read on incoming connection
    // pfds[0].events = POLLIN | POLLOUT; // Use this for receiving as well as sending

    pfds[1].fd = sockfd_B;
    pfds[1].events = POLLIN | POLLOUT; // Report ready to read on incoming connection
    // pfds[1].events = POLLIN | POLLOUT; // Use this for receiving as well as sending

    // printf("Central: waiting for connections...\n");

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int child_fd;
    char ip4[INET_ADDRSTRLEN];

    while (1)
    {
        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1)
        {
            perror("Central: poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                child_fd = accept(pfds[i].fd, (struct sockaddr *)&client_addr, &addr_len);
                if (child_fd == -1)
                {
                    perror("Central: accept");
                    exit(1);
                }
                int numbytes;
                char buffer[512];
                if ((numbytes = recv(child_fd, buffer, sizeof buffer, 0)) == -1)
                {
                    perror("Server: recv");
                    exit(1);
                }
                buffer[numbytes] = '\0';
                
                printf("The Central server received input=%s from the client using TCP over port %d\n", buffer, getPortNumber(pfds[i].fd));

                if (send(child_fd, "Hey", 3, 0) == -1)
                {
                    perror("Client B: send");
                    exit(1);
                }

                close(child_fd);
            }
        }
    }
}