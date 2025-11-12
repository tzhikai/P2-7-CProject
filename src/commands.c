#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is
#include <ctype.h>

#include "commands.h"
#include "data.h"
#include "sort.h"
#include "utils.h"

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

	printf("StudentDB->size: %d\n", StudentDB->size);

	printf("Here are all the records found in the table \"<insert table name>\".\n");

	printf("ID\tName\tProgramme\tMark\n");
	for (int i = 0; i < StudentDB->size; i++) {
		printf("%d\t%s\t%s\t%f\n", record[i].id, record[i].name, record[i].programme, record[i].mark);
	}

	return true;
};

// jaison deletee function 2501161[7] + 3 = 10
struct Database* delete_fn(char* context) {
	char cnfm[6], idbuffer[10];
	int cnfmdeleting = 0, deleting = 1;
	while (deleting == 1) {
		printf("\nID to Delete: ");

		fgets(idbuffer, sizeof(idbuffer), stdin);
		tempclean(idbuffer); //Accept id as string for cleanup
		//printf("\nidbuffer: %s", idbuffer);
		int iddelete = atoi(idbuffer); //Convert string to int, if string is not integer, atoi returns 0
		//printf("\niddelete: %d", iddelete);
		int count = countid(iddelete); //Counts digits in userinput, if input is 0 (or atoi returns 0)
		//printf("\ncount: %d", count);

		//if digit is less/more than 7, invalid id
		if (count != 7) {
			printf("\nPlease enter a valid ID");
			continue;
		}

		struct Database* StudentDB = get_database();

		if (StudentDB == NULL) {
			printf("\nNo records in database.");
			break;
		}

		struct Student* record = StudentDB->StudentRecord;
		struct Database* NEWdb = malloc(sizeof(struct Database));
		NEWdb->StudentRecord = malloc(sizeof(struct Student) * (StudentDB->size - 1));
		NEWdb->memory = sizeof(struct Database);
		NEWdb->size = StudentDB->size - 1;
		int newindex = 0, indexdelete = -1;
		struct Student* NEWrecord = NEWdb->StudentRecord;

		if (NEWdb == NULL) {
			printf("\nNEWrecord Memory Allocation Failed.");
			break;
		}
		if (NEWrecord == NULL) {
			printf("NEWrecord Memory allocation for StudentRecord failed.\n");
			break;
		}

		for (int i = 0; i < StudentDB->size; i++) {
			if (record[i].id == iddelete) {
				printf("\nFound record ID=%d at Index=%d", iddelete, i);
				indexdelete = i;
				cnfmdeleting = 1;
				break;
			}
		}

		if (indexdelete == -1) {
			printf("\nThe record with ID=%d does not exist", iddelete);
			deleting = 0;
			break;
		}

		while (cnfmdeleting == 1) {
			printf("\nAre you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to Cancel: ", iddelete);
			fgets(cnfm, sizeof(cnfm), stdin);
			tempclean(cnfm);

			printf("\nSize of Original Database: %d", StudentDB->size);
			printf("\nSize of New Database: %d", NEWdb->size);

			if (strcmp(cnfm, "Y") == 0) {
				for (int i = 0; i < StudentDB->size; i++) {
					printf("\nChecking Index %d\n", i);
					if (i == indexdelete) {
						continue;
					}
					NEWrecord[newindex] = record[i];
					newindex++;
				}
				for (int i = 0; i < NEWdb->size; i++) {
					printf("%d\t%s\t%s\t%f\n",
						NEWrecord[i].id,
						NEWrecord[i].name,
						NEWrecord[i].programme,
						NEWrecord[i].mark);
				}
				set_database(NEWdb);

				printf("\nThe record with ID=%d is successfully deleted\n", iddelete);
				cnfmdeleting = 0;
			}
			else if (strcmp(cnfm, "N") == 0) {
				printf("\nThe deletion is cancelled.");
				cnfmdeleting = 0;
			}
			else {
				printf("\nPlease enter either 'Y' or 'N'");
			}
		}
		deleting = 0;
		return NEWdb;
		}
}

// jaison sort function

bool sort_fn(char* context) {
	int sorting = 1;
	char sortchoice[8], sortupdown[14];

	// sortchoice is user input for Sorting by ID or Mark; sortupdown is user input for Sorting Ascending or Descending
	while (sorting == 1) {
		printf("Sort:\nBy ID\nBy Mark\nP2_7: ");
		fgets(sortchoice, sizeof(sortchoice), stdin);
		printf("\n");
		tempclean(sortchoice);

		// Checks if they didn't input id or mark
		if (strcmp(sortchoice, "ID") != 0 && strcmp(sortchoice, "MARK") != 0) {
			printf("\nInvalid Input, please enter 'ID' or 'MARK'\n");
			continue;
		}

		printf("Ascending or Descending?\nP2_7: ");
		fgets(sortupdown, sizeof(sortupdown), stdin);
		tempclean(sortupdown);

		if (strcmp(sortupdown, "ASCENDING") != 0 && strcmp(sortupdown, "DESCENDING") != 0) {
			printf("Invalid Input, please enter 'ASCENDING' or 'DESCENDING'");
			continue;
		}

		struct Database* StudentDB = get_database(); //struct Student and function get_database() is in data.c
		//int student_count = studentcount(); Old code 

		if (StudentDB == NULL) {
			printf("No Student Records could be found");
			return false;
		}

		if (strcmp(sortchoice, "ID") == 0) {
			printf("\nSorting by ID...");
			if (strcmp(sortupdown, "ASCENDING") == 0) {
				printf("\nSorting ID in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compidup);
			}
			else if (strcmp(sortupdown, "DESCENDING") == 0) {
				printf("\nSorting ID in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compiddown);
			}
		}
		else if (strcmp(sortchoice, "MARK") == 0) {
			printf("\nSorting by MARK...");
			if (strcmp(sortupdown, "ASCENDING") == 0) {
				printf("\nSorting MARK in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkup);
			}
			else if (strcmp(sortupdown, "DESCENDING") == 0) {
				printf("\nSorting MARK in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkdown);
			}
		}
		sorting = 0;
	}
	printf("StudentRecord is sorted");
	return true;
}

// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", 1, open_fn},
	{"SHOW ALL", 2, showall_fn},
	{"SORT", 1, sort_fn},
	{"DELETE", 1, delete_fn}
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
