
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>

#define BUFFER_SIZE 65536

int main() {
    int sock_raw;
    unsigned char *buffer;
    int packet_count = 0;

    sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sock_raw < 0) {
        perror("Socket Error");
        return 1;
    }

    buffer = (unsigned char *)malloc(BUFFER_SIZE);

    if (buffer == NULL) {
        perror("Memory Allocation Error");
        close(sock_raw);
        return 1;
    }

    printf("Packet Capture Started...\n");

    while (1) {
        int data_size;

        data_size = recvfrom(sock_raw, buffer, BUFFER_SIZE,
                             0, NULL, NULL);

        if (data_size < 0) {
            perror("Receive Error");
            break;
        }

        if (data_size < 14)
            continue;

        struct ethhdr *eth = (struct ethhdr *)buffer;

        if (ntohs(eth->h_proto) != ETH_P_IP)
            continue;

        if (data_size < 14 + (int)sizeof(struct iphdr))
            continue;

        struct iphdr *ip_header =
            (struct iphdr *)(buffer + 14);

        if (ip_header->version != 4 ||
            data_size < 14 + (int)(ip_header->ihl * 4))
            continue;

        struct in_addr src, dst;
        src.s_addr = ip_header->saddr;
        dst.s_addr = ip_header->daddr;

        printf("\nPacket No: %d\n", ++packet_count);
        printf("Packet Size : %d bytes\n", data_size);
        printf("Source IP : %s\n", inet_ntoa(src));
        printf("Destination IP : %s\n", inet_ntoa(dst));
        printf("--------------------------\n");
    }

    close(sock_raw);
    free(buffer);

    return 0;
}
