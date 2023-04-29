/*
 * testing-i3.c
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
    char *local_vde_cmd[] = { "vde_plug", "/tmp/net3.vde", NULL };
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
     * TEST 15: IP (A3 PI - receiving on I3)
     *
     * Ethernet frame 
     *  - Valid
     *
     * IP packet 
     *  - Valid length, checksum, version, and TTL 
     *  - dst: another router since device is not a directly connected network
     *         (should be leaving through I3) 
     *
     * EXPECTED RESULTS: Run Wireshark on with I3 and I1 and 
	 *					should see packet arrive on I1
     */
    // Ethernet Frame 
    memcpy(test.dst, "\x0d\x0e\x0f\x10\xff\xff", 6);
    memcpy(test.src, "\xdd\xee\xff\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x00", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // IP Packet
    memcpy(&ip_test.version_and_ihl, "\x45", 1);
    ip_test.ttl = 10;
    ip_test.protocol = 4;
    memcpy(&ip_test.src_addr, "\x0d\x0e\x0f\x00", 4);
    memcpy(&ip_test.dst_addr, "\x01\x02\x03\x00", 4);

    // Set data inside of the IP packet
    data_len = 64;
    memset(frame + sizeof(struct ether_header) + sizeof(struct ip_header), '\x00', data_len);

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
     *  From device H, looking for I3
     *
     * EXPECTED RESULTS: ARP reply
     */
    // Ethernet Frame 
    memcpy(test.dst, "\xff\xff\xff\xff\xff\xff", 6);
    memcpy(test.src, "\xdd\xee\xff\x00\xff\xff", 6);
    memcpy(test.type, "\x08\x06", 2);
    memcpy(frame, &test, sizeof(struct ether_header));

    // ARP
    memcpy(arp_test.hardware_type, "\x00\x01", 2);
    memcpy(arp_test.protocol_type, "\x08\x00", 2);
    arp_test.hardware_size = 0x06;
    arp_test.protocol_size = 0x04;
    arp_test.opcode = htons(0x0001);
    memcpy(&arp_test.target_ip_addr, "\x0d\x0e\x0f\x10", 4);
    memcpy(arp_test.target_mac_addr, "\x00\x00\x00\x00\x00\x00", 6);
    memcpy(&arp_test.sender_ip_addr, "\x0d\x0e\x0f\x00", 4);
    memcpy(arp_test.sender_mac_addr, "\xdd\xee\xff\x00\xff\xff", 6);

    memcpy(frame + sizeof(struct ether_header), &arp_test, sizeof(struct arp_header));

    // rest of Ethernet 
    frame_len = sizeof(struct ether_header) + sizeof(struct arp_header) + data_len;
    fcs = crc32(0, frame, frame_len);
    memcpy(frame + frame_len, &fcs, sizeof(uint32_t));
    frame_len += sizeof(uint32_t);
    printf("sending frame, length %ld\n", frame_len);
    send_ethernet_frame(fds[1], frame, frame_len);


	//----------------------------------------------------------------------------------

	printf("Press Control-C to terminate sender.\n");
    pause();

    return 0;
}
