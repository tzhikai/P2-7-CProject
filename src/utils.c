#include "utils.h"


// cleans up user input command before checking if it can be executed
void clean_input(char command[]) {

	printf("Before: command: %s, size: %d\n", command, strlen(command));

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

	printf("After: command: %s, size: %d\n", command, strlen(command));

	//change command to uppercase
	for (int i = 0; command[i] != '\0'; i++) {
		command[i] = toupper(command[i]);
	}

	return;
}