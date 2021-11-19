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

#define BACKLOG 5
#define localhost "127.0.0.1"
#define TCP_PORT_A "25499"
#define TCP_PORT_B "26499"
#define UDP_PORT "24499"
#define SERVER_T_PORT "21499"
#define SERVER_S_PORT "22499"
#define SERVER_P_PORT "23499"

void bootUpMsg()
{
    printf("The Central server is up and running.\n");
}

// Socket setup, bind and listen
int setupTCPSocket(const char *port)
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
        perror("Central TCP: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("Central TCP: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1)
    {
        close(sockfd);
        perror("Central TCP: listen");
        exit(1);
    }

    return sockfd;
}

int setupUDPSocket(const char *port)
{
    struct addrinfo hints;
    struct addrinfo *res;
    int sockfd;
    int status;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // use datagram sockets

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
        perror("Central UDP: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("Central UDP: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure
    return sockfd;
}

int getPortNumber(int socket_fd)
{
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr *)&sin, &sinlen);
    return ntohs(sin.sin_port);
}

void getTopologyServerT(int sockfd, char user_name_A[], char user_name_B[], int *&num_nodes_ptr, int *&nodes_list_ptr, int **&adjacency_matrix_ptr)
{

    struct addrinfo hints;
    struct addrinfo *servinfo;
    int status;
    int numbytes;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // use datagram sockets

    // error checking for getaddrinfo
    // getaddrinfo used to get server address

    if ((status = getaddrinfo(localhost, SERVER_T_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // First Send user_name_A

    if ((numbytes = sendto(sockfd, user_name_A, strlen(user_name_A), 0,
                           servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: Server T sendto");
        exit(1);
    }

    // Second Send user_name_B

    if ((numbytes = sendto(sockfd, user_name_B, strlen(user_name_B), 0,
                           servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: Server T sendto");
        exit(1);
    }

    printf("The Central server sent a request to Backend-Server T.\n");

    int *num_nodes = new int(0);

    if ((numbytes = recvfrom(sockfd, num_nodes, sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: Server T recvfrom num nodes");
        exit(1);
    }

    //------------------------------------------Debug Output Code--------------------------------------------------------------
    // printf("%d\n",*num_nodes);
    //------------------------------------------Debug Output Code--------------------------------------------------------------

    int nodes_list_size = *num_nodes;
    int *nodes_list = new int[nodes_list_size];

    if ((numbytes = recvfrom(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: Server T recvfrom nodes list");
        exit(1);
    }

    //------------------------------------------Debug Output Code--------------------------------------------------------------
    // for(int i = 0; i<nodes_list_size; i++)
    //     printf("%d ",nodes_list[i]);
    //------------------------------------------Debug Output Code--------------------------------------------------------------

    int **adjacency_matrix = new int *[nodes_list_size];

    for (int i = 0; i < nodes_list_size; i++)
    {
        adjacency_matrix[i] = new int[nodes_list_size];
        if ((numbytes = recvfrom(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
        {
            perror("Central: Server T recvfrom adjacency matrix");
            exit(1);
        }

        //------------------------------------------Debug Output Code--------------------------------------------------------------
        // for (int k = 0; k < nodes_list_size; k++)
        //     printf("%d ", adj_matrix[i][k]);
        // printf("\n");
        //------------------------------------------Debug Output Code--------------------------------------------------------------
    }

    // Transferring the data reference to the pointers from main

    num_nodes_ptr = num_nodes;
    nodes_list_ptr = nodes_list;
    adjacency_matrix_ptr = adjacency_matrix;

    freeaddrinfo(servinfo);
}

void getScoresServerS(int sockfd, int *&num_nodes_ptr, int *&nodes_list_ptr, int *&scores_list_ptr)
{

    struct addrinfo hints;
    struct addrinfo *servinfo;
    int status;
    int numbytes;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // use datagram sockets

    // error checking for getaddrinfo
    // getaddrinfo used to get server address

    if ((status = getaddrinfo(localhost, SERVER_S_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int *num_nodes = num_nodes_ptr;
    int *nodes_list = nodes_list_ptr;
    int nodes_list_size = *num_nodes;

    // First Send num_nodes

    if ((numbytes = sendto(sockfd, num_nodes_ptr, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS sendto num nodes");
        exit(1);
    }

    // Second Send node list

    if ((numbytes = sendto(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS sendto nodes list");
        exit(1);
    }

    printf("The Central server sent a request to Backend-Server S.\n");

    int *scores_list = new int[nodes_list_size];

    if ((numbytes = recvfrom(sockfd, scores_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS recvfrom nodes list");
        exit(1);
    }

    //------------------------------------------Debug Output Code--------------------------------------------------------------
    // for(int i = 0; i<nodes_list_size; i++)
    //     printf("%d ",nodes_list[i]);
    //------------------------------------------Debug Output Code--------------------------------------------------------------

    // Transferring the data reference to the pointers from main

    scores_list_ptr = scores_list;

    freeaddrinfo(servinfo);
}

void getCompatibilityServerP(int sockfd, int *&num_nodes_ptr, int *&scores_list_ptr, int **&adjacency_matrix_ptr)
{
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int status;
    int numbytes;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints); // make sure hints is empty
    hints.ai_family = AF_INET;       // use IPv4
    hints.ai_socktype = SOCK_DGRAM;  // use datagram sockets

    // error checking for getaddrinfo
    // getaddrinfo used to get server address

    if ((status = getaddrinfo(localhost, SERVER_P_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int *num_nodes = num_nodes_ptr;
    int *scores_list = scores_list_ptr;
    int **adjacency_matrix = adjacency_matrix_ptr;
    int nodes_list_size = *num_nodes;

    // First Send num_nodes

    if ((numbytes = sendto(sockfd, num_nodes_ptr, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: Central ServerP sendto num nodes");
        exit(1);
    }

    // Second Send node list

    if ((numbytes = sendto(sockfd, scores_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP sendto scores list");
        exit(1);
    }

    // Third Send adjacency matrix

    for (int i = 0; i < nodes_list_size; i++)
    {
        if ((numbytes = sendto(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerP sendto adjacency matrix");
            exit(1);
        }
    }

    printf("The Central server sent a processing request to Backend-Server P.\n");

    freeaddrinfo(servinfo);
}

int main()
{
    bootUpMsg();

    int sockfd_A = setupTCPSocket(TCP_PORT_A);
    int sockfd_B = setupTCPSocket(TCP_PORT_B);
    int sockfd_udp = setupUDPSocket(UDP_PORT);

    struct sockaddr_in client_addr_A, client_addr_B;
    socklen_t addr_len = sizeof(client_addr_A);
    int child_fd_A, child_fd_B;
    char user_name_A[512];
    char user_name_B[512];
    int numbytes;

    while (1)
    {

        child_fd_A = accept(sockfd_A, (struct sockaddr *)&client_addr_A, &addr_len);
        if (child_fd_A == -1)
        {
            perror("Central: client A accept");
            exit(1);
        }

        if ((numbytes = recv(child_fd_A, user_name_A, sizeof user_name_A, 0)) == -1)
        {
            perror("Central: client A recv");
            exit(1);
        }
        user_name_A[numbytes] = '\0';

        printf("The Central server received input=%s from the client using TCP over port %d.\n", user_name_A, getPortNumber(sockfd_A));

        child_fd_B = accept(sockfd_B, (struct sockaddr *)&client_addr_B, &addr_len);
        if (child_fd_B == -1)
        {
            perror("Central: client B accept");
            exit(1);
        }
        if ((numbytes = recv(child_fd_B, user_name_B, sizeof user_name_B, 0)) == -1)
        {
            perror("Central: client B recv");
            exit(1);
        }
        user_name_B[numbytes] = '\0';

        printf("The Central server received input=%s from the client using TCP over port %d.\n", user_name_B, getPortNumber(sockfd_B));

        // Done till this portion - Central server receiver usernames from both clients

        int *num_nodes;
        int *nodes_list;
        int **adjacency_matrix;

        getTopologyServerT(sockfd_udp, user_name_A, user_name_B, num_nodes, nodes_list, adjacency_matrix);

        // ------------------------------------------Debug Output Code--------------------------------------------------------------
        for (int i = 0; i < *num_nodes; i++)
        {
            for (int k = 0; k < *num_nodes; k++)
            {
                printf("%d ", adjacency_matrix[i][k]);
            }
            printf("\n");
        }
        // ------------------------------------------Debug Output Code--------------------------------------------------------------

        printf("The Central server received information from Backend-Server T using UDP over port %d.\n", getPortNumber(sockfd_udp));

        // IMPORTANT: Handle the corner case when the none of the given usernames exist in edgelist.txt as this will give a NULL matrix

        int *scores_list;

        getScoresServerS(sockfd_udp, num_nodes, nodes_list, scores_list);

        // ------------------------------------------Debug Output Code--------------------------------------------------------------
        for (int k = 0; k < *num_nodes; k++)
        {
            printf("%d ", scores_list[k]);
        }
        printf("\n");

        // ------------------------------------------Debug Output Code--------------------------------------------------------------

        printf("The Central server received information from Backend-Server S using UDP over port %d.\n", getPortNumber(sockfd_udp));

        getCompatibilityServerP(sockfd_udp, num_nodes, scores_list, adjacency_matrix);

        if (send(child_fd_A, "HeyA", 4, 0) == -1)
        {
            perror("Central: client A send");
            exit(1);
        }

        close(child_fd_A);

        if (send(child_fd_B, "HeyB", 4, 0) == -1)
        {
            perror("Central: client A send");
            exit(1);
        }

        close(child_fd_B);
    }
}