#include "commands.h"

// example open function
void open_fn() {
	printf("\nOpen sesame!!!\n\n");
}
// example show all function
void showall_fn() {
	printf("\nPretend im listing stuff!!!\n\n");
};


// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", open_fn},
	{"SHOW ALL", showall_fn}
};

// handles the execution of operation based on user input command
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