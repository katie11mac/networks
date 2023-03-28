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
	uint32_t num_collisions; // num_collisions - 1 will help us with the range
	uint32_t random_wait;
};

void send_devices(int num_devices);
void set_random_wait_time(struct device *curr_device);

int main(int argc, char *argv[]) {

	int num_devices;

	uint32_t testing;
	uint32_t num;

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

	/*
	// PLAYING AROUND WITH getrandom
	if((random_result = getrandom(&testing, sizeof(testing), 0)) == -1) {
		perror("getrandom");
		// DO WE WANT TO RETURN? 
	}

	//num = 1;
	//printf("random number: %d\n", testing & num);
	num = 3;
	printf("random number: %d\n", testing & num);
	
	testing = (uint32_t)exp2(1);
	printf("testing power: %d\n", testing);
	*/

}

/*
 * Simulate the sending of num_devices devices 
 * sending after all colliding
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
		
	// INCLUDE PRINT STATEMENT FOR WHEN ALL DEVICES WERE ABLE TO SEND

	// Iterate through our array until all devices have sent (while loop)
	// (can check this either through a boolean variable or 
	// counting the number of devices that have sent)
	// 
	//		- For Loop: Count how many are trying to send (random_time 0)
	//	****		****	CAN PROBABLY COMBINE THESE TWO LOOPS *****		*****
	//		- If more than one device is trying to send rn 
	//			- For Loop through devices looking for those with random_time 0: 
	//				- increase their number of collisions 
	//				- have them select a new wait time 
	//		- If one or none devices are trying to send rn 
	//			- Floop Loop through devices looking for those with random_time 0: 
	//				- Mark that device as sent
	//	****			*****			****			****		*****
	//		- For Loop: Need to decrement random times for every single device
	//			- But would we decrement for all devices? Where should this for loop land? 
	//		*** WE ALSO NEED TO FIGURE OUT WHEN TO CHECK IF THE DEVICE REACHED MAX COLLISIONS *** 
	//		- Increament curr_time
	// ********NEED TO THINK CAREFULLY ABOUT UPDATING VARIABLES************ 

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
