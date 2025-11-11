#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is

#include "commands.h"
#include "data.h"

bool open_fn(char* context) {
	//char cwd[1024];
	//_getcwd(cwd, sizeof(cwd));
	//printf("Current working directory: %s\n", cwd);

	if (context[0] == '\0' || context == NULL) {
		printf("No file name detected, please try again.\n");
		return false;
	}

	// char *filename = strtok_s(NULL, " ", &context);
	char filepath[250] = "src\\data\\";

	//printf("ZK: test context %s\n", context);

	strcat_s(filepath, sizeof(filepath), context);	// concat context gives remaining string (allows filenames w spaces)
	printf("filepath: %s\n", filepath);

	FILE* file_ptr;
	fopen_s(&file_ptr, filepath, "r");
	///*FILE* file_ptr = fopen("C:\\Users\\tzhik\\OneDrive\\Documents\\SIT\\Y1T1\\INF1002 Programming Fundamentals\\C_Half\\P2_7_C_Project\\src\\CMS.txt", "r");*/
	
	// handle possibility that file not found cuz no extension given
	if (file_ptr == NULL && strchr(context, '.') == NULL) {

		const char* extensions[] = {".txt", ".csv"};
		int loop_amt = sizeof(extensions) / sizeof(extensions[0]);
		
		for (int i = 0; i < loop_amt; i++) {
			char filepath_ext[250];	//zktodo: use snprintf?
			strcpy_s(filepath_ext, sizeof(filepath_ext), filepath);
			strcat_s(filepath_ext, sizeof(filepath_ext), extensions[i]);
			fopen_s(&file_ptr, filepath_ext, "r");

			if (file_ptr != NULL) {
				printf("Missing extension, found %s instead\n", filepath_ext);
				break;
			}
		}
		
	}
	// if it still fails, lost cause
	if (file_ptr == NULL) {
		printf("File %s not found.\n", filepath);
		return false;
	}

	printf("The database file \"%s\" is successfully opened.\n", context);	// zktodo: show found extension here
	struct Database* StudentDB = load_data(file_ptr);
	fclose(file_ptr);
	
	if (StudentDB->StudentRecord == NULL) {
		return false;
	}

	set_database(StudentDB);
	return true;
	
}

bool showall_fn(char* context) {
	//printf("\nPretend im listing stuff!!!%s\n\n", context);
	
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL) {
		printf("No records in database.\n");
		return false;
	}

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less
	
	printf("Here are all the records found in the table \"%s\".\n", StudentDB->tableName);

	printf("ID\tName\tProgramme\tMark\n");
	for (int i = 0; i < 3; i++) {
		printf("%d\t%s\t%s\t%f\n", record[i].id, record[i].name, record[i].programme, record[i].mark);
	}

	return true;
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
	bool command_found = false;
	bool command_success = false;


	while (command_ptr && !command_found) {
		if (callphrase[0] != '\0') {
			strcat_s(callphrase, sizeof(callphrase), " ");
		}
		strcat_s(callphrase, sizeof(callphrase), command_ptr);
		// printf("checking %s\n", callphrase);

		for (int i = 0; i < num_of_operations; i++) {
			if (_stricmp(callphrase, operations[i].name) == 0) {
				// printf("%s is equal to %s\n", callphrase, operations[i].name);
				command_found = true;
				command_success = operations[i].function(context);

				/*if (!command_success) {
					printf("Command %s failed to execute properly.\n", operations[i].name);
				}*/

				break;
			}
		}

		command_ptr = strtok_s(NULL, " ", &context);
	}

	if (!command_found) {
		printf("Command %s not recognised. Please try again.\n", command);
	}

	free(command_copy);
	return 0;
}
