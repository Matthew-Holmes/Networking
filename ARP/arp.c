#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

#include <arpa/inet.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#define PROTO_ARP 0x0806 // used in ethernet frames to indicate it contains and ARP packet
#define ETH2_HEADER_LEN 14 // length of ethenet II fram header
// 6 bytes for destination MAC, 6 bytes for source MAC, 2 bytes for EtherType
#define HW_TYPE 1 // hardware type for ethernet
#define PROTOCOL_TYPE 0x800 // indicates IPv4
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01 // operation code for an ARP request
#define ARP_REPLY 0x02 // operation code for an ARP reply
#define BUF_SIZE 60 // big enough for ethernet header and ARP message

struct arp_header
{
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[MAC_LENGTH];
    unsigned char sender_ip[IPV4_LENGTH];
    unsigned char target_mac[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
};



//https://stackoverflow.com/questions/16710040/arp-request-and-reply-using-c-socket-programming
int main()
{

    int sd;
    unsigned char buffer[BUF_SIZE];
    unsigned char source_ip[4] = {192,168,1,57};
    unsigned char target_ip[4] = {192,168,1,57};
    struct ifreq ifr; // to hold interface request data
    // cast start of buffer to the ethernet header pointer fo sending and recieving frames
    struct ethhdr *send_req = (struct ethhdr *)buffer;
    struct ethhdr *rcv_resp = (struct ethhdr *)buffer;
    // cast a position in the buffer just past the ethernet header to the ARP header pointer
    struct arp_header *arp_req = (struct arp_header *)(buffer+ETH2_HEADER_LEN);
    struct arp_header *arp_resp = (struct arp_header *)(buffer+ETH2_HEADER_LEN);
    struct sockaddr_ll socket_address; // holds the low-level socket address
    int index,ret,length=0,ifindex; // length tracks frame length

    memset(buffer,0x00,60);
    /*open socket*/
    // WARNING - this won't run on windows subsystem for linux!
    // WARNING - need sudo to run this
    sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sd == -1)
    {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }


    strcpy(ifr.ifr_name,"enp8s0");
    /* retrieve ethernet interface index*/
    if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1)
    {
        perror("couldn't find ethernet interface index");
        close(sd);
        exit(1);
    }
    ifindex = ifr.ifr_ifindex;
    printf("interface index is %d\n", ifindex);
    
    /* retrieve corresponding MAC*/
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) == -1)
    {
        perror("finding MAC address failed");
        close(sd);
        exit(EXIT_FAILURE);
    }
    close(sd);

    /* print the MAC address */
    unsigned char *hwaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
        hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);


    // form the ARP request
    for (index = 0; index < 6; index++)
    {
        send_req->h_dest[index] = (unsigned char)0xff;
        arp_req->target_mac[index] = (unsigned char)0x00;
        /* filling the source MAC address in the header */
        send_req->h_source[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];
        arp_req->sender_mac[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];
        socket_address.sll_addr[index] = (unsigned char)ifr.ifr_hwaddr.sa_data[index];
    }


    /* prepare sockaddr_ll*/
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_hatype = htons(ARPHRD_ETHER);
    socket_address.sll_pkttype = (PACKET_BROADCAST);
    socket_address.sll_halen = MAC_LENGTH;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00; 

    /* setting protocol of the packet*/
    send_req->h_proto = htons(ETH_P_ARP);

    /* creating the ARP request*/
    arp_req->hardware_type = htons(HW_TYPE);
    arp_req->protocol_type = htons(ETH_P_IP);
    arp_req->hardware_len = MAC_LENGTH;
    arp_req->protocol_len = IPV4_LENGTH;
    arp_req->opcode = htons(ARP_REQUEST);

    for (index = 0; index<5; index++)
    {
        arp_req->sender_ip[index] = (unsigned char)source_ip[index];
        arp_req->target_ip[index] = (unsigned char)target_ip[index];
    }

    // submit request for a raw socket descriptor
    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        perror("failed to create a socket");
        exit(EXIT_FAILURE);
    }

    buffer[32]=0x00;
    ret = sendto(sd, buffer, 42, 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
    if (ret == -1)
    {
        perror("sending request failed");
        close(sd);
        exit(EXIT_FAILURE);
    }
    
    printf("sent the ARP request\n\t");
    for (index = 0; index < 42; index++)
    {
        printf("%02X ", buffer[index]);
        if (index % 16 == 0 && index != 0)
        {
            printf("\n\t");
        }
    }
    printf("\n\t");
     

    // wait for a reply
    while(1)
    {
        length = recvfrom(sd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (length == -1)
        {
            perror("recvfrom failed: ");
            close(sd);
            exit(EXIT_FAILURE);
        }
        if (htons(rcv_resp->h_proto) == PROTO_ARP)
        {
            printf(" recieved ARP response len=%d \n", length);
            printf(" sender IP: ");
            for (index = 0; index < 4; index++)
            {
                printf("%u.", (unsigned int)arp_resp->sender_ip[index]);
            }
            printf("\n sender MAC:");
            for (index = 0; index < 6; index++)
            {
                printf(" %02X:", arp_resp->sender_mac[index]);
            }

            printf(" receiver IP: ");
            for (index = 0; index < 4; index++)
            {
                printf(" %u.", arp_resp->target_ip[index]);
            }
            printf("\n self MAC:");
            for (index = 0; index < 6; index++)
            {
                printf(" %02X:", arp_resp->target_mac[index]);
            }
            printf("\n :");
            break;
        }
    }
    close(sd);
    return 0;
}
