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
#include <fstream>
#include <set>
#include <vector>
#include <cstring>

#define PORT "22499"
#define localhost "127.0.0.1"

class ScoreList
{
public:
    std::vector<int> score_vec;        //Vector of scores sorted according to name order
    std::vector<std::string> name_vec; //Vector of names sorted according to name order
    const char *file_path;
    ScoreList(const char *);
    void PrintList();
};

ScoreList::ScoreList(const char *scorepath = "scores.txt") : file_path(scorepath)
{
    std::ifstream file(file_path);
    std::string name;
    int score;
    if (!file)
    {
        perror("ServerS: Couldn't find scores.txt");
        exit(1);
    }
    std::map<std::string, int> temp_sorted_names;
    if (file.is_open())
    {
        while (file >> name >> score)
        {
            temp_sorted_names[name] = score;
        }
    }

    for (std::map<std::string, int>::iterator itr = temp_sorted_names.begin(); itr != temp_sorted_names.end(); itr++)
    {
        name_vec.push_back(itr->first);
        score_vec.push_back(itr->second);
    }
    file.close();
}

int *MatchingScores(std::vector<int> &scores_vec, int *&nodes_list, int num_nodes)
{
    int *scores_list = new int[num_nodes];
    for (int i = 0; i < num_nodes; i++)
    {
        scores_list[i] = scores_vec[nodes_list[i]];
    }
    return scores_list;
}

std::vector<std::string> MatchingNames(std::vector<std::string> &names_vec, int *&nodes_list, int num_nodes)
{
    std::vector<std::string> names_list;
    for(int i = 0; i<num_nodes; i++)
    {
        names_list.push_back(names_vec[nodes_list[i]]);
    }
    return names_list;
}

void BootUpMsg()
{
    printf("The ServerS is up and running using UDP on port %s\n", PORT);
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
        perror("ServerS: socket");
        exit(1);
    }

    // bind it to the port and IP address we passed in to getaddrinfo():

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("ServerS: bind");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    ScoreList score_mapping;

    //------------------------------------------Debug Output Code--------------------------------------------------------------

    // for (int i = 0; i < score_mapping.score_vec.size(); i++)
    // {
    //     std::cout << score_mapping.score_vec[i] << "\n";
    // }

    //------------------------------------------Debug Output Code--------------------------------------------------------------

    struct sockaddr_in cliaddr;
    socklen_t addr_len = sizeof(cliaddr);
    int numbytes;
    while (1)
    {

        int *num_nodes = new int(0);

        if ((numbytes = recvfrom(sockfd, num_nodes, sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerS: recvfrom");
            exit(1);
        }

        //------------------------------------------Debug Output Code--------------------------------------------------------------
        // printf("%d\n",*num_nodes);
        //------------------------------------------Debug Output Code--------------------------------------------------------------

        int nodes_list_size = *num_nodes;
        int *nodes_list = new int[nodes_list_size];

        if ((numbytes = recvfrom(sockfd, nodes_list, nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, &addr_len)) == -1)
        {
            perror("ServerS: recvfrom");
            exit(1);
        }

        printf("The ServerS received a request from Central to get the scores.\n");

        //nodes_list is the array of nodes converted from the set that is to be sent to the central server
        int *scores_list = MatchingScores(score_mapping.score_vec, nodes_list, nodes_list_size);

        if ((numbytes = sendto(sockfd, scores_list, nodes_list_size * sizeof(int), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
        {
            perror("ServerS: Central sendto scores list");
            exit(1);
        }

        // Added later on as realized that the names of the intermediate as well as the input nodes also have to be printed out

        std::vector<std::string> names_list = MatchingNames(score_mapping.name_vec, nodes_list, nodes_list_size);
        for (int i = 0; i < nodes_list_size; i++)
        {
            const char* temp = names_list[i].c_str();
            if ((numbytes = sendto(sockfd, temp, (names_list[i].length() + 1), 0, (struct sockaddr *)&cliaddr, addr_len)) == -1)
            {
                perror("ServerT: Central sendto names list");
                exit(1);
            }
        }

        printf("The ServerS finished sending the scores to Central.\n");
    }
    close(sockfd);
}