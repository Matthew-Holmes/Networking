#include <stdio.h>
#include <stdlib.h>
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
    int packet_len = strlen(packet);

    // create the UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);    
    if (sockfd < 0) 
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("created socket.\naddress family: internet\ncode: %d\n", AF_INET);
    printf("and with SOCK_DGRAM: %d\n", SOCK_DGRAM);

    char *ip = "104.16.184.241"; // icanhazip.com

    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(8080); // desination port
    server_addr.sin_addr.s_addr = inet_addr(ip); // destination IP

    printf("addressing port 8080 at IP: %s\n", ip);
    printf("network byte order port num: %d\n", htons(8080));
    printf("inet address of IP: %d\n", inet_addr(ip));

    if (sendto(
            sockfd,
            packet,
            packet_len,
            0,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr)) < 0) 
    {
        perror("send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    } 

    printf("packet sent successfully\n");

    close(sockfd);

    printf("socket closed");

    return 0;
}
