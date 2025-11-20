#ifndef INPUT_H		// checks if INPUT_H has been defined before, and skips it if it has
#define UTILS_H

#include <stdbool.h>

void clean_input(char str[]);
bool is_valid_name(const char* s);
bool is_valid_programme(const char* s);
bool read_line(char* buffer, int size);
bool read_float_range(float* value, float min, float max);


// function prototypes
void clean_input(char command[]);

void join_words(char input[]);

#endif // !INPUT_H
