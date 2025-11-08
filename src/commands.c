#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is

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

	char* filename = strtok_s(NULL, " ", &context);
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
		printf("%d\t%s\t%s\t%f\n", StudentRecord[i].id, StudentRecord[i].name, StudentRecord[i].programme, StudentRecord[i].mark);
	}

	return true;
};



// INSERT the new students
bool insert_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	struct Student newStudent;
	printf("Enter Student ID: ");
	scanf_s("%d", &newStudent.id);

	if (find_student_by_id(StudentDB, newStudent.id) != NULL) {
		printf("CMS: The record with ID=%d already exists.\n", newStudent.id);
		return false;
	}

	printf("Enter Name: ");
	scanf_s(" %[^\n]", newStudent.name, (unsigned)_countof(newStudent.name));
	printf("Enter Programme: ");
	scanf_s(" %[^\n]", newStudent.programme, (unsigned)_countof(newStudent.programme));
	printf("Enter Mark: ");
	scanf_s("%f", &newStudent.mark);

	add_student(StudentDB, newStudent);

	char filepath[250] = "src\\CMS.txt"; // default save path
	save_database(StudentDB, filepath);
	printf("CMS: Record saved automatically to %s\n", filepath);
	return true;
}

// QUERY 
bool query_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	int id;
	printf("Enter Student ID to query: ");
	scanf_s("%d", &id);

	struct Student* student = find_student_by_id(StudentDB, id);
	if (student == NULL) {
		printf("CMS: The record with ID=%d does not exist.\n", id);
		return false;
	}

	printf("CMS: The record with ID=%d is found.\n", id);
	printf("ID\tName\tProgramme\tMark\n");
	printf("%d\t%s\t%s\t%.2f\n",
		student->id, student->name, student->programme, student->mark);
	return true;
}

// SAVE the records
bool save_fn(char* context) {
	struct Database* StudentDB = get_database();
	if (StudentDB == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	char filepath[250] = "src\\CMS.txt"; // default save path
	save_database(StudentDB, filepath);
	return true;
}

struct operation operations[] = {
	{"OPEN", 1, open_fn},
	{"SHOW ALL", 2, showall_fn},
	{"INSERT", 3, insert_fn},
	{"QUERY", 4, query_fn},
	{"SAVE", 5, save_fn}
};

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
