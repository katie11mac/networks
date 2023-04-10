#ifndef __STACK_H
#define __STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

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
	uint32_t genmask; // unsure of the data type 
	// gateway -- i dont think you need it, since we have the num_interface and can determine the ip address through there 
};

struct ip_header {
	uint8_t version : 4;
	uint8_t ihl : 4;
	uint8_t service;
	uint16_t total_length;
	uint16_t identification;
	uint8_t flags : 3;
	uint16_t frame_offset : 13;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t header_checksum;
	struct ip_address src_addr;
	struct ip_address dst_addr;
	// OPTIONS WITH VARIABLE LENGTH 
};

uint32_t crc32(uint32_t crc, const void *buf, size_t size);

int is_valid_frame_length(ssize_t frame_len);
int check_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], struct interface *interfaces, uint8_t num_interfaces);

#endif /* __STACK_H */
