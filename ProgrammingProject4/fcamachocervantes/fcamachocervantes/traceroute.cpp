#include "traceroute.h"

using namespace std;
// ****************************************************************************
// * Compute the Internet Checksum over an arbitrary buffer.
// * (written with the help of ChatGPT 3.5)
// ****************************************************************************
uint16_t checksum(unsigned short *buffer, int size) {
    unsigned long sum = 0;
    while (size > 1) {
        sum += *buffer++;
        size -= 2;
    }
    if (size == 1) {
        sum += *(unsigned char *) buffer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short) (~sum);
}

// Function to send ICMP Echo Request
void sendICMPEchoRequest(int sockfd, const string& destIP, int ttl) {
    // Prepare ICMP Echo Request packet
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    // IP header
    struct iphdr *ipHeader = reinterpret_cast<struct iphdr*>(packet);
    ipHeader->ihl = 5;
    ipHeader->version = 4;
    ipHeader->tos = 0;
    ipHeader->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + strlen("Traceroute Data");
    ipHeader->id = htons(getpid());
    ipHeader->frag_off = 0;
    ipHeader->ttl = ttl;
    ipHeader->protocol = IPPROTO_ICMP;
    ipHeader->check = 0;
    ipHeader->saddr = 0;  // Note: Set the source address appropriately
    ipHeader->daddr = inet_addr(destIP.c_str());

    // ICMP header
    struct icmphdr *icmpHeader = reinterpret_cast<struct icmphdr*>(packet + sizeof(struct iphdr));
    icmpHeader->type = ICMP_ECHO;
    icmpHeader->code = 0;
    icmpHeader->checksum = 0;
    icmpHeader->un.echo.id = htons(getpid());
    icmpHeader->un.echo.sequence = htons(ttl);

    // Add some data to the packet
    const char* data = "Traceroute Data";
    strncpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, strlen(data));

    // Calculate ICMP checksum
    icmpHeader->checksum = checksum(reinterpret_cast<unsigned short*>(packet + sizeof(struct iphdr)), sizeof(struct icmphdr) + strlen(data));

    // Set destination IP
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    inet_pton(AF_INET, destIP.c_str(), &(destAddr.sin_addr));

    // Send the packet
    if (sendto(sockfd, packet, sizeof(packet), 0, reinterpret_cast<struct sockaddr*>(&destAddr), sizeof(destAddr)) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

// Function to receive and process ICMP Response
bool receiveAndProcessICMPResponse(int sockfd, const string& destIP, int ttl) {
    // Set timeout for receiving response
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Receive the response
    char buffer[PACKET_SIZE];
    struct sockaddr_in fromAddr;
    socklen_t addrLen = sizeof(fromAddr);
    ssize_t bytesRead = recvfrom(sockfd, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr*>(&fromAddr), &addrLen);

    if (bytesRead == -1) {
        // Handle timeout
        cout << "No response with TTL of " << ttl << endl;
        return false;
    }

    // Process the response
    struct iphdr *ipHeader = reinterpret_cast<struct iphdr*>(buffer);
    struct icmphdr *icmpHeader = reinterpret_cast<struct icmphdr*>(buffer + (ipHeader->ihl << 2));

    if (icmpHeader->type == ICMP_ECHOREPLY) {
        // Echo Reply received
        cout << "Destination reached: " << destIP << endl;
        return true;
    } else if (icmpHeader->type == ICMP_TIME_EXCEEDED) {
        // TTL Exceeded message received
        cout << "TTL Exceeded. IP: " << inet_ntoa(fromAddr.sin_addr) << endl;
        return false;
    } else {
        // Unexpected response
        cout << "Unexpected response" << endl;
        return false;
    }
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <destination_ip>" << endl;
        exit(EXIT_FAILURE);
    }

    string destIP = argv[1];

    // Set initial TTL
    int ttl = 2;

    // Create raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options to receive IP header with recvmsg
    const int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Main loop for sending ICMP Echo Request datagrams
    while (ttl <= 31) {
        // Set TTL in IP header
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
            perror("setsockopt");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Craft and send ICMP Echo Request
        sendICMPEchoRequest(sockfd, destIP, ttl);

        // Receive and process the response
        if (receiveAndProcessICMPResponse(sockfd, destIP, ttl)) {
            // ICMP Echo Response received
            break;
        } else {
            // Continue to the next TTL
            ttl++;
        }
    }

    if (ttl > 31) {
        cout << "Maximum number of datagrams (30) sent." << endl;
    }

    close(sockfd);
    return 0;
}