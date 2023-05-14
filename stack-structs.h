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
    struct interface *iface; 
    uint8_t dst[4];
    uint8_t gateway[4];
    uint8_t genmask[4];
};

struct ip_header {
    uint8_t version_and_ihl;
    uint8_t service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_and_fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint8_t src_addr[4];
    uint8_t dst_addr[4];
    // OPTIONS WITH VARIABLE LENGTH
};

struct arp_header {
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

struct tcp_header {
	uint16_t src_port; 
	uint16_t dst_port; 
	uint32_t seq_num;
	uint32_t ack_num;
	uint16_t offset_reserved_control;
	uint16_t window; 
	uint16_t checksum;
	uint16_t urgent_ptr; 
	// OPTIONS, PADDING, DATA?? 
};

struct tcp_pseudo_header {
	uint8_t ip_src[4];
	uint8_t ip_dst[4];
	uint8_t zeros;
	uint8_t ptcl;
	uint16_t tcp_length;
};

enum connection_state {
	LISTEN, 
	SYN_SENT,
	SYN_RECEIVED, 
	ESTABLISHED,
	FIN_WAIT_1,
	FIN_WAIT_2,
	CLOSE_WAIT,
	CLOSING,
	LAST_ACK,
	TIME_WAIT,
	CLOSED      // initial state
};

struct tcp_flags {
	uint8_t URG; 
	uint8_t ACK; 
	uint8_t PSH; 
	uint8_t RST; 
	uint8_t SYN; 
	uint8_t FIN;
};

/*
 * The tcb struct contains information from our side of the connection 
 *
 * seq_num: 
 *	Represents our side's sequence number. 
 *	It stores the next sequence number that should be used for sending. 
 *	The acknowledgement number of received packets should be this value. 
 *
 * ack_num: 
 *  Represnts the last bytes we have received from the other side of our connection. 
 *  The sequence number of received packets should be this value. 
 */
struct tcb {
	uint8_t ip_src[4];
	uint8_t ip_dst[4];
	uint16_t src_port; 
	uint16_t dst_port;
	uint32_t seq_num; // our next sequence number for sending
	uint32_t ack_num; // last bytes received from other side of connection
	//uint16_t window;
	//struct tcp_flags flags; 
	enum connection_state state;	
	// ASSUMPTION: SEGMENTS NEVER ARRIVE OUT OF ORDER OR GO MISSING
	// WHICH PERSPECTIVE WILL THIS BE FROM: LAST RECEIVED OR EXPECTING? 
};




#endif /* __STACK_STRUCTS_H */
