#ifndef __STACK_STRUCTS_H
#define __STACK_STRUCTS_H

struct ether_header {
    uint8_t dst[6];
    uint8_t src[6];
    uint8_t type[2];
};

struct interface {
    uint8_t ether_addr[6];
    uint8_t ip_addr[4];
	char *name;
	int in_fd;
	int out_fd;
};

struct arp_entry {
    uint8_t ether_addr[6];
    uint8_t ip_addr[4];
};

struct route {
    int num_interface; // number interface on route
    uint8_t dst[4];
    uint8_t gateway[4];
    uint8_t genmask[4];
};

struct ip_header {
    uint8_t version_and_ihl; // NEED BIT SHIFTING
    uint8_t service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_and_fragment_offset; // NEED BIT SHIFTING
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint8_t src_addr[4];
    uint8_t dst_addr[4];
    // OPTIONS WITH VARIABLE LENGTH
};

struct arp_packet {
    uint8_t hardware_type[2];
    uint8_t protocol_type[2];
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t sender_mac_addr[6];
    uint8_t sender_ip_addr[4];
    uint8_t target_mac_addr[6];
    uint8_t target_ip_addr[4];
};

struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t unused;
};

#endif /* __STACK_STRUCTS_H */
