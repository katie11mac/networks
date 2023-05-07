/*
 * testing-sender.c
 */

#include "stack.h"

int main(int argc, char *argv[])
{
    int fds[2];

    uint8_t frame[1600];
    ssize_t frame_len;
	
	struct ether_header test;
	struct ip_header ip_test;
	struct arp_header arp_test;
	ssize_t data_len;
	uint32_t fcs;

	// Variables for vde_switch (connecting to receiving interface I0 on stack.c)
    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", "/tmp/net0.vde", NULL };
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	// Connect to vde virtual switch	
    if(connect_to_vde_switch(fds, vde_cmd) < 0) {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }

	/*
	 * TEST 1: Ethernet (A2) 
	 * 
	 * Broadcast with valid FCS
	 */
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
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 2: Ethernet (A2) 
	 *
	 * Broadcast with invalid FCS
	 */
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
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 3: Ethernet (A2)
	 * 
	 * Unrecognized type (not IPv4), for me, with valid FCS
	 */
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
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
    //send_ethernet_frame(fds[1], frame, frame_len);

	
	/*
	 * TEST 4: Ethernet (A2)
	 *
	 * Not for me with valid FCS
	 */
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
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 5: Ethernet (A2) 
	 *
	 * Frame too small 
	 *
	 * Notes: 
	 *		- Tried sending too large frame, but vde got mad 
	 *		- vde ends up padding this to 60 bytes instead
	 *		- Memory in the frame is never reset, but that shouldn't matter
	 */
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
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 6: IP (A3 PI - receiving on I0) 
	 *
	 * Valid Ethernet frame with invalid IPv4 packet (wrong length)
	 *
	 * Notes: 
	 *		- Checksum will still pass bc checksum was calculated with wrong length
	 */
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
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 

	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 7: IP (A3 PI - receiving on I0)
	 *
	 * Valid Ethernet frame with invalid IPv4 packet (bad IP header checksum) 
	 */
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
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 8: IP (A3 PI - receiving on I0) 
	 *
	 * Valid Ethernet frame with unrecognized IP version (not IPv4)
	 */
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
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 9: IP (A3 PI - receiving on I0) 
	 *
	 * Valid Ethernet frame with valid IPv4 packet for one of my interfaces (I1)
	 */
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
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 10: IP (A3 PI - receiving on I0) 
	 *
	 *
	 * Ethernet frame 
	 *	- Valid 
	 *
	 * IP packet 
	 *  - Valid length and checksum 
	 *  - Recognizable version (IPv4) 
	 *	- dst: Network connected to I3 (one more hop away) 
	 *	- ttl: will be exceeded 
	 *
	 * EXPECTED RESULTS: TTL exceeded
	 */
	// Ethernet Frame 
	memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
	memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
	memcpy(test.type, "\x08\x00", 2);
	memcpy(frame, &test, sizeof(struct ether_header));	
	
	// IP Packet
	memcpy(&ip_test.version_and_ihl, "\x45", 1);
	ip_test.ttl = 0;
	ip_test.protocol = 4;
	memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
	memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x00", 4);

	// Set data inside of the IP packet
	data_len = 64;
	memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);
	
	// Set ip header values that depend on the data
	ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
	ip_test.header_checksum = 0;
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);

	
	/*
	 * TEST 11: IP (A3 PI - receiving on I0)
	 *
	 * Ethernet frame 
	 *	- Valid 
	 *
	 * IP packet 
	 *	- Valid length, checksum, version, and TTL 
	 *	- dst: no entry in routing table
	 *
	 * EXPECTED RESULTS: no route
	 *
	 */
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
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 12: IP (A3 PI - receiving on I0)
	 *
	 * Ethernet frame 
	 *	- Valid
	 *
	 * IP packet 
	 *	- Valid length, checksum, version, and TTL 
	 *	- dst: device on network I3 connected to (one hop away) 	
	 *	- No arp entry for device
	 *
	 * EXPECTED RESULTS: no ARP
	 */
    // Ethernet Frame 
    memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x00", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // IP Packet
    memcpy(&ip_test.version_and_ihl, "\x45", 1);
    ip_test.ttl = 10;
    ip_test.protocol = 4;
    memcpy(&ip_test.src_addr, "\x01\x02\x03\x0a", 4);
    memcpy(&ip_test.dst_addr, "\x0d\x0e\x0f\x04", 4);

    // Set data inside of the IP packet
    data_len = 64;
    memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);

    // Set ip header values that depend on the data
    ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
    ip_test.header_checksum = 0;
    ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT

    // Copy all ip_header to ip_test for sending 
    memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
	 * TEST 13: IP (A3 PI - receiving on I0)
	 *
	 * Ethernet frame 
	 *	- Valid
	 *
	 * IP packet 
	 *	- Valid length, checksum, version, and TTL 
	 *	- dst: device on network I3 connected to (one hop away) 	
	 *	- No arp entry for device (which is a router) 
	 *
	 * Notes 
	 *		- Look at drawing
	 *
	 * EXPECTED RESULTS: no ARP
	 */
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
    memcpy(&ip_test.dst_addr, "\x0b\x0c\x0d\x0e", 4);

    // Set data inside of the IP packet
    data_len = 64;
    memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);

    // Set ip header values that depend on the data
    ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
    ip_test.header_checksum = 0;
    ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT

    // Copy all ip_header to ip_test for sending 
    memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header));


    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);


	/*
     * TEST 14: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * IP packet 
     *  - Valid length, checksum, version, and TTL 
     *  - dst: device on network I3 connected to (one hop away) 
     *
     * EXPECTED RESULTS: Run Wireshark on with I3 and should see packet arrive
     */
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
	ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT
	
	// Copy all ip_header to ip_test for sending 
	memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header)); 


	// rest of Ethernet 
	frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
	fcs = crc32(0, frame, frame_len);  
	memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
	frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);


	/*
     * TEST 15: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * IP packet 
     *  - Valid length, checksum, version, and TTL 
     *  - dst: another router since device is not a directly connected network
	 *		   (should be leaving through I3) 
     *
     * EXPECTED RESULTS: Run Wireshark on with I3 and should see packet arrive
     */
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
    memcpy(&ip_test.dst_addr, "\x11\x12\x13\x14", 4);

    // Set data inside of the IP packet
    data_len = 64;
    memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);

    // Set ip header values that depend on the data
    ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
    ip_test.header_checksum = 0;
    ip_test.header_checksum = checksum(&ip_test, 20); // DON'T KNOW IF I SHOULD HARD CODE THAT

    // Copy all ip_header to ip_test for sending 
    memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header));


    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);
   

	/*
     * TEST 16: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * IP packet 
     *  - Valid length, checksum, version, and TTL 
     *  - Invalid IHL
	 *  - dst: another router since device is not a directly connected network
	 *		   (should be leaving through I3) 
     *
     * EXPECTED RESULTS: bad IHL
	 */
    // Ethernet Frame 
    memcpy(test.dst, "\x01\x02\x03\x04\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x00", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // IP Packet
    memcpy(&ip_test.version_and_ihl, "\x44", 1);
    ip_test.ttl = 10;
    ip_test.protocol = 4;
    memcpy(&ip_test.src_addr, "\x01\x02\x03\x00", 4);
    memcpy(&ip_test.dst_addr, "\x11\x12\x13\x14", 4);

    // Set data inside of the IP packet
    data_len = 64;
    memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\xff', data_len);

    // Set ip header values that depend on the data
    ip_test.total_length = htons(sizeof(struct ip_header) + data_len);
    ip_test.header_checksum = 0;
    ip_test.header_checksum = checksum(&ip_test, (ip_test.version_and_ihl & 0x0f) * 32 / 8); // DON'T KNOW IF I SHOULD HARD CODE THAT

    // Copy all ip_header to ip_test for sending 
    memcpy(frame + sizeof(struct ether_header), &ip_test, sizeof(struct ip_header));


    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct ip_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
 

	/*
     * TEST 17: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Incompatible hardware type
	 *
     * EXPECTED RESULTS: incompatible hardware type
	 */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

	// ARP
	memcpy(arp_test.hardware_type, "\x00\x02", 2);
	memcpy(arp_test.protocol_type, "\x08\x00", 2);	
	arp_test.hardware_size = 0x06;
	arp_test.protocol_size = 0x04;
	arp_test.opcode = 0x0001;
	//sender_mac_addr
	//sender_ip_addr
	//target_mac_addr
	//target_ip_addr

	memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
 

	/*
     * TEST 18: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Compatible hardware type
	 *  - Bad hardware size
	 *
     * EXPECTED RESULTS: bad hardware size
	 */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

	// ARP
	memcpy(arp_test.hardware_type, "\x00\x01", 2);
	memcpy(arp_test.protocol_type, "\x08\x00", 2);
	arp_test.hardware_size = 0x07;
	arp_test.protocol_size = 0x04;
	arp_test.opcode = 0x0001;
	//sender_mac_addr
	//sender_ip_addr
	//target_mac_addr
	//target_ip_addr

	memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


    /*
     * TEST 19: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Compatible hardware type and good hardware size
     *  - Incompatible protocol type
     *
     * EXPECTED RESULTS: incompatible protocol type
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x09\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x04;
    arp_test.opcode = 0x0001;
    //sender_mac_addr
    //sender_ip_addr
    //target_mac_addr
    //target_ip_addr

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
	
	/*
     * TEST 20: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Compatible hardware type and good hardware size
     *  - Compatible protocol type
	 *  - Bad protocol size
     *
     * EXPECTED RESULTS: bad protocol size
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x08\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x05;
    arp_test.opcode = htons(0x0001);
    //sender_mac_addr
    //sender_ip_addr
    //target_mac_addr
    //target_ip_addr

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
	
	/*
     * TEST 21: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP  
     *  - Compatible hardware type and good hardware size
     *  - Compatible protocol type and good protocol size
	 *  - Not a request
	 *
	 *	From device A, looking for I0
     *
     * EXPECTED RESULTS: message saying that it's only receiving
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x08\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x04;
    arp_test.opcode = htons(0x0003);
    memcpy(&arp_test.target_ip_addr, "\x01\x02\x03\x04", 4);
	memcpy(arp_test.target_mac_addr, "\x00\x00\x00\x00\x00\x00", 6);
	memcpy(&arp_test.sender_ip_addr, "\x01\x02\x03\x00", 4);
	memcpy(arp_test.sender_mac_addr, "\x11\x22\x33\x00\xff\xff", 6);

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
	
	
	/*
     * TEST 22: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Compatible hardware type and good hardware size
     *  - Compatible protocol type and good protocol size
	 *  - Request
	 *  - Target IP is not the receiving interface
	 *
	 *	From device A, looking for something that is not I0
     *
     * EXPECTED RESULTS: nothing
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x08\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x04;
    arp_test.opcode = htons(0x0001);
    memcpy(&arp_test.target_ip_addr, "\x01\x02\x03\x07", 4);
	memcpy(arp_test.target_mac_addr, "\x00\x00\x00\x00\x00\x00", 6);
	memcpy(&arp_test.sender_ip_addr, "\x01\x02\x03\x00", 4);
	memcpy(arp_test.sender_mac_addr, "\x11\x22\x33\x00\xff\xff", 6);

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);


	/*
     * TEST 23: IP (A3 PI - receiving on I0)
     *
     * Ethernet frame 
     *  - Valid
     *
     * ARP request 
     *  - Compatible hardware type and good hardware size
     *  - Compatible protocol type and good protocol size
	 *  - Request
	 *
	 *	From device A, looking for I0
     *
     * EXPECTED RESULTS: send a reply
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\x11\x22\x33\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x08\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x04;
    arp_test.opcode = htons(0x0001);
    memcpy(&arp_test.target_ip_addr, "\x01\x02\x03\x04", 4);
	memcpy(arp_test.target_mac_addr, "\x00\x00\x00\x00\x00\x00", 6);
	memcpy(&arp_test.sender_ip_addr, "\x01\x02\x03\x00", 4);
	memcpy(arp_test.sender_mac_addr, "\x11\x22\x33\x00\xff\xff", 6);

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    //send_ethernet_frame(fds[1], frame, frame_len);
	// ---------------------------------------------------------------------------------------

	/*
	 * NOTES
	 *		- NEED TO TEST WHEN THERE ARE MULTIPLE ROUTES (VARIED GENMASK LENGTH)
	 */

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
