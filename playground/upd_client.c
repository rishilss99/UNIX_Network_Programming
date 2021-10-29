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

#define SERVERPORT "23499"
#define localhost "127.0.0.1"

void bootUpMsg()
{
    printf("The Trial Client is up and running.\n");
}

// /*Retrieve the locally-bound name of the specified socket and
// store it in the sockaddr structure*/
// Getsock_check=getsockname(TCP_Connect_Sock,(struct sockaddr
// *)&my_addr, (socklen_t *)&addrlen);
// //Error checking
// if (getsock_check== -1) {
// perror("getsockname");
// exit(1);
// }

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
    // getaddrinfo used to get server address

    if ((status = getaddrinfo(localhost, SERVERPORT, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket to communicate with server

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // error checking for socket creation

    if (sockfd == -1)
    {
        perror("ServerP: socket");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure
}