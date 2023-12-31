#ifndef __STACK_H
#define __STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/random.h>

#include "util.h"
#include "cs431vde.h"
#include "stack-structs.h"
#include "crc32.h"
#include "checksum.h"

// Ethernet 
#define ETHER_MIN_DATA_SIZE 46 
#define ETHER_MAX_DATA_SIZE 1500
#define ETHER_FCS_SIZE      4

// Changed minimum length bc ARP and TCP do not always have FCS
#define ETHER_MIN_FRAME_SIZE (sizeof(struct ether_header) + ETHER_MIN_DATA_SIZE)
#define ETHER_MAX_FRAME_SIZE (sizeof(struct ether_header) + ETHER_MAX_DATA_SIZE + ETHER_FCS_SIZE)

#define ETHER_BROADCAST_ADDR "\xff\xff\xff\xff\xff\xff"
#define ETHER_TYPE_IP        "\x08\x00"
#define ETHER_TYPE_ARP       "\x08\x06"

// IP 
#define DIRECT_NETWORK_GATEWAY     "\x00\x00\x00\x00"
#define IP_INITIAL_VERSION_AND_IHL "\x45" 

#define MAX_IP_PACKET_SIZE 1600
#define IP_INITIAL_TTL     64
#define IP_ICMP_PROTOCOL   1
#define IP_TCP_PROTOCOL	   6

// ARP
#define ARP_TYPE_ETHER "\x00\x01"
#define ARP_TYPE_IP    "\x08\x00"

#define ARP_ETHER_SIZE 6
#define ARP_IP_SIZE    4

// ICMP
#define ICMP_IP_ORIGINAL_DATA_SIZE 64 / 8
#define ICMP_MAX_DATA_SIZE         sizeof(struct ip_header) + ICMP_IP_ORIGINAL_DATA_SIZE

// TCP
#define TCP_LISTENING_PORT 12345
#define TCP_URG_FLAG       32
#define TCP_ACK_FLAG       16
#define TCP_PSH_FLAG       8
#define TCP_RST_FLAG       4
#define TCP_SYN_FLAG       2
#define TCP_FIN_FLAG       1
#define TCP_DEFAULT_OFFSET 5

// Stack
#define RECEIVING_INTERFACE 0
#define NUM_INTERFACES      4
#define NUM_ARP_ENTRIES     4
#define NUM_ROUTES          6
#define NUM_POLL_FDS        NUM_INTERFACES + 1
#define MAX_CONNECTIONS     10
#define BUFFER_SIZE         512 // Based on common MSS standards

#define CLOSE_CONNECTION_COMMAND "/CLOSE\n" 

// Initializing global variables
void init_fds();
void init_interfaces();
void init_poll_fds();
void init_routing_table();
void init_arp_cache();

// User input functions
int handle_user_input();

// Ethernet functions
int handle_ethernet_frame(struct interface *iface);
int is_valid_frame_len(ssize_t frame_len);
int is_valid_fcs(uint8_t *frame, size_t frame_len);
int check_ether_dst_addr(struct ether_header *curr_frame, ssize_t frame_len, struct interface iface);
int compose_ether_frame(uint8_t *frame, struct ether_header *new_ether_header, uint8_t *data, size_t data_size);

// ARP functions
int handle_arp_packet(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_len);
int send_arp_reply(uint8_t *src, struct interface *iface, uint8_t *packet, int packet_len);

// IP functions
int handle_ip_packet(struct interface *iface, uint8_t *packet, int packet_len);
int route_ip_packet(uint8_t *packet, size_t packet_len, int is_process);
int compose_ip_packet(uint8_t *packet, struct ip_header *ip_header, uint8_t *payload, size_t payload_len);
int is_valid_ip_checksum(struct ip_header *curr_ip_header);
int is_valid_ihl(struct ip_header *curr_ip_header);
int is_valid_ip_version(struct ip_header *curr_ip_header);
int is_valid_ttl(struct ip_header *curr_ip_header);
struct interface *determine_local_interface(struct ip_header *curr_ip_header);
uint32_t array_to_uint32(uint8_t array[4]);
struct route *determine_route(struct ip_header *curr_ip_header);
struct arp_entry *determine_mac_arp(uint8_t *ip_addr);
int send_ip_packet(uint8_t protocol, uint8_t dst_addr[4], uint8_t *payload, size_t payload_len);

// ICMP functions
int send_icmp_message(uint8_t *original_ip_packet, size_t original_ip_packet_len, uint8_t type, uint8_t code);

// TCP functions 
int handle_tcp_segment(uint8_t ip_src[4], uint8_t ip_dst[4], uint8_t *segment, int segment_len);
void print_all_connections_info();
void print_connection_info(struct tcb *tcb);
char *get_connection_state_str(struct tcb *tcb);
struct tcb *determine_tcb(uint8_t ip_src[4], uint8_t ip_dst[4], struct tcp_header *curr_tcp_header);
struct tcb *add_tcb(uint8_t ip_src[4], uint8_t ip_dst[4], struct tcp_header *curr_tcp_header);
uint16_t calculate_tcp_checksum(struct tcb *curr_tcb, uint8_t *curr_tcp_segment, int tcp_length);
int is_valid_tcp_checksum(struct tcb *curr_tcb, uint8_t *curr_tcp_segment, int tcp_length);
void set_tcp_flags(struct tcp_flags *flags, struct tcp_header *curr_tcp_header);
void update_tcp_state(struct tcb *curr_tcb, uint8_t *curr_tcp_segment, int segment_len);
int update_tcb(struct tcb *curr_tcb, uint8_t *original_tcp_segment, int original_segment_len, struct tcp_flags *original_flags);
int send_tcp_segment(struct tcb *curr_tcb, uint8_t flags, uint8_t *payload, size_t payload_len);
void print_tcp_data(uint8_t *original_tcp_segment, int original_segment_len);

#endif /* __STACK_H */
