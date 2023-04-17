/*
 * testing-sender.c
 */

#include "stack.h"

int main(int argc, char *argv[])
{
    int fds[2];

    uint8_t frame[1600];
    ssize_t frame_len;

    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", "/tmp/net1.vde", NULL };
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	struct ether_header test;
	struct ip_header ip_test;
	ssize_t data_len;
	uint32_t fcs;

    if(connect_to_vde_switch(fds, vde_cmd) < 0) {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }


	// TEST 1: Send a broadcast with valid FCS------------------------------------ 
	memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
	memcpy(test.src, "\x06\xdd\x79\xe0\x8b\x4d", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));
	
	data_len = 64;
    memset(frame + sizeof(struct ether_header), '\xff', data_len);
    
	frame_len = sizeof(struct ether_header) + data_len;
	fcs = crc32(0, frame, frame_len); 
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//----------------------------------------------------------------------------

	// TEST 2: Send a broadcast with invalid FCS----------------------------------
	memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
	memcpy(test.src, "\x06\xdd\x79\xe0\x8b\x4d", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));
	
	data_len = 64;
    memset(frame + sizeof(struct ether_header), '\xff', data_len);
    
	frame_len = sizeof(struct ether_header) + data_len;
	fcs = 123; 
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//----------------------------------------------------------------------------


	// TEST 3: Send frame not for me with unrecognized type (not IPv4)------------
	memcpy(test.dst, "\x11\x46\x6c\x7e\xff\x1a", 6);
    memcpy(test.src, "\x06\xdd\x79\xe0\x8b\x4d", 6);
    memcpy(test.type, "\x88\x80", 2);
    memcpy(frame, &test, sizeof(struct ether_header));
    
    data_len = 64;
    memset(frame + sizeof(struct ether_header), '\xff', data_len);
    
    frame_len = sizeof(struct ether_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//----------------------------------------------------------------------------


	// TEST 4: Send frame not for me with a valid fcs-----------------------------
	memcpy(test.dst, "\x11\x46\x6c\x7e\xff\x1a", 6);
    memcpy(test.src, "\x06\xdd\x79\xe0\x8b\x4d", 6);
    memcpy(test.type, "\x08\x00", 2);
    memcpy(frame, &test, sizeof(struct ether_header));
    
    data_len = 64;
    memset(frame + sizeof(struct ether_header), '\xff', data_len);
    
    frame_len = sizeof(struct ether_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//----------------------------------------------------------------------------



	// TEST 5: Send too small frame (tried sending too large frame, but vde got mad) 
	//		Note that vde ends up padding this to 60 bytes instead ... 
	//		Also memory in frame is never reset, but that shouldn't matter 
	memcpy(test.dst, "\x11\x46\x6c\x7e\xff\x1a", 6);
    memcpy(test.src, "\x06\xdd\x79\xe0\x8b\x4d", 6);
    memcpy(test.type, "\x08\x00", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    data_len = 15;
    memset(frame + sizeof(struct ether_header), '\xff', data_len);

    frame_len = sizeof(struct ether_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//----------------------------------------------------------------------------
	



	// TEST 6: Send frame for me with valid FCS and IPv4 packet w/ wrong length--
	// NOTE: THIS WILL STILL PASS THE CHECKSUM BC CHECKSUM WAS CALCULATED W WRONG LENGTH
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(60);
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	//--------------------------------------------------------------------------



	// TEST 7: Send frame for me with valid FCS and incorrect IP checksum ------  
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = 189; // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------

	// TEST 8: Send frame for me with valid FCS and unrecognized IP version------
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x65", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------

	// TEST 9: Send frame for me with valid FCS and IPv4 packet for one of my interfaces-
	// ----------------------------------

	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x05\x06\x07\x08", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------


	// TEST 10: Send Ethernet frame with IPv4 packet ------------------------------------------ 
	//			- Ether DST: single receiving interface on router
	//			- valid FCS
	//			- IP DST: device one hop away 
	//			- Valid length, checksum, and version
	//			- INVALID TTL
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 1;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------


	// TEST 11: Send Ethernet frame with IPv4 packet ------------------------------------------ 
	//			- Ether DST: single receiving interface on router
	//			- Valid length, checksum, version and TTL
	//			- NO ROUTE IN ROUTING TABLE
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x11\x18\x20\x01", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------


	


	// NEED TO TEST WHEN THERE ARE MULTIPLE ROUTES WITH THE GENMASK LENGTH

	// TEST 12: Send Ethernet frame with IPv4 packet ------------------------------------------ 
	//			- Ether DST: single receiving interface on router
	//			- valid FCS
	//			- IP DST: device one hop away 
	//			- Valid length, checksum, version and TTL 
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 10;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = ip_checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------



    /* If the program exits immediately after sending its frames, there is a
     * possibility the frames won't actually be delivered.  If, for example,
     * the "remote_vde_cmd" above is used, the user might not even finish
     * typing their password (which is accepted by a child process) before
     * this process terminates, which would result in send frames not actually
     * arriving.  Therefore, we pause and let the user manually end this
     * process. */

    printf("Press Control-C to terminate sender.\n");
    pause();

    return 0;
}
