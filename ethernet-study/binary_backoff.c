/*
 * binary-backoff.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <math.h>

struct device {
	int trying_to_send; // should i change this to also represent whether it errored
	uint32_t num_collisions; // num_collisions - 1 will help us with the range
	// FIGURE OUT RELATIONSHIP BETWEEN num_collisions and attempts 
	uint32_t random_wait;
};

void simulate_sending(int num_devices);

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

	simulate_sending(num_devices);

}

void simulate_sending(int num_devices) {
	int num_devices_sent = 0;
	int curr_time = 0;
	
	int num_devices_sending;
	struct device *devices;

	uint32_t testing;
	uint32_t random_num;
	ssize_t random_result;
	uint32_t num;


	// Create n device structs and store in array
	if ((devices = (struct device *) malloc(num_devices * sizeof(struct device))) == NULL) {
		printf("malloc failed\n");
	}
	
	// Iterate through devices array and initialize values for devices
	for (int i = 0; i < num_devices; i++) {
		devices[i].trying_to_send = 1;
		devices[i].num_collisions = 1; // Initalized to 1 bc of our scenario
		// setting the random value
		if((random_result = getrandom(&random_num, sizeof(random_num), 0)) == -1) {
			perror("getrandom");
			// DO WE WANT TO RETURN? 
		}
		// !!!!!! THIS IS WRONG !!!!! need to & with 2 raised to (devices[i].num_collisions) - 1
		// hmmmm but raising something to the power results in a double and we dont want that 
		//num_bits = (uint32_t)exp2(devices[i].num_collisions);
		//		POTENTIAL SOLUTION: change types of num_collisions and random_wait to unsigned int, but then there's ambiguity 
		devices[i].random_wait = random_num & devices[i].num_collisions;

		printf("DEVICE %d\n", i);
		printf("\tinitial sending time: %u\n", devices[i].random_wait);
	}

	printf("----------------------------------------------------\n");

	// lol lets just move on and act like random_wait is correct 

	// Keep looping through all devices until they have all been able to send 
	while (num_devices_sent < num_devices) {

		printf("\nTIME %d\n", curr_time);

		// MAKE SURE YOU ARE NOT CHECKING DEVICES THAT HAVE ALREADY SENT
		//		Note: could technically 

		// Count how many devices are trying to send right now  
		num_devices_sending = 0;
		for (int i = 0; i < num_devices; i++) {
			if ((devices[i].random_wait == -1) && (devices[i].trying_to_send == 1)) {
				num_devices_sending += 1; 		
			}
		}
		

		// Handle devices that are trying to send on next time slot
		for (int i = 0; i < num_devices; i++) {
			
			// Current device wants to send on next time slot 
			if ((devices[i].random_wait == -1) && (devices[i].trying_to_send == 1)) {
				
				// Only one device is trying to send on next time slot 
				if (num_devices_sending < 2) {
					devices[i].trying_to_send = 0;
					num_devices_sent += 1;

					printf("\tSENDING: device %d at time %d\n", i, curr_time);
				
				// More than one device is trying to send on next time slot
				} else {
				
					printf("\tCOLLISION: device %d trying to send at time %d (collision %d)\n", i, curr_time, devices[i].num_collisions);

					devices[i].num_collisions += 1;
					
					// Want to stop trying if has collided more than 10 times
					if (devices[i].num_collisions > 9) {
						devices[i].trying_to_send = 0;
						
						printf("\tERROR: device %d reached max collisions\n", i);
						
						// DON'T KNOW IF num_devices_sent IS GOOD VARIABLE NAME
						num_devices_sent += 1;
					}

					// Generate new random wait time 
					// THIS NEEDS TO GET FIXEDDDD and remember its range increases 
					if((random_result = getrandom(&random_num, sizeof(random_num), 0)) == -1) {
						perror("getrandom");
						// DO WE WANT TO RETURN? 
					}
					devices[i].random_wait = random_num & devices[i].num_collisions;
					printf("\tNEW RANDOM: device %d sending in %u\n", i, devices[i].random_wait); 
				}
			} 
		}
	
		for (int i = 0; i < num_devices; i++) {
			if (devices[i].trying_to_send == 1) {
				devices[i].random_wait -= 1;
			}
		}

		curr_time += 1;	

	}
		

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
