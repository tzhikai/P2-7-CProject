#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "data.h"


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

// removes spaces and _s within (& in front of & back of) a string (to join words tgt for comparing similar words)
void join_words(char input[]) {

	char* read_ptr = input;
	char* write_ptr = input;

	// read_ptr pointer goes thru input's memory addr
	while (*read_ptr != '\0') {

		// this rewrites write_ptr's value with read_ptr's value, but only when not _ or space
		if ((*read_ptr != '_') && (*read_ptr != ' ')) {
			*write_ptr++ = *read_ptr;
		}

		//write_ptr++;
		read_ptr++;
	}
	// if a char was overwritten, the string needs to end early
	*write_ptr = '\0';

}

// Counts digits of positive or negative integer by division of 10, returns 1 if 0
int countid(int command) {
	int count = 0;

	command = abs(command);

	if (command == 0) {
		return 1;
	}

	while (command > 0) {
		command /= 10;
		count++;
	}
	return count;
}

