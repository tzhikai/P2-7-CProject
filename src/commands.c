#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <direct.h>	// for checking current working directory to see what the relative path is
#include <ctype.h>
#include <float.h>

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
		printf("CMS: No file name detected. Please try again.\n");
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
			strncpy_s(filepath_ext, sizeof(filepath_ext), filepath, _TRUNCATE);
			strcat_s(filepath_ext, sizeof(filepath_ext), extensions[i]);
			fopen_s(&file_ptr, filepath_ext, "r");

			if (file_ptr != NULL) {
				printf("Missing extension, found %s instead\n", filepath_ext);
				strncpy_s(filepath, sizeof(filepath), filepath_ext, _TRUNCATE);	// update filepath for the printf
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

	strncpy_s(StudentDB->filepath, sizeof(StudentDB->filepath), filepath, _TRUNCATE);	// store filepath for save fn

	set_database(StudentDB);	// store in static var for access from other functions
	return true;
	
}

bool showall_fn(char* context) {
	
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less
	
	if (StudentDB->size == 0) {
		printf("No records to show.\n");
		return false;
	}

	//printf("%s and %s\n", StudentDB->databaseName, StudentDB->authors);
	printf("Here are all the records found in the table \"%s\".\n", StudentDB->tableName);

	print_headers(StudentDB);
	
	// print out the data rows, following header order
	for (int student_index = 0; student_index < StudentDB->size; student_index++) {
		print_datarow(StudentDB, student_index);
	}

	return true;
};

// jaison delete function
// zkchange: should be bool since thats what run_command expects, and theres no need to return the newDB when we use set_database
bool delete_fn(char* context) {
	char cnfm[6];
	char idbuffer[10] = "";

	int deleting = 1;
	while (deleting == 1) {
		struct Database* StudentDB = get_database(); // Initialize existing struct Student Database

		if (StudentDB == NULL || StudentDB->size == 0) {
			printf("CMS: No Database loaded. Please OPEN a file first.\n");
			break;
		}

		int id = 0;
		int* id_ptr = &id;

		/*struct HeaderValuePair hvp_array[10];
		memset(hvp_array, 0, sizeof(hvp_array));*/

		if (context != NULL && context[0] != '\0') {
			extract_extrainput_id(id_ptr, context, StudentDB, NULL, CMD_DELETE);	
			// NULL for the HeaderValuePair struct input, cuz DELETE wouldnt use it
		}

		char idbuffer[10] = "";

		// If ID was successfully extracted from context, use it
		if (id != 0) {
			snprintf(idbuffer, sizeof(idbuffer), "%d", id);
			//printf("id buffer: %s\n", idbuffer);
		}
		
		if (idbuffer[0] == '\0') {	// means user just typed DELETE, no id initially given
			int valid_id = 0;
			while (!valid_id) {
				printf("Enter ID of row to delete: ");
				fgets(idbuffer, sizeof(idbuffer), stdin);
				clean_input(idbuffer); //Accept id as string for cleanup

				if (_stricmp(idbuffer, "exit") == 0) {
					printf("Cancelling DELETE ...\n");
					return false;
				}

				switch (validate_id(idbuffer, 0, StudentDB, CMD_DELETE)) {
					case 1:	// ID is invalid
						//printf("Please enter a valid ID.\n");
						idbuffer[0] = '\0'; // prompts fgets again to ask user for id again
						continue;
					case 0:	// ID is valid but not in use, nothing to delete
						//printf("ID is not in use.\n");
						idbuffer[0] = '\0';
						continue;
					case 2:	// ID is valid and in use, proceed to deleting
						valid_id = 1;
						break;
					}
			}

			
		}

		int iddelete = atoi(idbuffer); //Convert string to int, if string is not integer, atoi returns 0

		struct Student* record = StudentDB->StudentRecord; // Initialize record pointer for easier writing

		int cnfmdeleting = 0; // Initialize cnfmdeleting to enter confirmation loop if ID to delete exists

		// find index of record to delete (returns -1 if not found)
		int indexdelete = id_search(iddelete);	//zkchange i made this a fn 

		if (indexdelete == -1) {
			printf("\nThe student record with ID=%d does not exist\n", iddelete); //Debug
			deleting = 0;
			return false;
		}
		else {
			//printf("\nFound record ID=%d at index %d\n", iddelete, indexdelete);
			cnfmdeleting = 1; //Starts confirmation loop
		}

		// Confirmation loop
		while (cnfmdeleting == 1) {
			struct UndoStack* undos = get_undostack();
			if (!undos->pause_inserts) {
				printf("\nAre you sure you want to delete record with ID=%d?\nType \"Y\" to Confirm or type \"N\" to Cancel: ", iddelete);
				fgets(cnfm, sizeof(cnfm), stdin);
				clean_input(cnfm);
			}
			else {	// if DELETE is run during UNDO, then dont bother asking (alr asked within undo_fn)
				strncpy_s(cnfm, sizeof(cnfm), "y", _TRUNCATE);
			}
			

			//printf("\nSize of Original Database: %d", StudentDB->size);
			//printf("\nSize of New Database: %d", NEWdb->size);

			// Yes confirmation
			if (_stricmp(cnfm, "y") == 0) {
				char temp_mark_str[10];
				struct ColumnMap* cols = StudentDB->columns;

				char undo_command[100] = "INSERT ID=";
				snprintf(undo_command, sizeof(undo_command), "INSERT ID=%d", iddelete);
				for (int i = 0; i < StudentDB->column_count; i++) {
					// ID is alr inserted first (since it has to be first), unexpected cols are ignored
					if (cols[i].column_id == COL_ID || cols[i].column_id == COL_OTHER) {
						continue;
					}
					strncat_s(undo_command, sizeof(undo_command), " ", _TRUNCATE);
					strncat_s(undo_command, sizeof(undo_command), cols[i].header_name, _TRUNCATE);
					strncat_s(undo_command, sizeof(undo_command), "=", _TRUNCATE);
					
					switch (cols[i].column_id) {
					case COL_NAME:
						strncat_s(undo_command, sizeof(undo_command), StudentDB->StudentRecord[indexdelete].name, _TRUNCATE);
						break;
					case COL_PROGRAMME:
						strncat_s(undo_command, sizeof(undo_command), StudentDB->StudentRecord[indexdelete].programme, _TRUNCATE);
						break;
					case COL_MARK:
						snprintf(temp_mark_str, sizeof(temp_mark_str), "%.1f", StudentDB->StudentRecord[indexdelete].mark);

						strncat_s(undo_command, sizeof(undo_command), temp_mark_str, _TRUNCATE);
						break;
					}
					
				}

				/*
				for (int i = indexdelete; i < StudentDB->size; i++) {
					StudentDB->StudentRecord[i] = StudentDB->StudentRecord[i + 1];
				}
				*/
				//zkstart
				 
				struct Database* NEWdb = malloc(sizeof(struct Database));
				if (NEWdb == NULL) {
					printf("\nMemory allocation for new database failed.");
					return false;
				}

				// initialise NEWdb
				memset(NEWdb, 0, sizeof(struct Database));

				// copy pasting over the details
				strncpy_s(NEWdb->databaseName, sizeof(NEWdb->databaseName),
					StudentDB->databaseName, _TRUNCATE);
				strncpy_s(NEWdb->authors, sizeof(NEWdb->authors),
					StudentDB->authors, _TRUNCATE);
				strncpy_s(NEWdb->tableName, sizeof(NEWdb->tableName),
					StudentDB->tableName, _TRUNCATE);
				strncpy_s(NEWdb->filepath, sizeof(NEWdb->filepath),
					StudentDB->filepath, _TRUNCATE);

				NEWdb->column_count = StudentDB->column_count;
				NEWdb->size = StudentDB->size - 1;
				NEWdb->capacity = NEWdb->size > 0 ? NEWdb->size : 1; // Ensure at least capacity 1

				// Allocate columns
				NEWdb->columns = malloc(sizeof(struct ColumnMap) * StudentDB->column_count);
				if (NEWdb->columns == NULL) {
					printf("Memory allocation for columns failed.\n");
					free(NEWdb);
					return false;
				}
				memcpy(NEWdb->columns, StudentDB->columns,
					sizeof(struct ColumnMap) * StudentDB->column_count);

				// Allocate student records
				if (NEWdb->size > 0) {
					NEWdb->StudentRecord = malloc(sizeof(struct Student) * NEWdb->size);
					if (NEWdb->StudentRecord == NULL) {
						printf("Memory allocation for StudentRecord failed.\n");
						free(NEWdb->columns);
						free(NEWdb);
						return false;
					}

					// Copy records, skipping the deleted one
					int newindex = 0;
					for (int i = 0; i < StudentDB->size; i++) {
						if (i == indexdelete) continue;
						NEWdb->StudentRecord[newindex++] = StudentDB->StudentRecord[i];
					}
				}
				else {
					// Handle case when deleting the last record
					NEWdb->StudentRecord = NULL;
				}
				 
				//struct Database* NEWdb = malloc(sizeof(struct Database)); //Initialize NEWdb to copy old database excluding deleted record
				//
				//if (NEWdb == NULL) {
				//	printf("\nMemory alocation for new database failed.");
				//	return false;
				//}
				//
				//NEWdb = cpyDatabaseDetails(StudentDB, NEWdb); // Refer to 'jaison addition' in data.c & data.h
				//
				//NEWdb->StudentRecord = malloc(sizeof(struct Student) * (StudentDB->size - 1));
				//NEWdb->size = StudentDB->size - 1;
				//int newindex = 0;
				//
				//

				//struct Student* NEWrecord = NEWdb->StudentRecord;

				//if (NEWrecord == NULL) {
				//	printf("NEWrecord Memory allocation for StudentRecord failed.\n");
				//	free(NEWdb);
				//	return false;
				//}
				//
				//for (int i = 0; i < StudentDB->size; i++) {
				//	//printf("\nChecking Index %d\n", i); //Debug
				//	if (i == indexdelete) {
				//		//printf("\nSkipping Index %d\n", i); //Debug
				//		continue;
				//	}
				//	NEWrecord[newindex] = record[i];
				//	newindex++;
				//}

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

				free_database(StudentDB);
				set_database(NEWdb);
				
			
				struct Database* db = get_database();
				if (NEWdb->size > 0) {	// doesnt run if theres no records after deleting
					update_width(db, indexdelete, CMD_DELETE);
				}

				printf("\nThe record with ID=%d is successfully deleted\n", iddelete);
				cnfmdeleting = 0;
				deleting = 0;

				insert_undostack(undo_command);

				return true;
			}
			else if (_stricmp(cnfm, "n") == 0) {
					printf("\nThe deletion is cancelled.\n");
					cnfmdeleting = 0;
					return true;
			}
			else {
					printf("\nPlease enter either 'Y' or 'N'\n");
			}
		}
	}

	//insert_undostack("test_insert_undo");
}

// jaison sort function

bool sort_fn(char* context) {
	struct Database* StudentDB = get_database(); //struct Student and function get_database() is in data.c
	//int student_count = studentcount(); Old code 

	if (StudentDB == NULL || StudentDB->size == 0) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	int sorting = 1;
	// sortchoice is user input for col to sort by; sortupdown is user input for Sorting Ascending or Descending
	char sortchoice[8], sortupdown[14];

	int col_index, sortchoice_int;	// storage ints to help w finding corresponding column
	int col_answered = 0;	//decides if user's input is valid

	
	while (sorting == 1) {
		//printf("\nSort:\nBy ID\nBy Mark\nBy Name\nP2_7: ");
		printf("\nWhat column would you like to sort by? Type the corresponding number or \"exit\":\n");//zkchange: this what i meant by not hardcoding col names
		for (col_index = 0; col_index < StudentDB->column_count; col_index++) {
			if (StudentDB->columns[col_index].column_id != COL_OTHER) {	// we dont have handling for unexpected cols zktodo: fix?
				printf("%d) %s\n", col_index + 1, StudentDB->columns[col_index].header_name);
			}
		}
		if (col_index == 0) {
			printf("No columns found to sort using.\n");
			return false;
		}
		printf("P2_7: ");

		do {	//zkchange: like this one mistake wont kick user out of sort
			fgets(sortchoice, sizeof(sortchoice), stdin);
			clean_input(sortchoice);

			if (_stricmp(sortchoice, "exit") == 0) {
				sorting = 0;
				printf("Cancelling SORT ...\n");
				return false;
			}

			for (int i = 0; sortchoice[i] != '\0'; i++) {
				if (!isdigit(sortchoice[i])) { // if any char is not a digit, invalid input
					printf("Invalid input, please try again or type \"exit\".\n");
					printf("P2_7: ");
					break;	//continue looping
				}
			}

			sortchoice_int = atoi(sortchoice);

			if (sortchoice_int >= 1 && sortchoice_int <= col_index) {	// if got here, col_index guaranteed to be >= 1
				if (StudentDB->columns[sortchoice_int - 1].column_id == COL_OTHER) {// we dont have handling for unexpected cols zktodo: fix?
					printf("Invalid input, please try again or type \"exit\".\n");
					printf("P2_7: ");
					continue;	//continue looping
				}
				col_answered = 1;
			}
			
		} while (!col_answered);

		Columns sort_col = StudentDB->columns[sortchoice_int - 1].column_id;

		// Checks if they didn't input id or mark
		//if (_stricmp(sortchoice, "id") != 0 && _stricmp(sortchoice, "mark") != 0 && _stricmp(sortchoice, "name") != 0) {
		//	printf("\nInvalid Input, please enter 'ID' or 'MARK'\n");
		//	continue;
		//}

		printf("\nAscending or Descending?\nP2_7: ");
		fgets(sortupdown, sizeof(sortupdown), stdin);
		clean_input(sortupdown);

		if (_stricmp(sortupdown, "exit") == 0) {
			printf("Cancelling SORT ...\n");
			sorting = 0;
			break;
		}

		if (_stricmp(sortupdown, "back") == 0) {
			printf("Returning to Column Selection.\n");
			col_answered = 0;
			continue;
		}

		if (_stricmp(sortupdown, "ascending") != 0 && _stricmp(sortupdown, "descending") != 0) {
			printf("\nInvalid Input, please enter 'ASCENDING' or 'DESCENDING'\n");
			continue;
		}

		if (sort_col == COL_ID) {	//zkchange: now it uses the columns from user file, oso it wont print sorting by ... twice
			printf("\nSorting by ID");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf(" in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compidup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf(" in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compiddown);
			}
		}
		else if (sort_col == COL_MARK) {
			printf("\nSorting by MARK");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf(" in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf(" in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compmarkdown);
			}
		}
		else if (sort_col == COL_NAME) {
			printf("\nSorting by NAME");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf(" in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compnameup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf(" in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compnamedown);
			}
		}	
		else if (sort_col == COL_PROGRAMME) {
			printf("\nSorting by PROGRAMME");
			if (_stricmp(sortupdown, "ascending") == 0) {
				printf(" in Ascending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compprogrammeup);
			}
			else if (_stricmp(sortupdown, "descending") == 0) {
				printf(" in Descending Order...");
				qsort(StudentDB->StudentRecord, StudentDB->size, sizeof(struct Student), compprogrammedown);
			}
		}
		printf("\nSorting Successful.\n");
		sorting = 0;
	}
	return true;
}

// Jaison NewFile Function
bool newfile_fn(char* context) {
	int cnfminit = 1, newfile = 0, cnfm_int;
	char cnfm[7];

	//initialize ^^
	while (cnfminit == 1) {
		printf("Would you like to\n1. Make a new empty text file\n2. Save current database to new text file\nType \"Exit\" to cancel\nP2_7: ");
		fgets(cnfm, sizeof(cnfm), stdin);
		clean_input(cnfm);

		if (_stricmp(cnfm, "exit") == 0) {
			cnfminit = 0; //break
			return false;
		}
		else if (isdigit(cnfm[0])) {
			cnfm_int = atoi(cnfm);
		}
		else {
			printf("\nInvalid input. Please enter either 1 or 2.\n\n");
			continue;
		}

		if (cnfm_int > 2 || cnfm_int < 0) {
			printf("\nInvalid input. Please enter either 1 or 2.\n\n");
			continue;
		}
		else {
			newfile = 1;
		}

		struct Database* StudentDB = get_database();

		if (cnfm_int == 2 && StudentDB == NULL) {
			printf("CMS: No Database loaded. Please OPEN a file first.\n");
			cnfminit = 0; //break
			return false;
		}

		while (newfile == 1) {
			char filename[50], dbname[50], authorname[50], tablename[50], file_ext[5] = ".txt", filepath[250] = "src\\data\\";
			FILE* file = NULL;

			printf("Enter the name for new file (without .txt extension):\nMAX 50 CHARACTERS\nP2_7: ");
			fgets(filename, sizeof(filename), stdin);
			clean_input(filename);
			strcat_s(filepath, sizeof(filepath), filename);
			strcat_s(filepath, sizeof(filepath), file_ext);

			if (fopen_s(&file, filepath, "r") == 0 && file != NULL) {
				fclose(file);
				printf("File %s already exists. Please choose a different name.\n", filename);
				continue;
			}

			printf("Enter the database name for new file:\nMAX 50 CHARACTERS\nP2_7: ");
			fgets(dbname, sizeof(dbname), stdin);
			clean_input(dbname);

			printf("Enter the author name(s) for new file:\nMAX 50 CHARACTERS\nP2_7: ");
			fgets(authorname, sizeof(authorname), stdin);
			clean_input(authorname);

			printf("Enter the table name for new file:\nMAX 50 CHARACTERS\nP2_7: ");
			fgets(tablename, sizeof(tablename), stdin);
			clean_input(tablename);

			fopen_s(&file, filepath, "w");

			if (file == NULL) {
				printf("Unable to open file for saving: %s\n", filename);
				newfile = 0;
				cnfminit = 0;
				return false; //break
			}

			// Default text file header and titles
			fprintf(file, "Database Name: %s\n", dbname);
			fprintf(file, "Authors: %s\n\n", authorname);
			fprintf(file, "Table Name: %s\n", tablename);

			if (cnfm_int == 1) { // New Empty Text File
				fprintf(file, "ID\tNAME\tPROGRAMME\tMARK\n");
			}
			else if (cnfm_int == 2) {
				if (StudentDB != NULL) {
					for (int i = 0; i < StudentDB->column_count; i++) {
						fprintf(file, "%s", StudentDB->columns[i].header_name);
						if (i < StudentDB->column_count - 1)
							fprintf(file, "\t");
					}
					fprintf(file, "\n");

					// Records
					for (int i = 0; i < StudentDB->size; i++) {
						fprintf(file, "%d\t%s\t%s\t%.1f\n",
							StudentDB->StudentRecord[i].id,
							StudentDB->StudentRecord[i].name ? StudentDB->StudentRecord[i].name : "N/A",
							StudentDB->StudentRecord[i].programme ? StudentDB->StudentRecord[i].programme : "N/A",
							StudentDB->StudentRecord[i].mark);
					}
				}
				else {
					printf("Failed to save Database into new text file");
				}
			}
			else {
				printf("This Error shouldn't even be possible because an earlier check should have handled.\n");
				newfile = 0;
				cnfminit = 0;
				break;
			}
			fclose(file);
			printf("CMS: New database file %s created successfully.\n", filename);
			newfile = 0;
		}
		cnfminit = 0;
		return true;
	}
}

bool undo_fn(char* context){
	struct Database* StudentDB = get_database(); //struct Student and function get_database() is in data.c
	//int student_count = studentcount(); Old code 

	if (StudentDB == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	struct UndoStack* undos = get_undostack();
	if (undos == NULL) {
		printf("UndoStack not created yet.\n");
		return false;
	}

	clean_input(context);
	int undo_count = 1;

	// decide how many undos to do
	if (context != NULL && context[0] != '\0') {

		clean_input(context);

		if (sscanf_s(context, "%d", &undo_count) != 1) {
			printf("Invalid undo amount specified, please use an integer.\n");
			return false;
		}

		undo_count = atoi(context);
	}

	if (undo_count < 0) {
		printf("Invalid undo amount. Use between 1 and %d\n", MAX_UNDOS);
		return false;
	}
	if (undos->cmd_count == 0) {
		printf("No undos available. Undos are added for every INSERT, UPDATE and DELETE.\n");
		return false;
	}
	
	int undo_limit;
	if (undos->cmd_count < MAX_UNDOS) {
		undo_limit = undos->cmd_count;
	}
	else {
		undo_limit = MAX_UNDOS;
	}

	if (undo_count > undos->cmd_count) {
		printf("Undo amount exceeds available amount, which is %d. Capping at %d...\n", undo_limit, undo_limit);
		undo_count = undo_limit;
	}

	char buffer[255];
	char yn_buffer[10];

	printf("Here are the commands that will be run. Starting from most recent...\n");

	for (int i = 0; i < undo_count; i++) {
		//printf("debug: loop no. %d\n", i);
		if (!preview_undostack(i)) {
			printf("Error previewing undos in undostack.\n");
			return false;
		}
	}

	do {
		printf("Are you sure? (Y/N): ");
		fgets(yn_buffer, sizeof(yn_buffer), stdin);
		clean_input(yn_buffer);
		printf("\n");

		if (_stricmp(yn_buffer, "n") == 0) {
			printf("Cancelling undo.\n");
			return false;
		}
	} while (_stricmp(yn_buffer, "y") != 0);


	undos->pause_inserts = true;	// pause inserting only when we actually start running them

	while (undo_count > 0) {
		switch (undo_count) {
		case 1:
			printf("Performing final undo...\n");
			break;
		case 2:
			printf("Performing 2nd to last undo...\n");
			break;
		case 3:
			printf("Performing 3rd to last undo...\n");
			break;
		default:	// hopefully its not 21 22 23
			printf("Performing %dth to last undo...\n", undo_count);
			break;
		}

		if (!use_undostack(buffer)) {
			printf("Error running undo function.\n");
			return false;
		}

		//printf("Undo command to run: %s\n", buffer);
		run_command(buffer);

		undo_count--;
	}

	undos->pause_inserts = false;	//revert back to allow INSERT, DELETE, UPDATE to insert_undostack
	return true;
}

// HY show summary
void print_summary(struct Database* db) {
	// Validate database
	if (db == NULL || db->size == 0) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return;
	}
	// Initialize tracking variables
	float highest = -1;
	float lowest = 101;
	float total = 0;
	int highestIndex = -1;
	int lowestIndex = -1;
	int validCount = 0;

	// Iterate through all student records
	for (int i = 0; i < db->size; i++) {
		float m = db->StudentRecord[i].mark;
		// when mark is negative (skipped)
		if (m < 0) {
			continue;
		}

		total += m;
		validCount++;

		// Check highest mark
		if (m > highest) {
			highest = m;
			highestIndex = i;
		}
		// Check lowest mark
		if (m < lowest) {
			lowest = m;
			lowestIndex = i;
		}
	}

	int mark_col = get_column(COL_MARK);
	char mark_header[20] = "";
	strncpy_s(mark_header, sizeof(mark_header), db->columns[mark_col].header_name, _TRUNCATE);

	printf("CMS: Summary Statistics\n");

	// Exception handling for no valid marks in database
	if (validCount == 0) {
		
		printf("No valid %s to summarise.\n", mark_header);
		return;
	}
	// Gets average of non invalid marks
	float average = total / validCount;

	printf("Total number of students: %d\n", db->size);
	// Only displays valid-mark count if some marks were invalid
	if (db->size != validCount) {
		printf("Total number of students with valid %s: %d\n", mark_header, validCount);
	}
	printf("Average %s: %.2f\n",mark_header, average);

	printf("Highest %s: %.2f (%s)\n",
		mark_header, highest, db->StudentRecord[highestIndex].name);

	printf("Lowest %s: %.2f (%s)\n",
		mark_header, lowest, db->StudentRecord[lowestIndex].name);
}

// Call summary function
bool summary_fn(char* context) {
	struct Database* db = get_database();
	print_summary(db);
	return true;
}

//hy update
bool update_fn(char* context) {
	// Load database and validate
	struct Database* db = get_database();
	if (db == NULL || db->StudentRecord == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}
	//printf("context here: %s\n", context);
	// Extract ID and header/value pairs
	int id = 0, idx = -1;
	int* id_ptr = &id;	// for passing by reference to extract_extrainput_id

	struct HeaderValuePair hvp_array[10];
	memset(hvp_array, 0, sizeof(hvp_array));
	
	int hvpair_count = 0;
	if (context != NULL && context[0] != '\0') {
		hvpair_count = extract_extrainput_id(id_ptr, context, db, hvp_array, CMD_UPDATE);
	}

	// Prompt user if ID not found
	if (id == 0) {
		// ask for ID
		char id_buffer[20];
		do {
			printf("CMS: Enter the student ID to update: ");
			fgets(id_buffer, sizeof(id_buffer), stdin);

			if (strlen(id_buffer) > 0) {
				id_buffer[strlen(id_buffer) - 1] = '\0';
			}

			if (_stricmp(id_buffer, "exit") == 0) {
				printf("Cancelling UPDATE ...\n");
				return false;
			}

			if (validate_id(id_buffer, -1, db, CMD_UPDATE) != 2) {
				printf("CMS: The record with ID \"%s\" does not exist. Please try again.\n", id_buffer);
				continue;
			}
			
			id = atoi(id_buffer);
			// Search for student with the given ID
			idx = id_search(id);

		} while (idx == -1);
	}
	else {
		idx = id_search(id);
		if (idx == -1) {
			printf("CMS: The record with ID \"%d\" does not exist. Please try again.\n", id);
		}

	}

	struct Student* s = &db->StudentRecord[idx];
	struct Student s_backup = *s;

	printf("\nCMS: Record found.\n");

	print_headers(db);
	print_datarow(db, idx);

	// If extrainput header-value pairs exist, apply them

	char undo_fields_all[100] = "";
	

	if (hvpair_count > 0) {
		//struct HeaderValuePair* hvarray = get_hvarray();

		if (hvp_array == NULL) {
			printf("NULL HVARRAY\n");
			return false;
		}

		for (int i = 0; i < hvpair_count; i++) {
			char og_data[50] = "";
			//printf("Header: %d\n Value: %s\n", hvp_array[i].column_id, hvp_array[i].datapoint);
		
			switch (hvp_array[i].column_id) {
				case COL_ID:
					//printf("ID cannot be modified.\n");
					snprintf(og_data, sizeof(og_data), "=%d", s->id);
					s->id = atoi(hvp_array[i].datapoint);
					break;
				case COL_NAME:
					snprintf(og_data, sizeof(og_data), "=%s", s->name);
					strncpy_s(s->name, sizeof(s->name), hvp_array[i].datapoint, _TRUNCATE);
					break;
				case COL_PROGRAMME:
					snprintf(og_data, sizeof(og_data), "=%s", s->programme);
					strncpy_s(s->programme, sizeof(s->programme), hvp_array[i].datapoint, _TRUNCATE);
					break;
				case COL_MARK:
					snprintf(og_data, sizeof(og_data), "=%.1f", s->mark);
					s->mark = atof(hvp_array[i].datapoint);
					break;
			}

			if (hvp_array[i].column_id != COL_OTHER) {
				int col_index = get_column(hvp_array[i].column_id);

				// this adds a space before the next field
				strncat_s(undo_fields_all, sizeof(undo_fields_all), " ", _TRUNCATE);

				// eg concat Mark=23.4
				strncat_s(undo_fields_all, sizeof(undo_fields_all), db->columns[col_index].header_name, _TRUNCATE);
				strncat_s(undo_fields_all, sizeof(undo_fields_all), og_data, _TRUNCATE);


			}


		}

	}
	// If no extrainput go to interactive update mode
	else {
		/*printf("No extrainput given or invalid\n");*/
		//printf("\n=== Enter New Data (Press Enter to skip) ===\n\n");

		char buf[100];

		for (int i = 0; i < db->column_count; i++) {
			if (db->columns[i].column_id == COL_ID) {	// dont allow user to update id
				continue;
			}
			int valid_input = 0;
			while (!valid_input) {
				printf("Enter new %s (Enter = skip): ", db->columns[i].header_name);

				if (!fgets(buf, sizeof(buf), stdin)) continue; // read input
				size_t len = strlen(buf);
				if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0'; // remove newline

				if (strlen(buf) == 0) { //skip if user just pressed enter
					valid_input = 1;
					continue;
				}
				// allow user to type back to go back a column or exit to return to main menu
				if (_stricmp(buf, "back") == 0) {
					int scouting_i = back_column(db, i);
					if (scouting_i >= 0) {
						i = scouting_i;
						continue;
					}
					else {
						printf("No previous column.\n");
						continue;
					}
				}
				else if (_stricmp(buf, "exit") == 0) {
					printf("Cancelling UPDATE ...\n");
					*s = s_backup;
					return false;
				}

				switch (db->columns[i].column_id) {
					case COL_NAME:
						if (!validate_name(buf, idx, CMD_UPDATE)) {
							strncpy_s(s->name, sizeof(s->name), buf, _TRUNCATE);
							valid_input = 1;
						}
						break;

					case COL_PROGRAMME:
						if (!validate_name(buf, idx, CMD_UPDATE) && !validate_programme(buf, idx, CMD_UPDATE)) {
							strncpy_s(s->programme, sizeof(s->programme), buf, _TRUNCATE);
							valid_input = 1;
						}
						break;

					case COL_MARK: {
						float mark = validate_mark(buf, idx, CMD_UPDATE); // returns -1 if invalid
						if (mark >= 0.0f) {
							s->mark = mark;
							valid_input = 1;
						}
						break;
					}
					case COL_OTHER:	// skip
						valid_input = 1;
						break;
					}
			}
		}
	}
	// Update column width for table formatting
	update_width(db, idx, CMD_UPDATE);

	char undo_command[100];
	snprintf(undo_command, sizeof(undo_command), "UPDATE ID=%d%s", id, undo_fields_all);

	insert_undostack(undo_command);

	printf("\nCMS: Record with ID=%d successfully updated.\n", id);
	return true;
}

// =====================================================
// INSERT FUNCTION
// =====================================================
bool insert_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL || db->StudentRecord == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	int id = 0;
	int* id_ptr = &id;	// for passing by reference to extract_extrainput_id

	struct HeaderValuePair hvp_array[10];
	memset(hvp_array, 0, sizeof(hvp_array));

	int hvpair_count = 0;
	if (context != NULL && context[0] != '\0') {
		hvpair_count = extract_extrainput_id(id_ptr, context, db, hvp_array, CMD_INSERT);
	}

	if (id == 0) {
		// ask for ID first, since its the primary key
		char id_buffer[20];

		do {
			printf("CMS: Enter the new student ID: ");
			fgets(id_buffer, sizeof(id_buffer), stdin);

			if (strlen(id_buffer) > 0) {
				id_buffer[strlen(id_buffer) - 1] = '\0';
			}

			if (_stricmp(id_buffer, "exit") == 0) {
				printf("Cancelling INSERT ...\n");
				return false;
			}

		} while (validate_id(id_buffer, -1, db, CMD_INSERT) != 0);

		id = atoi(id_buffer);
	}

	struct Student newStudent = { 0 };
	newStudent.id = id;

	if (hvpair_count > 0) {

		if (hvp_array == NULL) {
			printf("No Header-Value Pair array found\n");
			return false;
		}

		for (int i = 0; i < hvpair_count; i++) {
			//printf("Header: %d\n Value: %s\n", hvp_array[i].column_id, hvp_array[i].datapoint);

			switch (hvp_array[i].column_id) {
			case COL_ID:	// ID is calculated before the rest of the fields and so isnt in this array
				break;
			case COL_NAME:
				strncpy_s(newStudent.name, sizeof(newStudent.name), hvp_array[i].datapoint, _TRUNCATE);
				break;
			case COL_PROGRAMME:
				strncpy_s(newStudent.programme, sizeof(newStudent.programme), hvp_array[i].datapoint, _TRUNCATE);
				break;
			case COL_MARK:
				newStudent.mark = atof(hvp_array[i].datapoint);
				break;
			}
		}


	}
	else {
		//printf("Handle normally\n");
		char buf[100];
		printf("\nInsert New Student\n\n");

		for (int i = 0; i < db->column_count; i++) {
			int col = db->columns[i].column_id;

			switch (col) {
			case COL_ID:	// alr given, skip
				continue;
			case COL_OTHER:	//unexpected column, skip
				continue;
			}

			int valid_input = 0;
			while (!valid_input) {
				printf("CMS: Enter %s (Enter to skip): ", db->columns[i].header_name);
				fgets(buf, sizeof(buf), stdin);

				if (strlen(buf) > 0) {
					buf[strlen(buf) - 1] = '\0';
				}

				if (strlen(buf) == 0) {	//empty (user pressed enter)
					switch (col) {
					case COL_NAME:
						strncpy_s(newStudent.name, sizeof(newStudent.name), "N/A", _TRUNCATE);
						break;
					case COL_PROGRAMME:
						strncpy_s(newStudent.programme, sizeof(newStudent.programme), "N/A", _TRUNCATE);
						break;
					case COL_MARK:
						newStudent.mark = -1.0f;
						break;
					}

					valid_input = 1;
					continue;
				}

				if (_stricmp(buf, "back") == 0) {
					int scouting_i = back_column(db, i);
					if (scouting_i >= 0) {
						i = scouting_i;
						continue;
					}
					else {
						printf("No previous column.\n");
						continue;
					}
				}
				else if (_stricmp(buf, "exit") == 0) {
					printf("Cancelling INSERT ...\n");
					return false;
				}

				//continue looping if not valid
				switch (col) {
					case COL_MARK:
						if (!(validate_mark(buf, -1, CMD_INSERT) < 0)) {	// -1.0 return means not validate mark
							newStudent.mark = atof(buf);
							valid_input = 1;
						}
						break;
					case COL_NAME:
						if (!validate_name(buf, -1, CMD_INSERT)) {
							strncpy_s(newStudent.name, sizeof(newStudent.name), buf, _TRUNCATE);
							valid_input = 1;
						}
						break;
					case COL_PROGRAMME:
						if (!validate_name(buf, -1, CMD_INSERT) && !validate_programme(buf, -1, CMD_INSERT)) {
							strncpy_s(newStudent.programme, sizeof(newStudent.programme), buf, _TRUNCATE);
							valid_input = 1;
						}
						break;
					case COL_OTHER:
						valid_input = 1;
						break;
					}	
			}
		}
	}

	if (!add_student(newStudent)) {
		printf("CMS: Failed to insert student into database.\n");
		return false;
	}

	update_width(db, db->size - 1, CMD_INSERT);

	char undo_command[100];
	snprintf(undo_command, sizeof(undo_command), "DELETE %d", newStudent.id);

	insert_undostack(undo_command);

	printf("CMS: New student inserted successfully.\n");

	return true;
}

// =====================================================
// QUERY FUNCTION
// =====================================================
bool query_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	if (context == NULL || strlen(context) == 0) {
		char user_prompt[100] = "Please provide a keyword to query columns ";
		const char* sep = "";

		for (int col_index = 0; col_index < db->column_count; col_index++) {
			if (db->columns[col_index].column_id == COL_OTHER || db->columns[col_index].column_id == COL_MARK) {
				continue;
			}
			strncat_s(user_prompt, sizeof(user_prompt), sep, _TRUNCATE);
			strncat_s(user_prompt, sizeof(user_prompt), db->columns[col_index].header_name, _TRUNCATE);
			sep = ", ";
		}

		printf("%s with: ", user_prompt);
		char input[100];
		fgets(input, sizeof(input), stdin);
		clean_input(input);
		context = input;
	}

	char keyword[100];
	strncpy_s(keyword, sizeof(keyword), context, _TRUNCATE);
	for (int i = 0; keyword[i]; i++) keyword[i] = tolower(keyword[i]);

	bool found = false;
	printf("Results for keyword \"%s\":\n", context);
	print_headers(db);

	for (int i = 0; i < db->size; i++) {	// this works even if id, name or programme column isnt present
		char name[100], programme[100];
		strncpy_s(name, sizeof(name), db->StudentRecord[i].name, _TRUNCATE);
		strncpy_s(programme, sizeof(programme), db->StudentRecord[i].programme, _TRUNCATE);
		for (int j = 0; name[j]; j++) name[j] = tolower(name[j]);
		for (int j = 0; programme[j]; j++) programme[j] = tolower(programme[j]);

		char id_str[20];
		sprintf_s(id_str, sizeof(id_str), "%d", db->StudentRecord[i].id);

		if (strstr(name, keyword) || strstr(programme, keyword) || strstr(id_str, keyword)) {
			print_datarow(db, i);
			found = true;
		}
	}

	if (!found) {
		printf("No matching record found for keyword \"%s\".\n", context);
	}
	return true;
}

// =====================================================
// SAVE FUNCTION
// =====================================================
bool save_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: No Database loaded. Please OPEN a file first.\n");
		return false;
	}

	save_database(db, db->filepath);

	printf("CMS: File saved successfully.\n");
	return true;
}

// Get string length of a student field based on column
int get_student_field_len(struct Student* s, struct ColumnMap* col) {
	char buf[32];
	switch (col->column_id) {
	case COL_ID:
		sprintf_s(buf, sizeof(buf), "%d", s->id);
		return (int)strlen(buf);
	case COL_NAME:
		return (int)strlen(s->name);
	case COL_PROGRAMME:
		return (int)strlen(s->programme);
	case COL_MARK:
		sprintf_s(buf, sizeof(buf), "%.1f", s->mark);
		return (int)strlen(buf);
	default:
		return 0;
	}
}

// Recalculate max width for a specific column
int recalc_column_max(struct Database* db, struct ColumnMap* col) {
	int max_len = (int)strlen(col->header_name); // start with header length
	for (int r = 0; r < db->size; r++) {
		int len = get_student_field_len(&db->StudentRecord[r], col);
		if (len > max_len) max_len = len;
	}
	return max_len;
}

// Main update_width function
void update_width(struct Database* db, int row_idx, CmdAction action) {
	if (!db || !db->StudentRecord || row_idx < 0) return;

	for (int i = 0; i < db->column_count; i++) {
		struct ColumnMap* col = &db->columns[i];
		int old_width = col->max_width;
		int row_len = 0;

		// Only compute row_len for INSERT or UPDATE (existing row)
		if (action != CMD_DELETE && row_idx < db->size) {
			row_len = get_student_field_len(&db->StudentRecord[row_idx], col);
		}
		////printf("[DEBUG] Column %d (%s): old_width=%d, row_len=%d\n",
		//	i, col->header_name, old_width, row_len);
		if (action == CMD_INSERT) {
			if (row_len > col->max_width)
				col->max_width = row_len;
			//printf("[DEBUG] New width after INSERT: %d\n", col->max_width);
		}
		else if (action == CMD_UPDATE) {
			if (row_len > col->max_width) {
				col->max_width = row_len;
			}
			else if (old_width == col->max_width) {
				col->max_width = recalc_column_max(db, col);
			}
			//printf("[DEBUG] New width after UPDATE: %d\n", col->max_width);
		}
		else if (action == CMD_DELETE) {
			col->max_width = recalc_column_max(db, col);
			//printf("[DEBUG] New width after DELETE: %d\n", col->max_width);
		}
	}
}

bool help_fn(char* context) {
	FILE* file = fopen("src\\Commands.txt", "r");

	if (file == NULL) {
		printf("Help file not available.\n");
		return false;
	}

	char line_buffer[256];
	int index = 1;

	while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
		printf("%d. %s", index, line_buffer);
		index++;
	}

	fclose(file);
	return true;
}


// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", 1, open_fn},				//1
	{"SHOW ALL", 2, showall_fn},		//2
	{"SORT", 1, sort_fn},				//3
	{"DELETE", 1, delete_fn},			//4
	{"UNDO", 1, undo_fn},				//5
	{"SHOW SUMMARY", 2, summary_fn},	//6
	{"UPDATE", 1, update_fn},			//7
	{"INSERT", 1, insert_fn},			//8
	{"QUERY", 1, query_fn},				//9
	{"SAVE", 1, save_fn},				//10
	{"NEW FILE",2, newfile_fn},			//11
	{"HELP", 1, help_fn}				//12
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
		//printf("checking context in loops %s\n", context);

		for (int i = 0; i < num_of_operations; i++) {
			if (_stricmp(callphrase, operations[i].name) == 0) {
				// printf("%s is equal to %s\n", callphrase, operations[i].name);
				command_found = true;
				//printf("context in run_command %s\n", context);
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
		printf("Command %s not recognised. You may use \"help\" to receive a list of available commands.\n", command);
	}

	free(command_copy);
	return 0;
}
