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
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <queue>
#include <fstream>
#include <set>
#include <algorithm>

#define PORT "21499"
#define localhost "127.0.0.1"
#define MAXBUFLEN 512

class Graph
{
public:
    int vertices; // No. of vertices

    // Pointer to an array containing adjacency lists
    std::list<int> *adj;

    Graph(int); // Constructor

    void addEdge(int, int);

    std::vector<int> BFS(int, int, int[]);
};

Graph::Graph(int V)
{
    this->vertices = V;
    adj = new std::list<int>[V + 1];
}

void Graph::addEdge(int u, int v)
{
    adj[u].push_back(v); // Add w to v’s list.
    adj[v].push_back(u); // Add v to w’s list.
}

std::vector<int> Graph::BFS(int componentNum, int src,
                            int visited[])
{
    // Mark all the vertices as not visited
    // Create a queue for BFS
    std::queue<int> queue;

    queue.push(src);

    // Assign Component Number
    visited[src] = componentNum;

    // Vector to store all the reachable nodes from 'src'
    std::vector<int> reachableNodes;

    while (!queue.empty())
    {
        // Dequeue a vertex from queue
        int u = queue.front();
        queue.pop();

        reachableNodes.push_back(u);

        // Get all adjacent vertices of the dequeued
        // vertex u. If a adjacent has not been visited,
        // then mark it visited nd enqueue it
        for (std::list<int>::iterator itr = adj[u].begin();
             itr != adj[u].end(); itr++)
        {
            if (!visited[*itr])
            {
                // Assign Component Number to all the
                // reachable nodes
                visited[*itr] = componentNum;
                queue.push(*itr);
            }
        }
    }
    return reachableNodes;
}

class EdgeList
{
public:
    std::map<std::string, int> name_mapping;
    Graph *social_network = NULL; //Graph pointer that is initialized once name_list is populated
    const char *file_path;
    EdgeList(const char *);
    void PrintList();
    void FormSocialNetwork();
    std::vector<int> FindReachableNodes(std::string node_name); //Finds reachable nodes from a given node - Input is the name of the node/person
    void DisplayReachableNodes(std::string node_name);
};

EdgeList::EdgeList(const char *edgepath = "edgelist.txt") : file_path(edgepath)
{
    std::ifstream file(file_path);
    std::string name_a, name_b;
    if (!file)
    {
        perror("ServerT: Couldn't find edgelist.txt");
        exit(1);
    }
    std::set<std::string> temp_sorted_names;
    if (file.is_open())
    {
        while (file >> name_a >> name_b)
        {
            temp_sorted_names.insert(name_a);
            temp_sorted_names.insert(name_b);
        }
    }
    int count = 0;
    for (std::set<std::string>::iterator itr = temp_sorted_names.begin(); itr != temp_sorted_names.end(); itr++)
    {
        name_mapping[*itr] = count++;
    }
    file.close();
}

void EdgeList::PrintList()
{
    for (std::map<std::string, int>::iterator itr = name_mapping.begin(); itr != name_mapping.end(); itr++)
        std::cout << itr->first << " " << itr->second << "\n";
}

void EdgeList::FormSocialNetwork()
{
    if (name_mapping.size() == 0)
    {
        perror("ServerT: Name List not populated yet check file I/O");
        exit(1);
    }
    social_network = new Graph(name_mapping.size());
    std::ifstream file(file_path);
    std::string name_a, name_b;
    if (file.is_open())
    {
        while (file >> name_a >> name_b)
        {
            social_network->addEdge(name_mapping[name_a], name_mapping[name_b]);
        }
    }
    file.close();
}

// Display all the Reachable Nodes from a node 'n'
void EdgeList::DisplayReachableNodes(std::string node_name)
{
    // At this point, we have all reachable nodes
    std::cout << "Reachable Nodes from " << node_name << " are\n";
    std::vector<int> m = FindReachableNodes(node_name);
    if (m.empty())
        std::cout << "None";
    else
    {
        for (int i = 0; i < m.size(); i++)
            std::cout << m[i] << " ";
    }
    std::cout << "\n";
}

// Find all the reachable nodes for every element
// in the arr
std::vector<int> EdgeList::FindReachableNodes(std::string node_name)
{
    // Get the number of nodes in the graph
    int V = social_network->vertices;

    // Take a integer visited array and initialize
    // all the elements with 0
    int visited[V + 1];
    memset(visited, 0, sizeof(visited));

    // Map to store list of reachable Nodes for a
    // given node.
    std::vector<int> m;

    // Initialize component Number with 0
    int componentNum = 0;

    // Condition to check if node_name is in the network or not (For edge case when name is not in the network)
    if (name_mapping.find(node_name) == name_mapping.end())
        return m;

    int node_idx = name_mapping[node_name];

    // Visit all the nodes of the component
    if (!visited[node_idx])
    {
        componentNum++;

        // Store the reachable Nodes corresponding to
        // the node 'i'
        m = social_network->BFS(componentNum, node_idx, visited);
    }
    return m;
}

std::set<int> MergeNodesList(std::vector<int> listA, std::vector<int> listB)
{
    std::set<int> merge_list;
    for (int i = 0; i < listA.size(); i++)
        merge_list.insert(listA[i]);
    for (int k = 0; k < listB.size(); k++)
        merge_list.insert(listB[k]);
    return merge_list;
}

void bootUpMsg()
{
    printf("The ServerT is up and running using UDP on port %s\n", PORT);
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

    // Socket setup and bind to UDP port ------ Phase 1 Done

    // Phase 2: Load edgelist.txt and form adjacency list ------Done

    EdgeList network_topology;
    network_topology.FormSocialNetwork();
    // network_topology.PrintList();
    // network_topology.DisplayReachableNodes("King");

    // Phase 2: Receive 2 node names from Central Server and Generate: 1) Sorted List of reachable nodes 2) Adjacency matrix of reachable nodes
    char user_name_A[512];
    char user_name_B[512];
    struct sockaddr_in cliaddr;
    socklen_t addr_len = sizeof(cliaddr);
    int numbytes;
    while (1)
    {
        if ((numbytes = recvfrom(sockfd, user_name_A, MAXBUFLEN - 1, 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }
        user_name_A[numbytes] = '\0';
        std::string node_A(user_name_A);
        // printf("Server T User Name A: \"%s\"\n", user_name_A);

        if ((numbytes = recvfrom(sockfd, user_name_B, MAXBUFLEN - 1, 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }
        user_name_B[numbytes] = '\0';
        std::string node_B(user_name_B);
        // printf("Server T User Name B: \"%s\"\n", user_name_B);

        printf("The ServerT received a request from Central to get the topology.\n");

        // Server T has received the node names

        // network_topology.DisplayReachableNodes(node_A);
        // network_topology.DisplayReachableNodes(node_B);

        std::set<int> result = MergeNodesList(network_topology.FindReachableNodes(node_A), network_topology.FindReachableNodes(node_B));


        // for (std::set<int>::iterator itr = result.begin(); itr != result.end(); itr++)
        //     std::cout << *itr << " ";
        // std::cout << "\n";
    }
    close(sockfd);
}