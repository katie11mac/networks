#ifndef __STACK_H
#define __STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "util.h"
#include "cs431vde.h"
#include "stack-structs.h"
#include "crc32.h"
#include "checksum.h"

// Ethernet 
#define ETHER_MIN_DATA_SIZE 46 
#define ETHER_MAX_DATA_SIZE 1500
#define ETHER_FCS_SIZE 4

#define ETHER_MIN_FRAME_SIZE (sizeof(struct ether_header) + ETHER_MIN_DATA_SIZE + ETHER_FCS_SIZE)
#define ETHER_MAX_FRAME_SIZE (sizeof(struct ether_header) + ETHER_MAX_DATA_SIZE + ETHER_FCS_SIZE)

#define ETHER_BROADCAST_ADDR "\xff\xff\xff\xff\xff\xff"
#define ETHER_TYPE_IP "\x08\x00"
#define ETHER_TYPE_ARP "\x08\x06"

// IP 
#define IP_INITIAL_TTL 64
#define DIRECT_NETWORK_GATEWAY "\x00\x00\x00\x00"

// ICMP
#define ICMP_IP_ORIGINAL_DATA_SIZE 64 / 8
#define ICMP_MAX_DATA_SIZE sizeof(struct ip_header) + ICMP_IP_ORIGINAL_DATA_SIZE

// Stack
#define RECEIVING_INTERFACE 0
#define NUM_INTERFACES 4
#define NUM_ARP_ENTRIES 3
#define NUM_ROUTES 6


// Initializing global variables
void init_fds();
void init_interfaces();
void init_routing_table();
void init_arp_cache();

// Ethernet functions
int handle_ethernet_frame(struct interface *iface);
int is_valid_frame_length(ssize_t frame_len);
int is_valid_fcs (uint8_t *frame, size_t frame_len);
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface iface);
int compose_ether_frame(uint8_t *frame, struct ether_header *new_ether_header, uint8_t *data, size_t data_size);

// ARP functions
int handle_arp_packet(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_length);


// IP functions
int handle_ip_packet(struct interface *iface, uint8_t *packet, int packet_len);
int route_ip_packet(uint8_t *packet, size_t packet_length);
int compose_ip_packet(uint8_t *packet, struct ip_header *ip_header, uint8_t *payload, size_t payload_len);
int is_valid_ip_checksum(struct ip_header *curr_packet);
int is_valid_ihl(struct ip_header *curr_packet);
int is_valid_ip_version(struct ip_header *curr_packet);
struct interface *determine_local_interface(struct ip_header *curr_packet);
uint32_t array_to_uint32(uint8_t array[4]);
struct route *determine_route(struct ip_header *curr_packet);
int determine_mac_from_ip(uint8_t *mac_dst, uint8_t *ip_addr);

// ICMP functions
void send_icmp_message(uint8_t *original_ip_packet, size_t original_ip_packet_len, uint8_t type, uint8_t code);
void old_send_icmp_message(uint8_t frame[1600], ssize_t frame_len, uint8_t type, uint8_t code, int (*fds)[2]);

#endif /* __STACK_H */
