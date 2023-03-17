/*
 * binary-backoff.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <math.h>

struct device {
	int has_sent;
	uint32_t num_collisions; // num_collisions - 1 will help us with the range
	uint32_t random_wait;
};

int main(int argc, char *argv[]) {

	int all_devices_sent = 0;
	uint32_t curr_time = 0;
	uint32_t num_devices;
	struct device *devices;

	uint32_t testing;
	uint32_t random_num;
	ssize_t random_result;
	uint32_t num_bits;
	uint32_t num;

	int num_devices_sent = 0;
	
	if (argc == 2) {
		num_devices = atoi(argv[1]);
		printf("Starting simulation for %d devices...\n", num_devices);
	} else {
		printf("Please provide one command-line argument specifying the number of devices to simulate\n"); 
		return 0;
	}

	// Create n device structs and store in array
	if ((devices = (struct device *) malloc(num_devices * sizeof(struct device))) == NULL) {
		printf("malloc failed\n");
	}
	
	// Iterate through devices array and initialize values for devices
	for (int i = 0; i < num_devices; i++) {
		devices[i].has_sent = 0;
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
	}
	
	// lol lets just move on and act like random_wait is correct 
	

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
	//		- For Loop: Need to decrement random times 
	//			- But would we decrement for all devices? Where should this for loop land? 
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
