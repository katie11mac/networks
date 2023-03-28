/*
 * binary-backoff.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <math.h>

struct device {
	int trying_to_send;
	uint32_t num_collisions; 
	uint32_t random_wait;
};

void send_devices(int num_devices);
void set_random_wait_time(struct device *curr_device);

int main(int argc, char *argv[]) {

	int num_devices;

	// Read number of devices from command line 
	if (argc == 2) {
		
		if (sscanf(argv[1], "%d", &num_devices) == 0) {
			printf("Invalid number of devices.\n");
			return 0;
		}
		
		if (num_devices < 1) {
			printf("Invalid number of devices.\n"); 
			return 0;
		}

		printf("Starting simulation for %d devices...\n", num_devices);
	
	} else {
		
		printf("Please provide one command-line argument specifying the number of devices to simulate.\n"); 
		return 0;
	
	}

	send_devices(num_devices);
}


/*
 * Simulate the sending of num_devices devices sending after all colliding
 */
void send_devices(int num_devices) {
	int num_devices_completed = 0;
	int curr_time = 0;
	int num_devices_sending;
	struct device *devices;

	// Create num_devices device structs and store in array
	if ((devices = (struct device *) malloc(num_devices * sizeof(struct device))) == NULL) {
		printf("malloc failed\n");
	}
	
	// Iterate through and initialize devices array
	for (int i = 0; i < num_devices; i++) {
		devices[i].trying_to_send = 1;
		devices[i].num_collisions = 1; // 1 because scenario simulates all devices colliding 
		// Set random wait time
		set_random_wait_time(&devices[i]);

		printf("DEVICE %d\n", i);
		printf("\tinitial sending time: %u\n", devices[i].random_wait);
	}

	printf("----------------------------------------------------\n");

	// Keep looping through all devices until they have all have sent or errored 
	while (num_devices_completed < num_devices) {

		printf("\nTIME %d\n", curr_time);

		// Count how many devices are trying to send right now and have not sent already
		num_devices_sending = 0;
		for (int i = 0; i < num_devices; i++) {
			if ((devices[i].random_wait == -1) && (devices[i].trying_to_send == 1)) {
				num_devices_sending += 1; 		
			}
		}

		// Handle devices that are trying to send right now
		for (int i = 0; i < num_devices; i++) {
			
			// Current device wants to send right now 
			if ((devices[i].random_wait == -1) && (devices[i].trying_to_send == 1)) {
				
				// Only one device is trying to send right now
				if (num_devices_sending < 2) {
					devices[i].trying_to_send = 0;
					num_devices_completed += 1;

					printf("\tSENDING: device %d at time %d\n", i, curr_time);
				
				// More than one device is trying to send right now
				} else {
				
					devices[i].num_collisions += 1;
					
					printf("\tCOLLISION: device %d trying to send at time %d (collision %d)\n", i, curr_time, devices[i].num_collisions);

					// Want to stop trying if has collided more than 10 times
					if (devices[i].num_collisions > 9) {
						devices[i].trying_to_send = 0;
						
						printf("\tERROR: device %d reached max collisions\n", i);
						
						num_devices_completed += 1;

					// Otherwise generate a new random wait time
					} else {
						// Generate new random wait time 
						set_random_wait_time(&devices[i]);

						printf("\tNEW RANDOM: device %d sending in %u\n", i, devices[i].random_wait); 
					}
				}
			} 
		}
	
		// Decrement wait time for all devices trying to send
		for (int i = 0; i < num_devices; i++) {
			if (devices[i].trying_to_send == 1) {
				devices[i].random_wait -= 1;
			}
		}

		curr_time += 1;	
	}
	
	free(devices);
}


/*
 * Generate random wait time for curr_device using getrandom 
 */
void set_random_wait_time(struct device *curr_device) {
	ssize_t random_result;
	uint32_t random_num;	
	uint32_t wait_range;

	// Set random wait time
	if((random_result = getrandom(&random_num, sizeof(random_num), 0)) == -1) {
		perror("getrandom");
		// DO WE WANT TO RETURN? 
	}
	// Use bit manipulation to find 2 to the num_collisions power, which is the wait range 
	wait_range = (1 << curr_device->num_collisions) - 1;
	//printf("\t\tRANGE UP TO: %u\n", wait_range);
	curr_device->random_wait = random_num & wait_range;
}
