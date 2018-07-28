#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>

#define MAX_PACKET_LEN 8192

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char const *argv[])
{
	u_int16_t source_port, destination_port;
    u_int32_t source_addr, destination_addr;
    source_addr = inet_addr(argv[1]);
    destination_addr = inet_addr(argv[3]);
    source_port = atoi(argv[2]);
    destination_port = atoi(argv[4]);

    char buffer[MAX_PACKET_LEN];
    struct iphdr *ip_header = (struct iphdr *) buffer;
    struct udphdr *udp_header = (struct udp_headerhdr *) (buffer + sizeof(struct iphdr));
    char* msg = (udp_header + sizeof(struct udphdr));

    struct sockaddr_in sin;
    int one = 1;
    const int *val = &one;

    memset(buffer, 0, MAX_PACKET_LEN);
    char text[] = "Hello kitty";

    int fd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    setsockopt(fd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(destination_port);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	//make ip header
    ip_header->ihl      = 5;
    ip_header->version  = 4;
    ip_header->tos      = 16; //delay
    ip_header->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(text) + 1;
    ip_header->id       = htons(58562);
    ip_header->ttl      = 64;
    ip_header->protocol = 17; //UDP
    //ip_header address
    ip_header->saddr = source_addr;
    ip_header->daddr = destination_addr;

    udp_header->source = htons(source_port);
    //destination port
    udp_header->dest = htons(destination_port);
    udp_header->len = htons(sizeof(struct udphdr));

    strcpy(msg, text);
    // calculate the checksum for integrity
    ip_header->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));

    sendto(fd, buffer, ip_header->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
    printf("Done\n");

    close(fd);
    return 0;
}