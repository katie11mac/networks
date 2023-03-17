/*
 * binary-backoff.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>

struct device {
	int has_sent;
	uint32_t num_collisions; // num_collisions - 1 will help us with the range
	uint32_t random_wait;
};

int main(int argc, char *argv[]) {

	int all_devices_sent = 0;
	uint32_t num_devices;
	uint32_t curr_time;

	uint32_t testing;
	ssize_t random_result;
	uint32_t num;
	
	if (argc == 2) {
		num_devices = atoi(argv[1]);
		printf("Starting simulation for %d devices...\n", num_devices);
	} else {
		printf("Please provide one command-line argument specifying the number of devices to simulate\n"); 
		return 0;
	}

	if((random_result = getrandom(&testing, sizeof(testing), 0)) == -1) {
		perror("getrandom");
		// DO WE WANT TO RETURN? 
	}

	num = 1;
	printf("random number: %d\n", testing & num);


	// what if we randomly generate the number and store it in random_wait, but only
	// read the lowest num_collisions bits 
	//		to read the lowest num_collision bits, would we & it with that number 

	// Malloc n * size of struct to hold an array of devices 
	// for each i in n, we are gonna want to go to the &array[i] to initalize all the devices 
	
	// We need to think more about the generating random numbers to really move on

}
