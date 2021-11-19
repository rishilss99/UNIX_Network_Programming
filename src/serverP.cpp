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

// More than what is specified in project details
#include <iostream>
#include <cmath>

void bootUpMsg()
{
    printf("The ServerP is up and running using UDP on port %s\n", PORT);
}

double **GenerateWeightedAdjacencyMatrix(int num_nodes, int *&scores_list, int **&original_adjacency_matrix)
{
    double **weight_adj_matrix = new double *[num_nodes];

    for (int i = 0; i < num_nodes; i++)
    {
        weight_adj_matrix[i] = new double[num_nodes];
        for (int j = 0; j < num_nodes; j++)
        {
            if (original_adjacency_matrix[i][j] == 0 || i == j)
            {
                weight_adj_matrix[i][j] = 0;
            }
            else
            {
                double score = (abs(scores_list[i] - scores_list[j]) * 1.0) / (scores_list[i] + scores_list[j]);
                weight_adj_matrix[i][j] = score;
            }
        }
    }

    // ------------------------------------------Debug Output Code--------------------------------------------------------------
    // for (int i = 0; i < num_nodes; i++)
    // {
    //     for (int k = 0; k < num_nodes; k++)
    //     {
    //         printf("%.2f ", weight_adj_matrix[i][k]);
    //     }
    //     printf("\n");
    // }
    // ------------------------------------------Debug Output Code--------------------------------------------------------------

    return weight_adj_matrix;
}

void FindCompatibility(int num_nodes, double **&weighted_adjacency_matrix)
{
    
}

int main()
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

    struct sockaddr_in cliaddr;
    socklen_t addr_len = sizeof(cliaddr);
    int numbytes;

    while (1)
    {

        int *num_nodes = new int(0);

        if ((numbytes = recvfrom(sockfd, num_nodes, sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerP: Central recvfrom num nodes");
            exit(1);
        }

        //------------------------------------------Debug Output Code--------------------------------------------------------------
        // printf("%d\n",*num_nodes);
        //------------------------------------------Debug Output Code--------------------------------------------------------------

        int nodes_list_size = *num_nodes;
        int *scores_list = new int[nodes_list_size];

        if ((numbytes = recvfrom(sockfd, scores_list, nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerP: Central recvfrom scores list");
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
            if ((numbytes = recvfrom(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
            {
                perror("ServerP: Central recvfrom adjacency matrix");
                exit(1);
            }

            //------------------------------------------Debug Output Code--------------------------------------------------------------
            // for (int k = 0; k < nodes_list_size; k++)
            //     printf("%d ", adj_matrix[i][k]);
            // printf("\n");
            //------------------------------------------Debug Output Code--------------------------------------------------------------
        }

        printf("The ServerP received the topology and score information.\n");

        double **weight_adjacency_matrix = GenerateWeightedAdjacencyMatrix(nodes_list_size, scores_list, adjacency_matrix);

        FindCompatibility(nodes_list_size, weight_adjacency_matrix);
    }
    close(sockfd);
}