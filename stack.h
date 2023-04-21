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


#define METADATA_SIZE 18
#define MIN_DATA_SIZE 46 
#define MAX_DATA_SIZE 1500
#define BROADCAST_ADDR "\xff\xff\xff\xff\xff\xff"
#define RECEIVING_INTERFACE 0

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
uint16_t checksum (void *addr, uint32_t count);


void init_interfaces(struct interface **interfaces, uint8_t num_interfaces);
void init_routing_table(struct route **routing_table, uint8_t num_routes);
void init_arp_cache(struct arp_entry **arp_entries, uint8_t num_arp_entries);
int is_valid_frame_length(ssize_t frame_len);
int is_valid_fcs (uint8_t (*frame)[1600], size_t frame_len, ssize_t data_len, uint32_t fcs);
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface *interfaces, uint8_t num_interfaces);

int is_valid_total_length(uint32_t *fcs_ptr, struct ip_header *curr_packet); 
int is_valid_ip_checksum(struct ip_header *curr_packet);
int is_valid_ihl(struct ip_header *curr_packet);
int is_valid_ip_version(struct ip_header *curr_packet);
int check_ip_dst(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces);
int compare_ip_addr_structs(struct ip_address addr1, struct ip_address addr2);
uint32_t convert_ip_addr_struct(struct ip_address ip);
int determine_route(struct ip_header *curr_packet, struct interface *interfaces, uint8_t num_interfaces, struct route *routing_table, uint8_t num_routes);
int determine_mac_from_ip(uint8_t *mac_dst, struct ip_address ip_addr, struct arp_entry *arp_cache, uint8_t num_arp_entries);

#endif /* __STACK_H */
