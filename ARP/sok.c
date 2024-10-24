#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{

    int sockfd;
    struct sockaddr_in server_addr;    
    char *packet = "hello!";

    // create the UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("created socket.\naddress family: internet\ncode: %d\n", AF_INET);
    printf("and with SOCK_DGRAM: %d\n", SOCK_DGRAM);

    close(sockfd);

    printf("socket closed");

    return 0;
}
