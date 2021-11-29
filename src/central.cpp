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
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>

#define BACKLOG 5
#define localhost "127.0.0.1"
#define TCP_PORT_A "25499"
#define TCP_PORT_B "26499"
#define UDP_PORT "24499"
#define SERVER_T_PORT "21499"
#define SERVER_S_PORT "22499"
#define SERVER_P_PORT "23499"

#define MAX_BUF_LEN 30000

void BootUpMsg()
{
    printf("The Central server is up and running.\n");
}

// TCP Socket setup, bind and listen code from Beej’s Guide to Network Programming
int SetupTCPSocket(const char *port)
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

// UDP Socket setup code from Beej’s Guide to Network Programming
int SetupUDPSocket(const char *port)
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

// Function to extract port number from socket function descriptor
int GetPortNumber(int socket_fd)
{
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr *)&sin, &sinlen);
    return ntohs(sin.sin_port);
}

// Function to communicate with ServerT using UDP and get the topology i.e., an adjacency matrix of connected nodes
void GetTopologyServerT(int sockfd, char user_name_A[], char user_name_B[], int *&num_nodes_ptr, int *&nodes_list_ptr, int **&adjacency_matrix_ptr, int *&node_A_mapping_ptr, int *&node_B_mapping_ptr)
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
        perror("Central: ServerT sendto user_name_A");
        exit(1);
    }

    // Second Send user_name_B

    if ((numbytes = sendto(sockfd, user_name_B, strlen(user_name_B), 0,
                           servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerT sendto user_name_B");
        exit(1);
    }

    printf("The Central server sent a request to Backend-Server T.\n");

    // First receive the number of nodes in the adjacency matrix

    int *num_nodes = new int(0);

    if ((numbytes = recvfrom(sockfd, num_nodes, sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerT recvfrom num nodes");
        exit(1);
    }

    int nodes_list_size = *num_nodes;
    int *nodes_list = new int[nodes_list_size];

    // Second receive the nodes list which is the corresponding index of the nodes so that their scores can be obtained from Server S

    if ((numbytes = recvfrom(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerT recvfrom nodes list");
        exit(1);
    }

    int **adjacency_matrix = new int *[nodes_list_size];

    // Third receive the adjacency matrix of the related nodes

    for (int i = 0; i < nodes_list_size; i++)
    {
        adjacency_matrix[i] = new int[nodes_list_size];
        if ((numbytes = recvfrom(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerT recvfrom adjacency matrix");
            exit(1);
        }
    }

    int *node_A_mapping = new int(0);

    // Fifth receive the index of the given user_name_A which is the index of the input name in the sorted nodes name list

    if ((numbytes = recvfrom(sockfd, node_A_mapping, sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerT recvfrom node A mapping");
        exit(1);
    }

    int *node_B_mapping = new int(0);

    // Sixth receive the index of the given user_name_B which is the index of the input name in the sorted nodes name list

    if ((numbytes = recvfrom(sockfd, node_B_mapping, sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerT recvfrom node B mapping");
        exit(1);
    }

    // Transferring the ownership of obtained data to the pointers from main

    num_nodes_ptr = num_nodes;
    nodes_list_ptr = nodes_list;
    adjacency_matrix_ptr = adjacency_matrix;
    node_A_mapping_ptr = node_A_mapping;
    node_B_mapping_ptr = node_B_mapping;

    freeaddrinfo(servinfo);
}

// Function to communicate with ServerS using UDP and get the scores list i.e., an array of scores for the related nodes
void GetScoresServerS(int sockfd, int *&num_nodes_ptr, int *&nodes_list_ptr, int *&scores_list_ptr, std::vector<std::string> &names_list)
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

    // First send num_nodes in the nodes list

    if ((numbytes = sendto(sockfd, num_nodes, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS sendto num nodes");
        exit(1);
    }

    // Second send nodes list which is the corresponding index of the nodes so that their scores can be obtained from Server S

    if ((numbytes = sendto(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS sendto nodes list");
        exit(1);
    }

    printf("The Central server sent a request to Backend-Server S.\n");

    int *scores_list = new int[nodes_list_size];

    // First receive the scores list which the correspoding scores of the nodes

    if ((numbytes = recvfrom(sockfd, scores_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerS recvfrom scores list");
        exit(1);
    }

    // Second receive the names list which is the corresponding names of the nodes

    for (int i = 0; i < nodes_list_size; i++)
    {
        char name[512];
        if ((numbytes = recvfrom(sockfd, name, sizeof name, 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerS recvfrom names list");
            exit(1);
        }
        names_list.push_back(name);
    }

    // Transferring the ownership to the pointer from main

    scores_list_ptr = scores_list;

    freeaddrinfo(servinfo);
}

// Function to communicate with ServerP using UDP and get the output strings based on the compatibility of both usernames
void GetCompatibilityServerP(int sockfd, int *&num_nodes_ptr, int *&scores_list_ptr, int **&adjacency_matrix_ptr, int *&node_A_mapping_ptr,
                             int *&node_B_mapping_ptr, std::vector<std::string> &names_list, int *&string_size_ptr, std::string &clientA_output, std::string &clientB_output)
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
    int *node_A_mapping = node_A_mapping_ptr;
    int *node_B_mapping = node_B_mapping_ptr;
    int nodes_list_size = *num_nodes;

    // First send number nodes in the adjacency matrix and scores list

    if ((numbytes = sendto(sockfd, num_nodes, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP sendto num nodes");
        exit(1);
    }

    // Second send the scores list which is an array of scores of the nodes in the adjacency matrix

    if ((numbytes = sendto(sockfd, scores_list, nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP sendto scores list");
        exit(1);
    }

    // Third send the adjacency matrix of the related nodes

    for (int i = 0; i < nodes_list_size; i++)
    {
        if ((numbytes = sendto(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerP sendto adjacency matrix");
            exit(1);
        }
    }

    // Fourth send the index of node A user_name i.e., its mapping

    if ((numbytes = sendto(sockfd, node_A_mapping, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP sendto node A mapping");
        exit(1);
    }

    // Fifth send the index of node B user_name i.e., its mapping

    if ((numbytes = sendto(sockfd, node_B_mapping, sizeof(int), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP sendto node B mapping");
        exit(1);
    }

    // Sixth send the names list as we need to mention the intermediate nodes in the output string

    for (int i = 0; i < names_list.size(); i++)
    {
        const char *temp = names_list[i].c_str();
        if ((numbytes = sendto(sockfd, temp, (names_list[i].length() + 1), 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerP sendto names list");
            exit(1);
        }
    }

    printf("The Central server sent a processing request to Backend-Server P.\n");

    // The Central server first is waiting to receives the entire output string from the central server

    int *string_length = new int(0);

    if ((numbytes = recvfrom(sockfd, string_length, sizeof(int), 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
    {
        perror("Central: ServerP recvfrom string length");
        exit(1);
    }

    int string_size = *string_length;

    // Once the central receives the output string length it decides whether it can receive the entire string in one go
    // or receive broken substrings

    // This is done to avoid overflowing the buffer and receiving a EMGSIZE error

    if (string_size < MAX_BUF_LEN)
    {
        char clientA_str[MAX_BUF_LEN];
        if ((numbytes = recvfrom(sockfd, clientA_str, sizeof clientA_str, 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerP recvfrom clientA str");
            exit(1);
        }
        clientA_output.append(clientA_str);

        char clientB_str[MAX_BUF_LEN];
        if ((numbytes = recvfrom(sockfd, clientB_str, sizeof clientB_str, 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
        {
            perror("Central: ServerP recvfrom clientB str");
            exit(1);
        }
        clientB_output.append(clientB_str);
    }
    else
    {
        char clientA_str[MAX_BUF_LEN + 10];
        char clientB_str[MAX_BUF_LEN + 10];
        while (string_size > 0)
        {

            if ((numbytes = recvfrom(sockfd, clientA_str, sizeof clientA_str, 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
            {
                perror("Central: ServerP recvfrom clientA str");
                exit(1);
            }
            clientA_output.append(clientA_str);

            if ((numbytes = recvfrom(sockfd, clientB_str, sizeof clientB_str, 0, servinfo->ai_addr, &servinfo->ai_addrlen)) == -1)
            {
                perror("Central: ServerP recvfrom clientB str");
                exit(1);
            }
            clientB_output.append(clientB_str);
            string_size -= MAX_BUF_LEN;
        }
    }

    string_size_ptr = string_length;

    freeaddrinfo(servinfo);
}

// Function to send the compatibility of usernames back to the clients over the input TCP socket
void SendCompatibilityClient(int sockfd, std::string &client_output)
{
    int numbytes;
    int string_size = client_output.length();
    int *string_length = new int(string_size);

    if ((numbytes = send(sockfd, string_length, sizeof(int), 0)) == -1)
    {
        perror("Central: Client send string length");
        exit(1);
    }

    // Based on the output string length central decides whether it can send the entire string in one go
    // or send broken down substrings

    // This is done to avoid overflowing the buffer and receiving a EMGSIZE error

    if (string_size < MAX_BUF_LEN)
    {
        const char *client_str = client_output.c_str();
        if ((numbytes = send(sockfd, client_str, string_size + 1, 0)) == -1)
        {
            perror("Central: Client send clientA str");
            exit(1);
        }
    }
    else
    {
        int i = 0;
        while (string_size > 0)
        {
            std::string client_substr = client_output.substr(i * MAX_BUF_LEN, MAX_BUF_LEN);

            const char *client_str = client_substr.c_str();
            if ((numbytes = send(sockfd, client_str, string_size + 1, 0)) == -1)
            {
                perror("Central: Client recvfrom clientA str");
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

    int sockfd_A = SetupTCPSocket(TCP_PORT_A);
    int sockfd_B = SetupTCPSocket(TCP_PORT_B);
    int sockfd_udp = SetupUDPSocket(UDP_PORT);

    struct sockaddr_in client_addr_A, client_addr_B;
    socklen_t addr_len = sizeof(client_addr_A);
    int child_fd_A, child_fd_B;
    char user_name_A[512];
    char user_name_B[512];
    int numbytes;

    while (1)
    {
        // Child socket generated after TCP handshake with client A
        child_fd_A = accept(sockfd_A, (struct sockaddr *)&client_addr_A, &addr_len);
        if (child_fd_A == -1)
        {
            perror("Central: client A accept");
            exit(1);
        }

        // Receive the username from client A
        if ((numbytes = recv(child_fd_A, user_name_A, sizeof user_name_A, 0)) == -1)
        {
            perror("Central: client A recv");
            exit(1);
        }
        user_name_A[numbytes] = '\0';

        printf("The Central server received input=%s from the client using TCP over port %d.\n", user_name_A, GetPortNumber(sockfd_A));

        // Child socket generated after TCP handshake with client B
        child_fd_B = accept(sockfd_B, (struct sockaddr *)&client_addr_B, &addr_len);
        if (child_fd_B == -1)
        {
            perror("Central: client B accept");
            exit(1);
        }

        // Receive number of usernames from client B to decide whether it is main case or optional case
        int *num_usernames = new int(0);

        if ((numbytes = recv(child_fd_B, num_usernames, sizeof(int), 0)) == -1)
        {
            perror("Central: client B recv");
            exit(1);
        }

        // For main case when client B has only 1 input user name
        if (*num_usernames == 1)
        {
            if ((numbytes = recv(child_fd_B, user_name_B, sizeof user_name_B, 0)) == -1)
            {
                perror("Central: client B recv");
                exit(1);
            }
            user_name_B[numbytes] = '\0';

            printf("The Central server received input=%s from the client using TCP over port %d.\n", user_name_B, GetPortNumber(sockfd_B));

            // Done till this portion - Central server receiver usernames from both clients

            int *num_nodes;
            int *nodes_list;
            int **adjacency_matrix;
            int *user_name_A_mapping;
            int *user_name_B_mapping;

            // Function to communicate with Server T and get topology information

            GetTopologyServerT(sockfd_udp, user_name_A, user_name_B, num_nodes, nodes_list, adjacency_matrix, user_name_A_mapping, user_name_B_mapping);

            printf("The Central server received information from Backend-Server T using UDP over port %d.\n", GetPortNumber(sockfd_udp));

            int *scores_list;
            std::vector<std::string> names_list;

            // Function to communicate with Server S and get related scores

            GetScoresServerS(sockfd_udp, num_nodes, nodes_list, scores_list, names_list);

            printf("The Central server received information from Backend-Server S using UDP over port %d.\n", GetPortNumber(sockfd_udp));

            int *string_size;
            std::string clientA_output;
            std::string clientB_output;

            // Function to communicate with Server P to get compatibility and corresponding output strings

            GetCompatibilityServerP(sockfd_udp, num_nodes, scores_list, adjacency_matrix, user_name_A_mapping, user_name_B_mapping, names_list, string_size, clientA_output, clientB_output);

            printf("The Central server received the results from backend server P.\n");

            // Function to send output string to client A for display

            SendCompatibilityClient(child_fd_A, clientA_output);

            printf("The Central server sent the results to client A.\n");

            // Function to send output string to client A for display

            SendCompatibilityClient(child_fd_B, clientB_output);

            printf("The Central server sent the results to client B.\n");

            close(child_fd_A);

            close(child_fd_B);

            // Freeing allocated dynamic memory
            for (int i = 0; i < *num_nodes; i++)
            {
                delete adjacency_matrix[i];
            }
            delete[] adjacency_matrix;
            delete num_nodes;
            delete[] nodes_list;
            delete user_name_A_mapping;
            delete user_name_B_mapping;
            delete[] scores_list;
            delete string_size;
        }

        // For optional case when client B has only 2 input user names
        else
        {
            std::vector<std::string> user_names_B;

            int *name_length = new int(0);

            if ((numbytes = recv(child_fd_B, name_length, sizeof(int), 0)) == -1)
            {
                perror("Central: client B recv");
                exit(1);
            }

            if ((numbytes = recv(child_fd_B, user_name_B, *name_length, 0)) == -1)
            {
                perror("Central: client B recv");
                exit(1);
            }
            user_name_B[numbytes] = '\0';

            user_names_B.push_back(user_name_B);

            if ((numbytes = recv(child_fd_B, name_length, sizeof(int), 0)) == -1)
            {
                perror("Central: client B recv");
                exit(1);
            }

            if ((numbytes = recv(child_fd_B, user_name_B, *name_length, 0)) == -1)
            {
                perror("Central: client B recv");
                exit(1);
            }
            user_name_B[numbytes] = '\0';

            user_names_B.push_back(user_name_B);

            delete name_length;

            printf("The Central server received inputs=%s and %s from the client using TCP over port %d.\n", const_cast<char *>(user_names_B[0].c_str()), const_cast<char *>(user_names_B[1].c_str()), GetPortNumber(sockfd_B));

            std::string clientA_output;
            std::string clientB_output;

            // Run a loop over the 2 usernames to find respective compatibility with username from client A
            for (int i = 0; i < 2; i++)
            {

                int *num_nodes;
                int *nodes_list;
                int **adjacency_matrix;
                int *user_name_A_mapping;
                int *user_name_B_mapping;

                // Function to communicate with Server T and get topology information

                GetTopologyServerT(sockfd_udp, user_name_A, const_cast<char *>(user_names_B[i].c_str()), num_nodes, nodes_list, adjacency_matrix, user_name_A_mapping, user_name_B_mapping);

                printf("The Central server received information from Backend-Server T using UDP over port %d.\n", GetPortNumber(sockfd_udp));

                int *scores_list;
                std::vector<std::string> names_list;

                // Function to communicate with Server S and get related scores

                GetScoresServerS(sockfd_udp, num_nodes, nodes_list, scores_list, names_list);

                printf("The Central server received information from Backend-Server S using UDP over port %d.\n", GetPortNumber(sockfd_udp));

                int *string_size;
                std::string temp_clientA_output;
                std::string temp_clientB_output;

                // Function to communicate with Server P to get compatibility and corresponding output strings

                GetCompatibilityServerP(sockfd_udp, num_nodes, scores_list, adjacency_matrix, user_name_A_mapping, user_name_B_mapping, names_list, string_size, temp_clientA_output, temp_clientB_output);

                printf("The Central server received the results from backend server P.\n");

                clientA_output.append(temp_clientA_output);
                clientB_output.append(temp_clientB_output);

                // Freeing allocated dynamic memory
                for (int i = 0; i < *num_nodes; i++)
                {
                    delete adjacency_matrix[i];
                }
                delete[] adjacency_matrix;
                delete num_nodes;
                delete[] nodes_list;
                delete user_name_A_mapping;
                delete user_name_B_mapping;
                delete[] scores_list;
                delete string_size;
            
            }

            // Function to send output string to client A for display

            SendCompatibilityClient(child_fd_A, clientA_output);

            printf("The Central server sent the results to client A.\n");

            // Function to send output string to client A for display

            SendCompatibilityClient(child_fd_B, clientB_output);

            printf("The Central server sent the results to client B.\n");

            close(child_fd_A);

            close(child_fd_B);
        }

        // Freeing allocated dynamic memory
        delete num_usernames;
    }
    close(sockfd_udp);
    close(sockfd_A);
    close(sockfd_B);
}