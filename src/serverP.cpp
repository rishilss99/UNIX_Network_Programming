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
#include <float.h>

#define PORT "23499"
#define localhost "127.0.0.1"

// More than what is specified in project details
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#define MAX_VAL FLT_MAX
#define MAX_BUF_LEN 30000

void BootUpMsg()
{
    printf("The ServerP is up and running using UDP on port %s\n", PORT);
}

int MinDistanceIdx(int num_nodes, double *dist, bool *sptSet)
{

    // Initialize min value
    double min = MAX_VAL;
    int min_index = -1;

    for (int v = 0; v < num_nodes; v++)
    {
        if (sptSet[v] == false && dist[v] <= min)
        {
            min = dist[v];
            min_index = v;
        }
    }
    // // ------------------------------------------Debug Output Code--------------------------------------------------------------
    // std::cout << min_index << "\n";
    // // ------------------------------------------Debug Output Code--------------------------------------------------------------
    return min_index;
}

void FindIntermediateNodes(int *parent, int target, std::vector<std::string> &names_list, std::vector<std::string> &intermediate_nodes)
{

    // Base Case : If j is source
    if (parent[target] == -1)
        return;

    FindIntermediateNodes(parent, parent[target], names_list, intermediate_nodes);

    intermediate_nodes.push_back(names_list[target]);
    return;
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

std::string OutputMsg(std::string src, std::string target, std::vector<std::string> &intermediate_nodes, double score, bool reverse)
{
    std::string result = "Found compatibility for " + src + " and " + target + ":\n";
    result += src + " --- ";
    if (reverse)
        std::reverse(intermediate_nodes.begin(), intermediate_nodes.end());
    for (int i = 0; i < intermediate_nodes.size(); i++)
        result += intermediate_nodes[i] + " --- ";
    result += target + "\n";
    result += "Matching gap: ";
    std::ostringstream to_string;
    to_string << std::fixed << std::showpoint << std::setprecision(2) << score;
    result += to_string.str() + "\n";
    return result;
}

// Function that implements Dijkstra's single source shortest path algorithm for a graph represented using adjacency matrix representation
std::pair<std::string, std::string> FindCompatibility(int num_nodes, double **&weighted_adjacency_matrix, std::vector<std::string> &names_list, int src, int target)
{
    if (src == target)
    {
        printf("Invalid case both nodes can't be same\n");
        exit(1);
    }
    // The output array. dist[i] will hold the shortest distance from src to i
    double *dist = new double[num_nodes];

    // sptSet[i] will true if vertex i is included / in shortest path tree or shortest distance from src to i is finalized
    bool *sptSet = new bool[num_nodes];

    // Parent array to store shortest path tree
    int *parent = new int[num_nodes];

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < num_nodes; i++)
    {
        parent[i] = -1;
        dist[i] = MAX_VAL;
        sptSet[i] = false;
    }

    // Distance of source vertex from itself is always 0
    dist[src] = 0;
    int u = src;

    // Find shortest path for all vertices
    while (u >= 0)
    {

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the picked vertex.
        for (int v = 0; v < num_nodes; v++)

            // Update dist[v] only if is not in sptSet, there is an edge from u to v,
            // and total weight of path from src to v through u is smaller than current value of dist[v]
            if (!sptSet[v] && weighted_adjacency_matrix[u][v] != 0 && dist[u] + weighted_adjacency_matrix[u][v] < dist[v])
            {
                parent[v] = u;
                dist[v] = dist[u] + weighted_adjacency_matrix[u][v];
            }
        // Pick the minimum distance vertex from the set of vertices not yet processed. u is always equal to src in first iteration.
        u = MinDistanceIdx(num_nodes, dist, sptSet);
    }

    // // ------------------------------------------Debug Output Code--------------------------------------------------------------
    // // Print the constructed distance array
    // for (int i = 0; i < num_nodes; i++)
    // {
    //     std::cout << i << " " << dist[i] << " " << sptSet[i] << " " << parent[i] << "\n";
    // }
    // // ------------------------------------------Debug Output Code--------------------------------------------------------------

    //------------------------------------------------End of Dijkstra---------------------------------------------------------------

    std::string clientA_output;
    std::string clientB_output;

    if (dist[target] == MAX_VAL)
    {
        clientA_output = "Found no compatibility for " + names_list[src] + " and " + names_list[target] + ".\n";
        clientB_output = "Found no compatibility for " + names_list[target] + " and " + names_list[src] + ".\n";
    }
    else
    {
        std::vector<std::string> intermediate_nodes;
        FindIntermediateNodes(parent, target, names_list, intermediate_nodes);
        intermediate_nodes.pop_back(); // Pop back as the src is also included in the intermediate nodes so we need to remove it for correct string
        clientA_output = OutputMsg(names_list[src], names_list[target], intermediate_nodes, dist[target], false);
        clientB_output = OutputMsg(names_list[target], names_list[src], intermediate_nodes, dist[target], true);
    }

    // Freeing allocated dynamic memory
    delete[] dist;
    delete[] sptSet;
    delete[] parent;

    return std::make_pair(clientA_output, clientB_output);
}

void SendStrings(std::pair<std::string, std::string> &compatible_result, int sockfd, struct sockaddr_in cliaddr, socklen_t addr_len)
{
    int numbytes;
    int string_size = compatible_result.first.length();
    int *string_length = new int(string_size);

    if ((numbytes = sendto(sockfd, string_length, sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
    {
        perror("ServerP: Central sendto string length");
        exit(1);
    }

    if (string_size < MAX_BUF_LEN)
    {
        const char *clientA_str = compatible_result.first.c_str();
        if ((numbytes = sendto(sockfd, clientA_str, string_size + 1, 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerP: Central sendto clientA str");
            exit(1);
        }

        const char *clientB_str = compatible_result.second.c_str();
        if ((numbytes = sendto(sockfd, clientB_str, string_size + 1, 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerP: Central sendto clientB str");
            exit(1);
        }
    }
    else
    {
        int i = 0;
        while (string_size > 0)
        {
            std::string clientA_substr = compatible_result.first.substr(i * MAX_BUF_LEN, MAX_BUF_LEN);
            std::string clientB_substr = compatible_result.second.substr(i * MAX_BUF_LEN, MAX_BUF_LEN);

            const char *clientA_str = clientA_substr.c_str();
            if ((numbytes = sendto(sockfd, clientA_str, string_size + 1, 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
            {
                perror("ServerP: Central sendto clientA str");
                exit(1);
            }

            const char *clientB_str = clientB_substr.c_str();
            if ((numbytes = sendto(sockfd, clientB_str, string_size + 1, 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
            {
                perror("ServerP: Central sendto clientB str");
                exit(1);
            }
            i++;
            string_size -= MAX_BUF_LEN;
        }
    }

     // Freeing allocated dynamic memory
     delete string_length;
}

int main()
{

    BootUpMsg();

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

        int *node_A_mapping = new int(0);

        if ((numbytes = recvfrom(sockfd, node_A_mapping, sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerP: Central recvfrom node A mapping");
            exit(1);
        }

        int *node_B_mapping = new int(0);

        if ((numbytes = recvfrom(sockfd, node_B_mapping, sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerP: Central recvfrom node B mapping");
            exit(1);
        }

        std::vector<std::string> names_list;
        for (int i = 0; i < nodes_list_size; i++)
        {
            char name[512];
            if ((numbytes = recvfrom(sockfd, name, sizeof name, 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
            {
                perror("ServerP: Central recvfrom names list");
                exit(1);
            }
            name[numbytes] = '\0';
            names_list.push_back(name);
        }

        printf("The ServerP received the topology and score information.\n");

        double **weight_adjacency_matrix = GenerateWeightedAdjacencyMatrix(nodes_list_size, scores_list, adjacency_matrix);

        // Returns a pair of strings where the first string is the output message for client A and the second string is the output message for client B
        std::pair<std::string, std::string> compatible_result = FindCompatibility(nodes_list_size, weight_adjacency_matrix, names_list, *node_A_mapping, *node_B_mapping);

        // //------------------------------------------Debug Output Code--------------------------------------------------------------
        // std::cout << compatible_result.first;
        // std::cout << compatible_result.second;
        // //------------------------------------------Debug Output Code--------------------------------------------------------------

        SendStrings(compatible_result, sockfd, cliaddr, addr_len);

        printf("The ServerP finished sending the results to the Central.\n");

        // Freeing allocated dynamic memory
        for (int i = 0; i < *num_nodes; i++)
        {
            delete weight_adjacency_matrix[i];
        }
        delete[] weight_adjacency_matrix;
        for (int i = 0; i < *num_nodes; i++)
        {
            delete adjacency_matrix[i];
        }
        delete[] adjacency_matrix;
        delete[] scores_list;
        delete num_nodes;
        delete node_A_mapping;
        delete node_B_mapping;
    }
    close(sockfd);
}