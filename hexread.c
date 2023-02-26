/*
 * hexread.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "util.h"

int main(int argc, char *argv[])
{
	int i;

	// check if no arguments are provided first 
	if (argc == 1) {
		printf("no arguments provided\n");
	} else {

		// files have been provided 
		for (i = 1; i < argc; i++) {
			printf("argument %d: %s", i, argv[i]);	
			// need to open file
			// get and save the size of the file 
			// read contents of the file 
			// pass contents of file to function 
		}
	}
}
