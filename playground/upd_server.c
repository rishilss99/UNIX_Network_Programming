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

#define PORT "23499"
#define localhost "127.0.0.1"

void bootUpMsg()
{
    printf("The ServerP is up and running using UDP on port %s\n", PORT);
}

int main(int argc, char *argv[])
{

    bootUpMsg();

    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;
    int status;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // use datagram sockets

    // error checking for getaddrinfo

    if ((status = getaddrinfo(localhost, PORT, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // error checking for socket creation

    if (sockfd == -1)
    {
        perror("ServerP: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("ServerP: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    // Trial code
    char buf[1024];
    struct sockaddr_in cliaddr;
    int addr_len = sizeof(cliaddr);
    int n;
    while(1){
        n = recvfrom(sockfd, buf, 1023, 0, (struct sockaddr *)&cliaddr, addr_len);
        buf[n] = '\0';
        printf("Client: \"%s\"\n", buf);
    }    
    close(sockfd);
    return 0;
}