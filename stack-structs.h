#ifndef __STACK_STRUCTS_H
#define __STACK_STRUCTS_H

struct ether_header {
    uint8_t dst[6];
    uint8_t src[6];
    uint8_t type[2];
};

struct ip_address {
    uint8_t part1;
    uint8_t part2;
    uint8_t part3;
    uint8_t part4;
};

struct interface {
    uint8_t ether_addr[6];
    struct ip_address ip_addr;
};

struct arp_entry {
    uint8_t ether_addr[6];
    struct ip_address ip_addr;
};

struct route {
    int num_interface; // number interface on route
    struct ip_address dst;
    struct ip_address gateway;
    struct ip_address genmask;
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
    struct ip_address src_addr;
    struct ip_address dst_addr;
    // OPTIONS WITH VARIABLE LENGTH
};

struct arp_packet {
    uint8_t hardware_type[2];
    uint8_t protocol_type[2];
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t opcode;
    uint8_t sender_mac_addr[6];
    struct ip_address sender_ip_addr;
    uint8_t target_mac_addr[6];
    struct ip_address target_ip_addr;
};

struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint32_t unused;
    struct ip_header original_ip_header;
    uint8_t original_data[64];
};

#endif /* __STACK_STRUCTS_H */
