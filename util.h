#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char *binary_to_hex(void *data, ssize_t n);
void *hex_to_binary(char *hex);
ssize_t hex_digit_to_binary (char hex_digit); 

#endif /* __MYIO_H */
