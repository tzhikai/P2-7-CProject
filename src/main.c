#include <stdio.h>
#include <string.h>
#include <ctype.h>

// example open function
void open_fn() {
	printf("\nOpen sesame!!!\n\n");
}
// example show all function
void showall_fn() {
	printf("\nPretend im listing stuff!!!\n\n");
};

// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	void (*function)();
};

// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", open_fn},
	{"SHOW ALL", showall_fn}
};

void clean_input(char command[]) {
	if (*command == NULL) {
		printf("Null string");
		return;
	}

	printf("Before: command: %s, size: %d\n", command, strlen(command));

	// create a pointer to the end of the command string, move to remove whitespaces
	char* last_char = command + strlen(command) - 1;	// command = memory addr of first letter, -1 to skip null operator
	while (last_char >= command && isspace(*last_char)) {
		last_char--;
	}
	*(last_char + 1) = '\0';	// last_char is last nonwhitespace char, insert null operator after

	char* first_char = command;
	while (isspace(*first_char)) {	// loop will stop at null operator even if command is all whitespaces
		first_char++;
	}
	if (first_char != command) {
		memmove(command, first_char, strlen(first_char) +1);
	}

	printf("After: command: %s, size: %d\n", command, strlen(command));

	//change command to uppercase
	for (int i = 0; command[i] != '\0'; i++) {
		command[i] = toupper(command[i]);
	}

	return;
}

void run_command(char command[]) {
	/*printf("attempting run command %s\n", command);*/
	int size = sizeof(operations) / sizeof(operations[0]);

	for (int i = 0; i < size; i++) {
		if (strcmp(command, operations[i].name) == 0) {	//strcmp returns 0 if equal
			printf("result: %s\n", operations[i].name);
			operations[i].function();
			return;
		}
	}

	// command does not exist or misspelled
	printf("This command %s does not exist.\n", command);
	return;
}

int main() {
	
	char command[20];

	printf("Booting up . . .\n");

	while (1) {
		printf("What would you like to do?: ");
		fgets(command, sizeof(command), stdin);

		// removes leading, trailing whitespaces (incl. \n given by fgets), makes everything uppercase
		clean_input(command);

		/*printf("Running %s command\n", command);*/
		
		if (strcmp(command, "EXIT") == 0) {
			break;
		}

		run_command(command);

		/*printf("Finished %s command\n", command);*/
	}

	printf("Closing program ... bye!");
	return 0;
}