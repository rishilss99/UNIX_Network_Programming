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

#define PORT "22499"
#define localhost "127.0.0.1"

void bootUpMsg(){
    printf("The ServerS is up and running using UDP on port %s\n",PORT);
}

int main(){
    bootUpMsg();
}