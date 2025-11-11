#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"


// cleans up user input command before checking if it can be executed
void clean_input(char command[]) {

	// printf("Before: command: %s, size: %d\n", command, strlen(command));

	// loop for trailing whitespaces
	// create a pointer to the end of the command string, move to remove whitespaces
	char* last_char = command + strlen(command) - 1;	// command = memory addr of first letter, -1 to skip null operator
	while (last_char >= command && isspace(*last_char)) {
		last_char--;
	}
	*(last_char + 1) = '\0';	// last_char is last nonwhitespace char, insert null operator after

	// loop for leading whitespaces
	char* first_char = command;
	while (isspace(*first_char)) {	// loop will stop at null operator even if command is all whitespaces
		first_char++;
	}	// moves memory block starting from first non-whitespace char to command
	if (first_char != command) {
		memmove(command, first_char, strlen(first_char) + 1);	// +1 to include \0
	}

	// printf("After: command: %s, size: %d\n", command, strlen(command));

	return;
}

// Temporary CleanUP for sort_fn: check if it's english word, remove null terminator and Uppercase
void tempclean(char command[]) {
	printf("\nIssued Command: %s\n", command);

	// Remove newline character
	command[strcspn(command, "\n")] = '\0';

	// Remove all spaces from the string
	char* read_ptr = command;
	char* write_ptr = command;

	while (*read_ptr) {
		if (!isspace((unsigned char)*read_ptr)) {
			*write_ptr = toupper((unsigned char)*read_ptr);
			write_ptr++;
		}
		read_ptr++;
	}
	*write_ptr = '\0';

	printf("Cleaned Command: %s\n", command);
	return;
}