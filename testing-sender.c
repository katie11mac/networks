/*
 * testing-sender.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "cs431vde.h"


struct ether_header {
    uint8_t dst[6];
    uint8_t src[6];
    uint8_t type[2];
};

uint32_t crc32(uint32_t crc, const void *buf, size_t size);

int main(int argc, char *argv[])
{
    int fds[2];

    uint8_t frame[1600];
    ssize_t frame_len;

    int connect_to_remote_switch = 0;
    char *local_vde_cmd[] = { "vde_plug", NULL };
    char *remote_vde_cmd[] = { "ssh", "pjohnson@weathertop.cs.middlebury.edu",
                                      "/home/pjohnson/cs431/bin/vde_plug",
                                      NULL };
    char **vde_cmd = connect_to_remote_switch ? remote_vde_cmd : local_vde_cmd;
	
	struct ether_header test;
	ssize_t data_len;
	uint32_t fcs;

    if(connect_to_vde_switch(fds, vde_cmd) < 0) {
        printf("Could not connect to switch, exiting.\n");
        exit(1);
    }


	// TEST 1: Send a broadcast with valid FCS 
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

	// TEST 2: Send a broadcast with invalid FCS
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

	// TEST 3: Send frame for me with valid FCS
	memcpy(test.dst, "\x86\x46\x6c\x7e\xff\x1a", 6);
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

	// TEST 4: Send frame not for me with a valid fcs
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
