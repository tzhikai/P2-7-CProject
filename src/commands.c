#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"
#include "data.h"

// example open function
void open_fn(char** file) {
	printf("\nOpen sesame!!!\n\n");
	printf("test:%s\n", strtok_s(NULL, " ", &file));	// here!!!
	printf("test:%s\n", strtok_s(NULL, " ", &file));	// here!!!
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
void run_command(char command[]) {
	// split input by space
	//struct commandSplit split = split_command(command, 2);
	//printf("outside:\ncommand:%s\nremainder:%s\n", split.callphrase, split.context);
	//printf("test:%s\n", strtok_s(NULL, " ", &split.context));	// here!!!
	//printf("test:%s\n", strtok_s(NULL, " ", &split.context));	// here!!!

	// use pointer from split command to print rest of inputs

	/*printf("attempting run command %s\n", command);*/
	int size = sizeof(operations) / sizeof(operations[0]);
	int num_extract;

	for (int i = 0; i < size; i++) {
		num_extract = operations[i].wordCount;
		struct commandSplit split = split_command(command, num_extract);	// inefficient, since it recalculates multiple times, need rethinking
		if (strcmp(split.callphrase, operations[i].name) == 0) {	//strcmp returns 0 if equal
			printf("result: %s\n", operations[i].name);
			operations[i].function(split.context);
			return;
		}
	}

	// free split.copy?

	// command does not exist or misspelled
	printf("This command %s does not exist.\n", command);
	return;
}