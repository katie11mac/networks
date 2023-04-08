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

uint32_t crc32(uint32_t crc, const void *buf, size_t size);

void initialize_interfaces(struct interface *interfaces);
int is_valid_frame_length(ssize_t frame_len);
int check_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, uint8_t broadcast_addr[6], struct interface *interfaces, uint8_t num_interfaces);

#endif /* __STACK_H */
