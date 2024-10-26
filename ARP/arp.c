#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

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
    unsigned char target_max[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
};



//https://stackoverflow.com/questions/16710040/arp-request-and-reply-using-c-socket-programming
int main()
{
    printf("Hello world\n");
    return 0;
}
