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

bool open_fn(char* context) {
	//char cwd[1024];
	//_getcwd(cwd, sizeof(cwd));
	//printf("Current working directory: %s\n", cwd);

	struct Database* oldDB = get_database();
	if (oldDB != NULL) {	// means open was run twice, need to free old stuff so can overwrite smoothly
		free_database(oldDB);
		set_database(NULL);
	}

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

		const char* extensions[] = { ".txt" };	// might add .csv, but too much work, too much difference
		int loop_amt = sizeof(extensions) / sizeof(extensions[0]);

		for (int i = 0; i < loop_amt; i++) {
			char filepath_ext[250];	//zktodo: use snprintf?
			strcpy_s(filepath_ext, sizeof(filepath_ext), filepath);
			strcat_s(filepath_ext, sizeof(filepath_ext), extensions[i]);
			fopen_s(&file_ptr, filepath_ext, "r");

			if (file_ptr != NULL) {
				printf("Missing extension, found %s instead\n", filepath_ext);
				strcpy_s(filepath, sizeof(filepath), filepath_ext);	// update filepath for the printf
				break;
			}
		}

	}
	// if it still fails, lost cause
	if (file_ptr == NULL) {
		printf("File %s not found.\n", filepath);
		return false;
	}

	printf("The database file \"%s\" is successfully opened.\n", filepath);
	struct Database* StudentDB = load_data(file_ptr);
	fclose(file_ptr);

	if (StudentDB == NULL || StudentDB->StudentRecord == NULL) {
		return false;
	}

	strcpy_s(StudentDB->filepath, sizeof(StudentDB->filepath), filepath);	// store filepath for save fn

	set_database(StudentDB);	// store in static var for access from other functions
	return true;
	
}

bool showall_fn(char* context) {
	
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL) {
		printf("No records in database.\n");
		return false;
	}

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less
	
	//printf("%s and %s\n", StudentDB->databaseName, StudentDB->authors);
	printf("Here are all the records found in the table \"%s\".\n", StudentDB->tableName);

	// print out header row, same as input file
	for (int column_index = 0; column_index < StudentDB->column_count; column_index++) {
		printf("%s", StudentDB->columns[column_index].header_name);
		//switch (StudentDB->columns[column_index].column_id) {
		//	case COL_ID:
		//		filler_width = 5;	// length of 7 digit id minus length of "ID"
		//		break;
		//	case COL_NAME:
		//		filler_width = StudentDB->columns[column_index].max_width - strlen(StudentDB->columns[column_index].header_name);
		//		break;
		//	case COL_PROGRAMME:
		//		filler_width = StudentDB->columns[column_index].max_width - strlen(StudentDB->columns[column_index].header_name);
		//		break;
		//	case COL_MARK:
		//		filler_width = 1;	// length of "100.0" minus length of "MARK"
		//		break;
		//	case COL_OTHER:
		//		break;
		//}
		int filler_width = StudentDB->columns[column_index].max_width - strlen(StudentDB->columns[column_index].header_name);
		printf("%*s\t", filler_width, "");
	}
	printf("\n");
	
	// print out the data rows, following header order
	for (int student_index = 0; student_index < StudentDB->size; student_index++) {
		// for each column in the row
		for (int column_index = 0; column_index < StudentDB->column_count; column_index++) {
			int datapoint_width = 0;
			char mark_str[10];

			switch (StudentDB->columns[column_index].column_id) {
				case COL_ID:
					printf("%d", record[student_index].id);
					// ID are fixed to 7 digits, so no extra spaces here
					//printf("%*s", StudentDB->columns[column_index].max_width - strlen(record[student_index].id), "");
					datapoint_width = 7;	// 7 digit id
					break;
				case COL_NAME:
					printf("%s", record[student_index].name);
					//printf("%*s", (StudentDB->columns[column_index].max_width) - (strlen(record[student_index].name)), "");
					datapoint_width = strlen(record[student_index].name);
					break;
				case COL_PROGRAMME:
					printf("%s", record[student_index].programme);
					//printf("%*s", (StudentDB->columns[column_index].max_width) - (strlen(record[student_index].programme)), "");
					datapoint_width = strlen(record[student_index].programme);
					break;
				case COL_MARK:
					printf("%.1f", record[student_index].mark);	// %.1f below cuz %f gives smth like 0.000000 (width becomes too high)
					sprintf_s(mark_str, sizeof(mark_str), "%.1f", record[student_index].mark);
					datapoint_width = strlen(mark_str);

					// Mark has a max possible length of 5 (100.0)
					break;
				case COL_OTHER:	// safety net	(not printing the value cuz im currently not storing those vals)
					printf("N/A");
					datapoint_width = 3;
					break;
			}

			printf("%*s", (StudentDB->columns[column_index].max_width - datapoint_width), "");	// add spaces to align columns in print

			if (column_index != StudentDB->column_count - 1) {//-1 because column_index starts from 0
				printf("\t");	// \t unless end of line, though doesnt rly matter (inputs are stripped anyway)
			}
		}
		printf("\n");
	}

	return true;
};

// jaison delete function
struct Database* delete_fn(char* context) {
	char cnfm[6], idbuffer[10];
	int deleting = 1;
	while (deleting == 1) {
		struct Database* StudentDB = get_database(); // Initialize existing struct Student Database

		if (StudentDB == NULL) {
			printf("\nNo records in database.");
			break;
		}
		
		printf("\nID to Delete: ");

		fgets(idbuffer, sizeof(idbuffer), stdin);
		clean_input(idbuffer); //Accept id as string for cleanup

		for (int i = 0; i < sizeof(StudentDB->size); i++) {
			if (validate_id(idbuffer, i, StudentDB) == 1) {
				printf("\nPlease enter a valid ID");
				continue;
			}
		}

		int iddelete = atoi(idbuffer); //Convert string to int, if string is not integer, atoi returns 0

		/*
		int count = countid(iddelete); //Counts digits in userinput, if input is 0 (or atoi returns 0)
		//printf("\ncount: %d", count);

		//if digit is less/more than 7, invalid id
		if (count != 7) {
			printf("\nPlease enter a valid ID");
			continue;
		}
		*/

		struct Student* record = StudentDB->StudentRecord; // Initialize record pointer for easier writing
		int indexdelete = -1;  // Initialize indexdelete to -1 to check if ID to delete exists
		int cnfmdeleting = 0; // Initialize cnfmdeleting to enter confirmation loop if ID to delete exists

			
		// Checks record.id if it matches userinput iddelete
		for (int i = 0; i < StudentDB->size; i++) {
			if (record[i].id == iddelete) {
				printf("\nFound record ID=%d at Index=%d", iddelete, i); //Debug
				indexdelete = i;
				cnfmdeleting = 1; //Starts confirmation loop
				break;
			}
		}
		
		// ID to delete does not exist
		if (indexdelete == -1) {
			printf("\nThe record with ID=%d does not exist\n", iddelete); //Debug
			deleting = 0;
			break;
		}

		// Confirmation loop
		while (cnfmdeleting == 1) {
			printf("\nAre you sure you want to delete record with ID=%d?\nType \"Y\" to Confirm or type \"N\" to Cancel: ", iddelete);
			fgets(cnfm, sizeof(cnfm), stdin);
			clean_input(cnfm);

			//printf("\nSize of Original Database: %d", StudentDB->size);
			//printf("\nSize of New Database: %d", NEWdb->size);

			// Yes confirmation
			if (_stricmp(cnfm, "y") == 0) {

				/*
				for (int i = indexdelete; i < StudentDB->size; i++) {
					StudentDB->StudentRecord[i] = StudentDB->StudentRecord[i + 1];
				}
				*/

				struct Database* NEWdb = malloc(sizeof(struct Database)); //Initialize NEWdb to copy old database excluding deleted record
				NEWdb = cpyDatabaseDetails(StudentDB, NEWdb); // Refer to 'jaison addition' in data.c & data.h
				
				NEWdb->StudentRecord = malloc(sizeof(struct Student) * (StudentDB->size - 1));
				NEWdb->size = StudentDB->size - 1;
				int newindex = 0;
				
				if (NEWdb == NULL) {
					printf("\nNEW Database Creation Failed.");
					break;
				}

				struct Student* NEWrecord = NEWdb->StudentRecord;

				if (NEWrecord == NULL) {
					printf("NEWrecord Memory allocation for StudentRecord failed.\n");
					break;
				}
				
				for (int i = 0; i < StudentDB->size; i++) {
					printf("\nChecking Index %d\n", i); //Debug
					if (i == indexdelete) {
						printf("\nSkipping Index %d\n", i); //Debug
						continue;
					}
					NEWrecord[newindex] = record[i];
					newindex++;
				}

				/* Debug NEWrecord
				printf("\nNew StudentRecord\n");
				for (int i = 0; i < NEWdb->size; i++) {
					printf("%d\t%s\t%s\t%f\n",
						NEWrecord[i].id,
						NEWrecord[i].name,
						NEWrecord[i].programme,
						NEWrecord[i].mark);
				}
				*/

				set_database(NEWdb);
				
				free(record);
				free(StudentDB->columns);
				free(StudentDB);

				printf("\nThe record with ID=%d is successfully deleted\n", iddelete);
				cnfmdeleting = 0;
				deleting = 0;
				return NEWdb;
			}
			else if (_stricmp(cnfm, "n") == 0) {
					printf("\nThe deletion is cancelled.\n");
					cnfmdeleting = 0;
					return StudentDB;
			}
			else {
					printf("\nPlease enter either 'Y' or 'N'\n");
			}
		}
	}
}

// jaison sort function

bool sort_fn(char* context) {
	int sorting = 1;
	char sortchoice[8], sortupdown[14];

	// sortchoice is user input for Sorting by ID or Mark; sortupdown is user input for Sorting Ascending or Descending
	while (sorting == 1) {
		printf("\nSort:\nBy ID\nBy Mark\nP2_7: ");
		fgets(sortchoice, sizeof(sortchoice), stdin);
		clean_input(sortchoice);

		// If they don't want to sort anymore
		if (_stricmp(sortchoice, "exit") == 0) {
			sorting = 0;
			break;
		}

		// Checks if they didn't input id or mark
		if (_stricmp(sortchoice, "id") != 0 && _stricmp(sortchoice, "mark") != 0) {
			printf("\nInvalid Input, please enter 'ID' or 'MARK'\n");
			continue;
		}

		printf("\nAscending or Descending?\nP2_7: ");
		fgets(sortupdown, sizeof(sortupdown), stdin);
		clean_input(sortupdown);

		if (_stricmp(sortupdown, "ascending") != 0 && _stricmp(sortupdown, "descending") != 0) {
			printf("\nInvalid Input, please enter 'ASCENDING' or 'DESCENDING'\n");
			continue;
		}

		struct Database* StudentDB = get_database(); //struct Student and function get_database() is in data.c
		//int student_count = studentcount(); Old code 

		if (StudentDB == NULL) {
			printf("No Student Records could be found");
			return false;
		}

		if (_stricmp(sortchoice, "id") == 0) {
			printf("\nSorting by ID...");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf("\nSorting ID in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compidup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf("\nSorting ID in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compiddown);
			}
		}
		else if (_stricmp(sortchoice, "mark") == 0) {
			printf("\nSorting by MARK...");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf("\nSorting MARK in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf("\nSorting MARK in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkdown);
			}
		}
		sorting = 0;
	}
	printf("\nSorting Successful.\n");
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
