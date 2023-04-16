#ifndef __STACK_H
#define __STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "util.h"
#include "cs431vde.h"

#define METADATA_SIZE 18
#define MIN_DATA_SIZE 46 
#define MAX_DATA_SIZE 1500

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
	// need something to signify which interface it would be for
	int num_interface;
	struct ip_address dst;
	struct ip_address genmask; // unsure of the data type 
	// gateway -- i dont think you need it, since we have the num_interface and can determine the ip address through there 
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

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
uint16_t ip_checksum (void *addr, uint32_t count);

void init_interfaces(struct interface **interfaces, uint8_t num_interfaces);
void init_routing_table(struct route **routing_table, uint8_t num_routes);
void init_arp_cache(struct arp_entry **arp_entries, uint8_t num_arp_entries);
int is_valid_frame_length(ssize_t frame_len);
int is_valid_fcs (uint8_t (*frame)[1600], size_t frame_len, ssize_t data_len, uint32_t fcs);
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], struct interface *interfaces, uint8_t num_interfaces);
int check_ip_dst(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces);
int compare_ip_addr_structs(struct ip_address addr1, struct ip_address addr2);
uint32_t convert_ip_addr_struct(struct ip_address ip);
int determine_route(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces, struct route *routing_table, uint8_t num_routes);

#endif /* __STACK_H */
