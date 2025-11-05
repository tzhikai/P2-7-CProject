#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is

#include "commands.h"
#include "data.h"

// example open function
struct Student *open_fn(char* context) {
	//char cwd[1024];
	//_getcwd(cwd, sizeof(cwd));  // Use getcwd() on Linux/Mac
	//printf("Current working directory: %s\n", cwd);

	if (context[0] == '\0' || context == NULL) {
		printf("No file name detected, please try again.\n");
		return NULL;
	}

	char *filename = strtok_s(NULL, " ", &context);
	char filepath[250] = "src\\";

	strcat_s(filepath, sizeof(filepath), filename);
	printf("filepath: %s\n", filepath);


	FILE *file_ptr = fopen(filepath, "r");
	///*FILE* file_ptr = fopen("C:\\Users\\tzhik\\OneDrive\\Documents\\SIT\\Y1T1\\INF1002 Programming Fundamentals\\C_Half\\P2_7_C_Project\\src\\CMS.txt", "r");*/

	if (file_ptr == NULL) {
		printf("File %s not found.\n", filepath);
		return NULL;
	}
	else {
		printf("The database file \"%s\" is successfully opened.\n", filename);
		struct Student* StudentRecord = load_data(file_ptr);
		fclose(file_ptr);
		return StudentRecord;
	}
}
// example show all function
struct Student* showall_fn(char* context) {
	printf("\nPretend im listing stuff!!!%s\n\n", context);
	return NULL;
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
				struct Student* StudentRecord = operations[i].function(context);
			}
		}

		command_ptr = strtok_s(NULL, " ", &context);
	}

	return command_found;
}
