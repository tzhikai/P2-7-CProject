#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is
#include <float.h> // Needed for FLT_MAX and FLT_MIN

#include "commands.h"
#include "data.h"

// example open function
bool open_fn(char* context) {
	//char cwd[1024];
	//_getcwd(cwd, sizeof(cwd));
	//printf("Current working directory: %s\n", cwd);

	if (context[0] == '\0' || context == NULL) {
		printf("No file name detected, please try again.\n");
		return false;
	}

	char *filename = strtok_s(NULL, " ", &context);
	char filepath[250] = "src\\";

	strcat_s(filepath, sizeof(filepath), filename);
	printf("filepath: %s\n", filepath);

	FILE* file_ptr;
	fopen_s(&file_ptr, filepath, "r");
	///*FILE* file_ptr = fopen("C:\\Users\\tzhik\\OneDrive\\Documents\\SIT\\Y1T1\\INF1002 Programming Fundamentals\\C_Half\\P2_7_C_Project\\src\\CMS.txt", "r");*/
	if (file_ptr == NULL) {
		printf("File %s not found.\n", filepath);
		return false;
	}

	printf("The database file \"%s\" is successfully opened.\n", filename);
	/*struct Student* StudentRecord = load_data(file_ptr);*/
	struct Database* StudentDB = load_data(file_ptr);
	fclose(file_ptr);
	
	if (StudentDB->StudentRecord == NULL) {
		return false;
	}

	set_database(StudentDB);
	return true;
	
}
// example show all function
bool showall_fn(char* context) {
	//printf("\nPretend im listing stuff!!!%s\n\n", context);
	
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL) {
		printf("No records in database.\n");
		return false;
	}

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less
	
	printf("Here are all the records found in the table \"<insert table name>\".\n");

	printf("ID\tName\tProgramme\tMark\n");
	for (int i = 0; i < 3; i++) {
		printf("%d\t%s\t%s\t%f\n", record[i].id, record[i].name, record[i].programme, record[i].mark);
	}

	return true;
};

bool save_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL) {
		printf("CMS: No database loaded. Please OPEN one first.\n");
		return false;
	}

	saveDatabase("src\\CMS.txt"); // Adjust path if needed
	return true;
}

bool summary_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL) {
		printf("CMS: No database loaded. Please OPEN one first.\n");
		return false;
	}

	showSummary();
	return true;
}

// save function
void saveDatabase(const char* filename) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL || StudentDB->StudentRecord == NULL) {
		printf("CMS: No database loaded.\n");
		return;
	}

	FILE* file;
	fopen_s(&file, filename, "w");
	if (file == NULL) {
		printf("CMS: Error opening file for writing.\n");
		return;
	}

	for (int i = 0; i < StudentDB->size; i++) {
		fprintf(file, "%d %s %s %.1f\n",
			StudentDB->StudentRecord[i].id,
			StudentDB->StudentRecord[i].name,
			StudentDB->StudentRecord[i].programme,
			StudentDB->StudentRecord[i].mark);
	}

	fclose(file);
	printf("CMS: The database file \"%s\" is successfully saved.\n", filename);
}

//update
bool update_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL || StudentDB->StudentRecord == NULL) {
		printf("CMS: No database loaded. Please OPEN one first.\n");
		return false;
	}

	int targetID;
	printf("CMS: Enter the student ID to update: ");
	scanf_s("%d", &targetID);

	// search for the record
	int foundIndex = -1;
	for (int i = 0; i < StudentDB->size; i++) {
		if (StudentDB->StudentRecord[i].id == targetID) {
			foundIndex = i;
			break;
		}
	}

	if (foundIndex == -1) {
		printf("CMS: The record with ID=%d does not exist.\n", targetID);
		return false;
	}

	// show current data
	printf("CMS: Record found.\n");
	printf("ID: %d | Name: %s | Programme: %s | Mark: %.1f\n",
		StudentDB->StudentRecord[foundIndex].id,
		StudentDB->StudentRecord[foundIndex].name,
		StudentDB->StudentRecord[foundIndex].programme,
		StudentDB->StudentRecord[foundIndex].mark);

	// ask for new info
	printf("\nEnter new name (no spaces): ");
	scanf_s("%s", StudentDB->StudentRecord[foundIndex].name, (unsigned)_countof(StudentDB->StudentRecord[foundIndex].name));

	printf("Enter new programme (no spaces): ");
	scanf_s("%s", StudentDB->StudentRecord[foundIndex].programme, (unsigned)_countof(StudentDB->StudentRecord[foundIndex].programme));

	printf("Enter new mark: ");
	scanf_s("%f", &StudentDB->StudentRecord[foundIndex].mark);

	printf("CMS: The record with ID=%d is successfully updated.\n", targetID);
	return true;
}

//show summary
void showSummary() {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL || StudentDB->size == 0) {
		printf("CMS: No records found.\n");
		return;
	}

	float total = 0;
	float highest = -FLT_MAX;
	float lowest = FLT_MAX;
	int hiIndex = 0, loIndex = 0;

	for (int i = 0; i < StudentDB->size; i++) {
		float m = StudentDB->StudentRecord[i].mark;
		total += m;
		if (m > highest) { highest = m; hiIndex = i; }
		if (m < lowest) { lowest = m; loIndex = i; }
	}

	printf("CMS: Summary Statistics\n");
	printf("Total number of students: %d\n", StudentDB->size);
	printf("Average mark: %.2f\n", total / StudentDB->size);
	printf("Highest mark: %.1f (%s)\n", highest, StudentDB->StudentRecord[hiIndex].name);
	printf("Lowest mark: %.1f (%s)\n", lowest, StudentDB->StudentRecord[loIndex].name);
}

// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", 1, open_fn},
	{"SHOW ALL", 2, showall_fn},
	{"SAVE", 1, save_fn},
	{"SHOW SUMMARY", 2, summary_fn},
	{ "UPDATE", 1, update_fn }
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
