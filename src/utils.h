#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

// hy proto
void clean_input(char str[]);
bool is_valid_name(const char* s);
bool read_line(char* buffer, int size);
bool read_optional_string(char* buffer, int size, const char* prompt);
bool read_optional_mark(float* result, const char* prompt);


// function prototypes
void clean_input(char command[]);

void join_words(char input[]);

#endif // !INPUT_H
