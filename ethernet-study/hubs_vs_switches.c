/*
 * hubs_vs_switches.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/random.h>

struct device {
	int is_sending;
	int dest_device; // NEED TO MAKE SURE IT DOES NOT GENERATE SELF AS DEST 
};

void set_devices_sending_and_dest(int num_devices, struct device *devices); 
int generate_random_int(void); 
void simulate_hub(int num_time_slots, int num_devices, struct device *devices);

int main(int argc, char *argv[]) {

	int num_devices, num_time_slots;
	struct device *devices;

	int random_number; // REMOVE: USED FOR DEBUGGING 

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

		printf("num_devices: %u\nnum_time_slots: %u\n", num_devices, num_time_slots);

	// Did not provide number of required command line arguments 
	} else {
		
		printf("Please provide two command-line arguments.\nThe first specifying the number of devices and the second specifying number of timeslots.\n");
		return 0;
	
	}

	// Create num_devices device structs and store in array
	if ((devices = (struct device *) malloc(num_devices * sizeof(struct device))) == NULL) {
		printf("malloc failed\n");
	}

	//random_number = generate_random_int();
	//printf("RANDOM NUMBER: %d\n", random_number);
	//printf("RANDOM NUMBER & 0x1: %d\n", random_number & 0x1);
	//printf("RANDOM NUMBER & num_devices: %d\n", random_number & num_devices);
	// I'm struggling to generate the number device for a random destination!!!!!! 
	// BC if we and it, it might not take care of the entire range 
	// if we or it, it might be outside of the range 


	simulate_hub(num_time_slots, num_devices, devices);

//	set_devices_sending_and_dest(num_devices, devices);

	// IDEA FOR NOW: 
	//		Do a function for hub 
	//		Reset the struct of devices (whatever that means) 
	//		Do a function for the switch
	


	// !!!!! DON'T FORGET TO FREE THE MALLOC !!!!!

}

/*
 * Given a struct device *, randomly set all fields in each struct device appropriately.
 * NOTES: 
 *		- Each device has a 50% probability of deciding to send a frame 
 *		- If device devices to send a frame, it randomly chooses which device to send to 
 */
void set_devices_sending_and_dest(int num_devices, struct device *devices) {
	
	// Go through every device in devices
	for (int i = 0; i < num_devices; i++) {
		devices[i].is_sending = generate_random_int() & 0x1;
		// THIS IS WRONG 
		devices[i].dest_device = generate_random_int(); 
		// MAYBE ONLY SET THE DESTINATION DEVICE WHEN is_sending is 1
		//		if is_sending is 0, then set destination device to -1 
		// NEED A WHILE LOOP MAKING SURE THAT DESTINATION RESULT IS NOT ITSELF
	}

}


/*
 * Generate a random int using getrandom
 */
int generate_random_int(void) {
	ssize_t random_result; 
	int random_num; 

	if ((random_result = getrandom(&random_num, sizeof(random_num), 0)) == -1) {
		perror("getrandom");
		// WHAT WOULD WE DO HERE?
	}

	return random_num;
	
}	


/*
 * Simulate the behavoir of a simplified Ethernet hub.
 * Assumes that devices do not attempt to resend in the face of collision. 
 */
void simulate_hub(int num_time_slots, int num_devices, struct device *devices) {

	int total_frames = 0;
	int sent_frames = 0;
	int num_devices_sending;

	// Run for num_time_slots
	for (int i = 0; i < num_time_slots; i++) {
		printf("TIMESLOT %d\n", i);
		
		num_devices_sending = 0;
		
		// Set appropriate values for all devices
		set_devices_sending_and_dest(num_devices, devices);

		// Count how many devices are trying to send
		for (int j = 0; j < num_devices; j++) {
			if (devices[j].is_sending == 1) {
				total_frames += 1; 
				num_devices_sending += 1; 
				printf("\tDEVICE %d TRYING TO SEND\n", j);
			}
		}
	
		// Can successfully send frame if only 1 device is sending 
		if (num_devices_sending == 1) {
			sent_frames += 1;
		
		// Collision detected if more than 1 frame is being sent
		} else if (num_devices_sending > 1) {
			printf("\tCOlLISION: %d devices trying to send at once.\n", num_devices_sending);
		}

	}
	
	printf("HUB SIMULATION RESULTS:\n");
	printf("TOTAL FRAMES SUCESSFULLY SENT: %d\n", sent_frames);
	printf("TOTAL FRAMES ATTEMPTED TO SEND: %d\n", total_frames); 
	printf("%f%% of frames successfully delivered\n", (double)sent_frames / (double)total_frames);

}






