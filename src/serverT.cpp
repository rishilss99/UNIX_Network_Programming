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

// Graph class that contains the entire network formed from the edgelist.txt file
// Source: GeeksforGeeks
class Graph
{
public:
    int vertices; // No. of vertices

    // Pointer to an array containing adjacency lists
    std::vector<int> *adj;

    Graph(int); // Constructor

    // Function to add edges to the adjacency list
    void addEdge(int, int);

    std::vector<int> BFS(int, int, int[]);
};

// Constructor for the Graph class
Graph::Graph(int V)
{
    this->vertices = V;
    adj = new std::vector<int>[V + 1];
}

// Function to add edges to the adjacency list
void Graph::addEdge(int u, int v)
{
    adj[u].push_back(v); // Add w to v’s list.
    adj[v].push_back(u); // Add v to w’s list.
}

// Function for running Breadth First Search on an adjacency list given a node
// Source: GeeksforGeeks
std::vector<int> Graph::BFS(int componentNum, int src,
                            int visited[])
{
    // Mark all the vertices as not visited, create a queue for BFS
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

        // Get all adjacent vertices of the dequeued vertex u. If a adjacent has not been visited, then mark it visited nd enqueue it
        for (std::vector<int>::iterator itr = adj[u].begin();
             itr != adj[u].end(); itr++)
        {
            if (!visited[*itr])
            {
                // Assign Component Number to all the reachable nodes
                visited[*itr] = componentNum;
                queue.push(*itr);
            }
        }
    }
    return reachableNodes;
}

// Edgelist class that contains the all the information about the network topology as well as the name to index mapping
class EdgeList
{
public:
    std::map<std::string, int> name_mapping;
    Graph *social_network = NULL; //Graph pointer that is initialized once name_list is populated
    const char *file_path;        // File path for edgelist.txt
    EdgeList(const char *);
    void PrintList(); // Disply function for debugging
    void FormSocialNetwork();
    std::vector<int> FindReachableNodes(std::string node_name); //Finds reachable nodes from a given node - Input is the name of the node/person
    void DisplayReachableNodes(std::string node_name);          // Disply function for debugging
};

// Edgelist class constructor that forms the name_mapping from the edgelist.txt file
EdgeList::EdgeList(const char *edgepath = "edgelist.txt") : file_path(edgepath)
{
    std::ifstream file(file_path);
    std::string name_a, name_b;
    if (!file)
    {
        perror("ServerT: Couldn't find edgelist.txt");
        exit(1);
    }

    // Set of strings used to ensure that each name is only added once to edgelist and also get sorted order
    std::set<std::string> temp_sorted_names;
    if (file.is_open())
    {
        while (file >> name_a >> name_b)
        {
            temp_sorted_names.insert(name_a);
            temp_sorted_names.insert(name_b);
        }
    }

    // Iterate over the sorted names set and get the mapping for each name and its corresponding index
    int count = 0;
    for (std::set<std::string>::iterator itr = temp_sorted_names.begin(); itr != temp_sorted_names.end(); itr++)
    {
        name_mapping[*itr] = count++;
    }
    file.close();
}

// Disply name mapping function used for debugging
void EdgeList::PrintList()
{
    for (std::map<std::string, int>::iterator itr = name_mapping.begin(); itr != name_mapping.end(); itr++)
        std::cout << itr->first << " " << itr->second << "\n";
}

// Function to form the adjacency list from edgelist.txt. We need name_mapping as we are using indices instead of actual names in the adjacency list
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

    // Sort the vector of direct edges for each node
    for (int i = 0; i < social_network->vertices; i++)
    {
        std::sort(social_network->adj[i].begin(), social_network->adj[i].end());
    }
}

// Display all the Reachable Nodes from a node 'n'. Function used for debugging
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

// Function that takes a node name as input and gets a list of all reachable nodes mapped to their indices using the name_mapping
std::vector<int> EdgeList::FindReachableNodes(std::string node_name)
{
    // Get the number of nodes in the graph
    int V = social_network->vertices;

    // Take a integer visited array and initialize all the elements with 0
    int visited[V + 1];
    memset(visited, 0, sizeof(visited));

    // Map to store list of reachable Nodes for a given node
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

        // Store the reachable Nodes corresponding to the node 'i'
        m = social_network->BFS(componentNum, node_idx, visited);
    }
    return m;
}

// Given 2 nodes list this function merges and sorts both lists ensuring their are no duplicate values
std::set<int> MergeNodesList(std::vector<int> listA, std::vector<int> listB)
{
    std::set<int> merge_list;
    for (int i = 0; i < listA.size(); i++)
        merge_list.insert(listA[i]);
    for (int k = 0; k < listB.size(); k++)
        merge_list.insert(listB[k]);
    return merge_list;
}

// Function to convert a set a dynamically allocated array
int *SetToArray(std::set<int> unique_list)
{
    int *arr = new int[unique_list.size()];
    int i = 0;
    for (std::set<int>::iterator itr = unique_list.begin(); itr != unique_list.end(); itr++)
        arr[i++] = *itr;
    return arr;
}


// Function that forms the adjacency matrix to be sent back to central using the adjacency list and the node list
int **AdjacencyListToMatrix(std::vector<int> *adjcency_list, int *&nodes_list, int num_nodes)
{
    int **adj_matrix = new int *[num_nodes];
    for (int i = 0; i < num_nodes; i++)
        adj_matrix[i] = new int[num_nodes];
    for (int k = 0; k < num_nodes; k++)
    {
        int node_index = nodes_list[k]; //This is the actual value of the node which is equal to its index in the adjacency list
        int i = 0;
        int j = 0;
        while (j < num_nodes)
        {
            if (k == j)
            {
                adj_matrix[k][j] = 1;
                j++;
            }
            else if (i == adjcency_list[node_index].size())
            {
                adj_matrix[k][j] = 0;
                j++;
            }
            else if (nodes_list[j] == adjcency_list[node_index][i])
            {
                adj_matrix[k][j] = 1;
                j++;
                i++;
            }
            else if (nodes_list[j] < adjcency_list[node_index][i])
            {
                adj_matrix[k][j] = 0;
                j++;
            }
            else
                i++;
        }
    }

    return adj_matrix;
}

// Given a node's index in the name mapping this function finds its index in the nodes list
// Uses Binary Search as nodes list is sorted
int FindIdxInNodesList(int nodes_list_size, int *&nodes_list, int key)
{
    int low = 0;
    int high = nodes_list_size - 1;
    int mid = 0;
    while (low <= high)
    {
        mid = low + (high - mid) / 2;
        if (nodes_list[mid] == key)
            return mid;
        else if (nodes_list[mid] > key)
            high = mid - 1;
        else
            low = mid + 1;
    }
    return mid;
}

void BootUpMsg()
{
    printf("The ServerT is up and running using UDP on port %s\n", PORT);
}

int main()
{
    BootUpMsg();

    // UDP Socket setup code from Beej’s Guide to Network Programming

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

    // Phase 2: Receive 2 node names from Central Server and Generate: 
    // 1) Sorted List of reachable nodes 2) Adjacency matrix of reachable nodes 3) Index mapping of the 2 node names
    char user_name_A[512];
    char user_name_B[512];
    struct sockaddr_in cliaddr;
    socklen_t addr_len = sizeof(cliaddr);
    int numbytes;
    while (1)
    {

        // Receive user_name_A from Central
        
        if ((numbytes = recvfrom(sockfd, user_name_A, MAXBUFLEN - 1, 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerT: recvfrom");
            exit(1);
        }
        user_name_A[numbytes] = '\0';
        std::string node_A(user_name_A);

        // Receive user_name_B from Central

        if ((numbytes = recvfrom(sockfd, user_name_B, MAXBUFLEN - 1, 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerT: recvfrom");
            exit(1);
        }
        user_name_B[numbytes] = '\0';
        std::string node_B(user_name_B);

        printf("The ServerT received a request from Central to get the topology.\n");

        // Server T has received the node names

        // nodes_set is a merged list of all related vertices/nodes from both the given nodes

        std::set<int> nodes_set = MergeNodesList(network_topology.FindReachableNodes(node_A), network_topology.FindReachableNodes(node_B));

        // num_nodes is the total number of related vertices/nodes

        int nodes_list_size = nodes_set.size();

        // Send num_nodes to Central so that it knows the size of array for nodes_list and adjacency_matrix

        int *num_nodes = new int(nodes_list_size);

        if ((numbytes = sendto(sockfd, num_nodes, sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerT: Central sendto num nodes");
            exit(1);
        }

        // nodes_list is the array of nodes converted from the set that is to be sent to the Central server

        int *nodes_list = SetToArray(nodes_set);

        if ((numbytes = sendto(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerT: Central sendto nodes list");
            exit(1);
        }

        // adjacency_matrix is the matrix of all the reachable nodes directly forwarded by the central server to server P

        int **adjacency_matrix = AdjacencyListToMatrix(network_topology.social_network->adj, nodes_list, nodes_list_size);

        for (int i = 0; i < nodes_list_size; i++)
        {
            if ((numbytes = sendto(sockfd, adjacency_matrix[i], nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
            {
                perror("ServerT: Central sendto adjacency matrix");
                exit(1);
            }
        }

        // node_A_mapping is the index of the given user_name_A in the final nodes_list

        int *node_A_mapping = new int(FindIdxInNodesList(nodes_list_size, nodes_list, network_topology.name_mapping[node_A]));

        if ((numbytes = sendto(sockfd, node_A_mapping, sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerT: Central sendto node A mapping");
            exit(1);
        }

         // node_B_mapping is the index of the given user_name_B in the final nodes_list

        int *node_B_mapping = new int(FindIdxInNodesList(nodes_list_size, nodes_list, network_topology.name_mapping[node_B]));

        if ((numbytes = sendto(sockfd, node_B_mapping, sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerT: Central sendto node A mapping");
            exit(1);
        }

        printf("The ServerT finished sending the topology to Central.\n");

        // Freeing allocated dynamic memory
        for (int i = 0; i < *num_nodes; i++)
        {
            delete adjacency_matrix[i];
        }
        delete[] adjacency_matrix;
        delete num_nodes;
        delete[] nodes_list;
        delete node_A_mapping;
        delete node_B_mapping;
    }
    close(sockfd);
}