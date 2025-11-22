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
// zkchange: should be bool since thats what run_command expects, and theres no need to return the newDB when we use set_database
bool delete_fn(char* context) {
	char cnfm[6];
	char idbuffer[10] = "";

	// means user typed something after delete command
	if (context != NULL && context[0] != '\0') {
		clean_input(context);
		strncpy_s(idbuffer, sizeof(idbuffer), context, _TRUNCATE);
		printf("User gave ID to delete: %s\n", idbuffer);
	}

	int deleting = 1;
	while (deleting == 1) {
		struct Database* StudentDB = get_database(); // Initialize existing struct Student Database

		if (StudentDB == NULL) {
			printf("\nNo records in database.");
			break;
		}
		
		if (idbuffer[0] == '\0') {	// means user just typed DELETE, no id initially given
			printf("\nID to Delete: ");
			fgets(idbuffer, sizeof(idbuffer), stdin);
			clean_input(idbuffer); //Accept id as string for cleanup
		}

		//for (int i = 0; i < StudentDB->size; i++) {	
		//	if (validate_id(idbuffer, i, StudentDB) == 1) {
		//		printf("\nPlease enter a valid ID");
		//		continue;
		//	}
		//}

		//zkchange: i made this make sense (altho this 0 1 2 system is not ideal)
		//if (validate_id(idbuffer, 0, StudentDB) < 2) {	// 0 = valid, unused id; 1 = invalid id; 2 = valid, duplicate id
		//	printf("\nPlease enter a valid ID");
		//	idbuffer[0] = '\0'; // prompts fgets again to ask user for id again
		//	continue;
		//}

		switch (validate_id(idbuffer, 0, StudentDB)) {
			case 1:	// ID is invalid
				printf("\nPlease enter a valid ID.");
				idbuffer[0] = '\0'; // prompts fgets again to ask user for id again
				continue;
			case 0:	// ID is valid but not in use, nothing to delete
				printf("\nID is not in use.");
				idbuffer[0] = '\0';
				continue;
			case 2:	// ID is valid and in use, proceed to deleting
				break;
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

		int cnfmdeleting = 0; // Initialize cnfmdeleting to enter confirmation loop if ID to delete exists

		// find index of record to delete (returns -1 if not found)
		int indexdelete = id_search(iddelete);	//zkchange i made this a fn 

		if (indexdelete == -1) {
			printf("\nThe record with ID=%d does not exist\n", iddelete); //Debug
			deleting = 0;
			return false;
		}
		else {
			printf("\nFound record ID=%d at index %d\n", iddelete, indexdelete);
			cnfmdeleting = 1; //Starts confirmation loop
		}
			
		// Checks record.id if it matches userinput iddelete
		//for (int i = 0; i < StudentDB->size; i++) {
		//	if (record[i].id == iddelete) {
		//		printf("\nFound record ID=%d at Index=%d", iddelete, i); //Debug
		//		indexdelete = i;
		//		cnfmdeleting = 1; //Starts confirmation loop
		//		break;
		//	}
		//}
		
		//// ID to delete does not exist
		//if (indexdelete == -1) {
		//	printf("\nThe record with ID=%d does not exist\n", iddelete); //Debug
		//	deleting = 0;
		//	break;
		//}

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
				
				if (NEWdb == NULL) {
					printf("\nMemory alocation for new database failed.");
					break;
				}
				
				NEWdb = cpyDatabaseDetails(StudentDB, NEWdb); // Refer to 'jaison addition' in data.c & data.h
				
				NEWdb->StudentRecord = malloc(sizeof(struct Student) * (StudentDB->size - 1));
				NEWdb->size = StudentDB->size - 1;
				int newindex = 0;
				
				

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
}

// jaison sort function

bool sort_fn(char* context) {
	struct Database* StudentDB = get_database(); //struct Student and function get_database() is in data.c
	//int student_count = studentcount(); Old code 

	if (StudentDB == NULL) {
		printf("No Student Records could be found. Please try using OPEN.\n");
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
				return false;
			}

			for (int i = 0; sortchoice[i] != '\0'; i++) {
				if (!isdigit(sortchoice[i])) { // if any char is not a digit, invalid input
					printf("Invalid input, please try again or type \"exit\".\n");
					break;	//continue looping
				}
			}

			sortchoice_int = atoi(sortchoice);

			if (sortchoice_int >= 1 && sortchoice_int <= col_index) {	// if got here, col_index guaranteed to be >= 1
				if (StudentDB->columns[sortchoice_int - 1].column_id == COL_OTHER) {// we dont have handling for unexpected cols zktodo: fix?
					printf("Invalid input, please try again or type \"exit\".\n");
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
		sorting = 0;
	}
	printf("\nSorting Successful.\n");
	return true;
}

bool undo_fn(char* context){
	printf("Undo function not yet implemented.\n");

	clean_input(context);

	int undo_count = 1;

	// decide how many undos to do
	if (context != NULL && context[0] != '\0') {

		char buffer[20];
		if (sscanf_s(context, "%d", &buffer) != 0) {
			printf("Invalid undo amount specified, please use an integer.\n");
			return false;
		}

		undo_count = atoi(context);
	}
	printf("here %d\n", undo_count);
	while (undo_count > 0) {
		printf("Performing undo %d...\n", undo_count);
		undo_count--;
	}

	return true;
}

// HY 
void print_summary(struct Database* db) {
	if (db == NULL || db->size == 0) {
		printf("CMS: No records found.\n");
		return;
	}

	float highest = -FLT_MAX;
	float lowest = FLT_MAX;
	float total = 0;
	int highestIndex = 0;
	int lowestIndex = 0;
	int validCount = 0;

	for (int i = 0; i < db->size; i++) {
		float m = db->StudentRecord[i].mark;
		// when mark is negative
		if (m < 0) {
			continue;
		}
		total += m;
		validCount++;

		if (m > highest) {
			highest = m;
			highestIndex = i;
		}
		if (m < lowest) {
			lowest = m;
			lowestIndex = i;
		}
	}
	// Exception handling for negative marks
	if (validCount == 0) {
		printf("CMS: Summary Statistics\n");
		printf("No valid marks to summarize (all marks are negative).\n");
		return;
	}

	float average = total / db->size;

	printf("CMS: Summary Statistics\n");
	printf("Total number of students: %d\n", db->size);
	if (db->size != validCount) {
		printf("Total number of students with valid marks: %d\n", validCount);
	}
	printf("Average mark: %.2f\n", average);

	printf("Highest mark: %.2f (%s)\n",
		highest, db->StudentRecord[highestIndex].name);

	printf("Lowest mark: %.2f (%s)\n",
		lowest, db->StudentRecord[lowestIndex].name);
}

bool summary_fn(char* context) {
	struct Database* db = get_database();
	print_summary(db);
	return true;
}





//hy update
bool update_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL || db->StudentRecord == NULL) {
		printf("CMS: No database loaded. Please OPEN one first.\n");
		return false;
	}
	//printf("context here: %s\n", context);
	int id = 0;
	int* id_ptr = &id;	// for passing by reference to extract_extrainput_id

	/*char* test_str = get_str();
	printf_s("test: %s", test_str);*/
	struct HeaderValuePair hvp_array[10];
	memset(hvp_array, 0, sizeof(hvp_array));
	int hvpair_count = extract_extrainput_id(id_ptr, context, db, hvp_array);
	

	if (id == 0) {
		// ask for ID
		printf("CMS: Enter the student ID to update: ");
		while (scanf_s("%d", &id) != 1) {
			printf("CMS: Invalid ID. Enter a numeric ID: ");
			while (getchar() != '\n'); // clear input buffer
		}
		while (getchar() != '\n'); // remove leftover newline
	}

	// Use your existing id_search()
	int idx = id_search(id);
	if (idx == -1) {
		printf("CMS: The record with ID=%d does not exist.\n", id);
		return false;
	}

	struct Student* s = &db->StudentRecord[idx];

	printf("\nCMS: Record found.\n");
	printf("ID: %d | Name: %s | Programme: %s | Mark: %.1f\n",
		s->id, s->name, s->programme, s->mark);

	if (hvpair_count > 0) {
		//struct HeaderValuePair* hvarray = get_hvarray();

		if (hvp_array == NULL) {
			printf("NULL HVARRAY\n");
			return false;
		}

		for (int i = 0; i < hvpair_count; i++) {
			//printf("Header: %d\n Value: %s\n", hvp_array[i].column_id, hvp_array[i].datapoint);

			switch (hvp_array[i].column_id) {
				case COL_ID:
					s->id = atoi(hvp_array[i].datapoint);
					break;
				case COL_NAME:
					strcpy_s(s->name, sizeof(s->name), hvp_array[i].datapoint);
					break;
				case COL_PROGRAMME:
					strcpy_s(s->programme, sizeof(s->programme), hvp_array[i].datapoint);
					break;
				case COL_MARK:
					s->mark = atof(hvp_array[i].datapoint);
					break;
			}
		}

	}
	else {
		printf("No extrainput given or invalid\n");
		printf("\n=== Enter New Data (Press Enter to skip) ===\n\n");

		char buf[100];

		for (int i = 0; i < db->column_count; i++) {
			printf("Enter new %s (Enter = skip): ", db->columns[i].header_name);

			if (!fgets(buf, sizeof(buf), stdin)) continue; // read input
			size_t len = strlen(buf);
			if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0'; // remove newline

			if (strlen(buf) == 0) continue; // skip if empty

			switch (db->columns[i].column_id) {
			case COL_ID:
				printf("[Skipping ID – cannot be modified]\n");
				break;

			case COL_NAME:
				validate_name(buf, idx); // your validate_name modifies buf in-place
				strcpy_s(s->name, sizeof(s->name), buf);
				break;

			case COL_PROGRAMME:
				validate_name(buf, idx); // capitalize properly
				validate_programme(buf, idx); // ensure valid programme
				strcpy_s(s->programme, sizeof(s->programme), buf);
				break;

			case COL_MARK: {
				float mark = validate_mark(buf, idx); // returns -1 if invalid
				if (mark >= 0.0f) {
					s->mark = mark;
				}
				else {
					printf("Invalid mark. Keeping previous value.\n");
				}
				break;
			}

			default:
				printf("[Unknown column — skipped]\n");
				break;
			}
		}
	}

	printf("\nCMS: Record with ID=%d successfully updated.\n", id);
	return true;
}

// =====================================================
// INSERT FUNCTION
// =====================================================
bool insert_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL || db->StudentRecord == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	struct Student newStudent = { 0 };
	char buf[100];

	printf("\nInsert New Student\n\n");

	for (int i = 0; i < db->column_count; i++) {
		int col = db->columns[i].column_id;

		while (1) {
			printf("Enter %s: ", db->columns[i].header_name);

			if (!fgets(buf, sizeof(buf), stdin))
				continue;
			size_t len = strlen(buf);

			if (len > 0 && buf[len - 1] == '\n') {
				buf[len - 1] = '\0';
			}
			if (strlen(buf) == 0) {
				printf("This field cannot be empty.\n");
				continue;
			}

			switch (col) {
			case COL_ID: {
				switch (validate_id(buf, -1, db)) {
					case 1: //invalid id
						continue;
					case 2: //invalid but duplicate id
						continue;
					case 0: //valid, new id
						newStudent.id = atoi(buf);
						break;
				}
				break;
			}
			case COL_NAME:
				validate_name(buf, -1);
				strcpy_s(newStudent.name, sizeof(newStudent.name), buf);
				break;
			case COL_PROGRAMME:
				validate_name(buf, -1);
				validate_programme(buf, -1);
				strcpy_s(newStudent.programme, sizeof(newStudent.programme), buf);
				break;

			case COL_MARK: {
				float mark = validate_mark(buf, -1);
				if (mark < 0 || mark > 100) {
					printf("Invalid mark. Must be between 0 and 100.\n");
					continue;
				}
				newStudent.mark = mark;
				break;
			}
			case COL_OTHER:
				printf("Unhandled column. Skipping.\n");
				break;
			}

			break;
		}
	}
	add_student(newStudent);
	printf("CMS: New student inserted successfully.\n");

	return true;
}

// =====================================================
// QUERY FUNCTION
// =====================================================
bool query_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	if (context == NULL || strlen(context) == 0) {
		printf("Please provide a keyword (ID, name, or programme): ");
		char input[100];
		fgets(input, sizeof(input), stdin);
		clean_input(input);
		context = input;
	}

	char keyword[100];
	strcpy_s(keyword, sizeof(keyword), context);
	for (int i = 0; keyword[i]; i++) keyword[i] = tolower(keyword[i]);

	bool found = false;
	printf("Results for \"%s\":\n", context);

	for (int i = 0; i < db->size; i++) {
		char name[100], programme[100];
		strcpy_s(name, sizeof(name), db->StudentRecord[i].name);
		strcpy_s(programme, sizeof(programme), db->StudentRecord[i].programme);
		for (int j = 0; name[j]; j++) name[j] = tolower(name[j]);
		for (int j = 0; programme[j]; j++) programme[j] = tolower(programme[j]);

		char id_str[20];
		sprintf_s(id_str, sizeof(id_str), "%d", db->StudentRecord[i].id);

		if (strstr(name, keyword) || strstr(programme, keyword) || strstr(id_str, keyword)) {
			printf("%-8d %-25s %-30s %.1f\n",
				db->StudentRecord[i].id,
				db->StudentRecord[i].name,
				db->StudentRecord[i].programme,
				db->StudentRecord[i].mark);
			found = true;
		}
	}

	if (!found) {
		printf("No matching record found for \"%s\".\n", context);
	}
	return true;
}

// =====================================================
// SAVE FUNCTION
// =====================================================
bool save_fn(char* context) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return false;
	}

	save_database(db, db->filepath);

	printf("CMS: File saved successfully.\n");
	return true;
}



// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"OPEN", 1, open_fn},
	{"SHOW ALL", 2, showall_fn},
	{"SORT", 1, sort_fn},
	{"DELETE", 1, delete_fn},
	{"UNDO", 1, undo_fn},
	{ "SHOW SUMMARY", 2, summary_fn },
	{ "UPDATE", 1, update_fn },
	{"INSERT", 1, insert_fn},
	{"QUERY", 1, query_fn},
	{"SAVE", 1, save_fn}
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
		printf("Command %s not recognised. Please try again.\n", command);
	}

	free(command_copy);
	return 0;
}
