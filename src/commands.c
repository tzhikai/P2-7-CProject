#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "commands.h"
#include "data.h"

// example open function
void open_fn(char* context) {
	char *filename = strtok_s(NULL, " ", &context);

	printf("filename: %s\n", filename);

	FILE *file_ptr = fopen(filename, "r");

	if (file_ptr == NULL) {
		printf("File %s not found.\n", filename);
	}
	else {
		printf("File found!\n");
	}

	return;
}
// example show all function
void showall_fn() {
	printf("\nPretend im listing stuff!!!\n\n");
};

// num_extract eg. open = 1, show all = 2
struct commandSplit split_command(char command[], int num_extract) {
	struct commandSplit result = { "", NULL, NULL };	// Local, callphrase and remainder ptr buffer

	int callphrase_limit = 1;	// counts until num_extract
	//char* context = NULL;		// allows strtok_s to store its curr position within the string

	result.context = NULL;

	result.copy = _strdup(command);	// copy of command since strtok_s transforms its input
	char* split_ptr = strtok_s(result.copy, " ", &result.context);

	if (split_ptr == NULL) {
		strcat_s(result.callphrase, sizeof(result.callphrase), command);
	}
	else {
		while (split_ptr != NULL && callphrase_limit <= num_extract) {
			/*printf("token: %s\n", split_cmd);*/
			if (callphrase_limit > 1) {
				split_ptr = strtok_s(NULL, " ", &result.context);  // NULL because strtok_s remembers the current string its tokenising
				strcat_s(result.callphrase, sizeof(result.callphrase), " ");
			}
			strcat_s(result.callphrase, sizeof(result.callphrase), split_ptr);
			callphrase_limit++;
				
		}
	}

	printf("within:\ncommand:%s\nremainder:%s\n", result.callphrase, result.context);
	return result;
};

// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", 1, open_fn},
	{"SHOW ALL", 2, showall_fn}
};

// handles the execution of operation based on user input command
bool run_command(char command[]) {
	// passed in command, without trailing or leading whitespaces
	
	char* context = NULL;
	char* command_copy = _strdup(command);

	char* command_ptr = strtok_s(command_copy, " ", &context);
	char callphrase[20] = "";

	int num_of_operations = sizeof(operations) / sizeof(operations[0]);
	bool command_found = 0;

	// while command_ptr is not null
	// append command_ptr to callphrase
	// then loop thru operations with for loop
	// check for operation equal
	// then run

	while (command_ptr && !command_found) {
		if (callphrase[0] != '\0') {
			strcat_s(callphrase, sizeof(callphrase), " ");
		}
		strcat_s(callphrase, sizeof(callphrase), command_ptr);
		printf("checking %s\n", callphrase);

		for (int i = 0; i < num_of_operations; i++) {
			if (_stricmp(callphrase, operations[i].name) == 0) {
				printf("%s is equal to %s\n", callphrase, operations[i].name);
				command_found = 1;
				operations[i].function(context);
			}
		}

		command_ptr = strtok_s(NULL, " ", &context);
	}

	return command_found;
}
