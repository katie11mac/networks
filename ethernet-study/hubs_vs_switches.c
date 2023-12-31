/*
 * hubs_vs_switches.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/random.h>

struct device {
	uint32_t is_sending;
	uint32_t dest_device;  
};

void set_devices_sending_and_dest(int num_devices, struct device *devices); 
uint32_t generate_random_uint(void); 
void simulate_hub(int num_time_slots, int num_devices, struct device *devices);
void simulate_switch(int num_time_slots, int num_devices, struct device *devices);

int main(int argc, char *argv[]) {

	int num_devices, num_time_slots;
	struct device *devices;

	// Provided required number of command line arguments
	if (argc == 3) {
	
		// Get num_devices from argv and check it is an integer
		if (sscanf(argv[1], "%d", &num_devices) == 0) {
			printf("Invalid number of devices.\n");
			return 0;
		}

		// Check num_devices is positive
		if (num_devices < 1) {
			printf("Invalid number of devices.\n");
			return 0;
		}

		// Get num_time_slots from argv and check it is an integer
		if (sscanf(argv[2], "%d", &num_time_slots) == 0) {
			printf("Invalid number of time slots.\n");
			return 0;
		}

		// Check num_time_slots is positive
		if (num_time_slots < 1) {
			printf("Invalid number of devices.\n");
			return 0;
		}

	// Did not provide number of required command line arguments 
	} else {
	
		printf("Please provide two command-line arguments.\nThe first specifying the number of devices and the second specifying number of timeslots.\n");
		return 0;
	
	}

	// Create num_devices device structs and store in array
	if ((devices = (struct device *) malloc(num_devices * sizeof(struct device))) == NULL) {
		printf("malloc failed\n");
	}

	simulate_hub(num_time_slots, num_devices, devices);
	simulate_switch(num_time_slots, num_devices, devices);

	free(devices);
}

/*
 * Given a struct device *, randomly set all fields in each struct device appropriately.
 * NOTES: 
 *		- Each device has a 50% probability of deciding to send a frame 
 *		- If device devices to send a frame, it randomly chooses which device to send to 
 */
void set_devices_sending_and_dest(int num_devices, struct device *devices) {
	
	uint32_t generated_dest; 

	// Go through every device in devices
	for (int i = 0; i < num_devices; i++) {
		
		// Randomly decide whether device i wants to send a frame
		devices[i].is_sending = generate_random_uint() & 1;
		
		// If device i is sending, generate new random destination
		if (devices[i].is_sending == 1) {
			generated_dest = generate_random_uint() % num_devices;
			
			// If generates self as destination and it's the first device
			if ((generated_dest == 0) && (i == 0)) {
				generated_dest += 1; 
			
			// If it generated self as destination
			} else if (generated_dest == i) {
				generated_dest -= 1;
			}

			devices[i].dest_device = generated_dest;
		
		}
	}
}


/*
 * Generate a random uint using getrandom
 */
uint32_t generate_random_uint(void) {
	ssize_t random_result; 
	uint32_t random_num; 

	if ((random_result = getrandom(&random_num, sizeof(random_num), 0)) == -1) {
		perror("getrandom");
		// WHAT WOULD WE DO HERE?
	}

	return random_num;
}	


/*
 * Simulate the behavior of a simplified Ethernet hub.
 * Assumes that devices do not attempt to resend in the face of collision. 
 */
void simulate_hub(int num_time_slots, int num_devices, struct device *devices) {

	int total_frames = 0;
	int sent_frames = 0;
	int num_devices_sending;

	// Run for num_time_slots
	for (int i = 0; i < num_time_slots; i++) {
		
		num_devices_sending = 0;
		
		// Set appropriate values for all devices
		set_devices_sending_and_dest(num_devices, devices);

		// Count how many devices are trying to send
		for (int j = 0; j < num_devices; j++) {
			if (devices[j].is_sending == 1) {
				total_frames += 1; 
				num_devices_sending += 1; 
			}
		}
	
		// Can successfully send frame if only 1 device is sending 
		if (num_devices_sending == 1) {
			sent_frames += 1;
		}
	}
	
	printf("Hub Simulation Results:\n");
	printf("\t%f%% of frames successfully delivered\n", ((double)sent_frames / (double)total_frames) * 100);
}

/*
 * Simulate the behavior of a simple switch. 
 * Assumes devices do not attempt to resend in the face of collision.
 * Does not buffer any frames if they are unable to immediately deliver them 
 * (ie, the switch silently discards the frame).
 * Assumes switch knows which port each device is connected to before the simulation begins.
 */
void simulate_switch(int num_time_slots, int num_devices, struct device *devices) {
	// NOTE: This 1-D array appraoch gets rid of information about the source of the frame.
	// While this information may not be relevant here, it might be needed if we needed to populate
	// our own table of ports for each devices. To retain this information maybe use a 2-D array?
	
	int total_frames = 0;
	int sent_frames = 0;
	int dest_device_counts[num_devices]; 
	// index of dest_device_counts represents destination device 
	// values represent how many devices trying to send to that host 

	// Run for num_time_slots
	for (int i = 0; i < num_time_slots; i++) {
		
		// Initialize/Reset dest_device_counts array 
		for (int j = 0; j < sizeof(dest_device_counts) / sizeof(dest_device_counts[0]); j++) {
			dest_device_counts[j] = 0;
		}
		
		// Set appropriate values for all devices
		set_devices_sending_and_dest(num_devices, devices);
		
		// Count how many devices are sending and their destinations 
		for (int j = 0; j < num_devices; j++) {
			if (devices[j].is_sending == 1) {
				total_frames += 1; 
				dest_device_counts[(devices[j].dest_device)] += 1; 
			}
		}
	
		// Check how many devices are trying to send to each device 
		for (int j = 0; j < sizeof(dest_device_counts) / sizeof(dest_device_counts[0]); j++) {
			// Can only send that constructed frame if only one device is sending to that destination
			if (dest_device_counts[j] == 1) {
				sent_frames += 1;
			}
		}
	}
	
	printf("Switch Simulation Results: \n");
	printf("\t%f%% of frames successfully delivered\n", ((double)sent_frames / (double)total_frames) * 100);

}






