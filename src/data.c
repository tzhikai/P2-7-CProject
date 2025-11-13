#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "utils.h"

// static variable to store the student records data
static struct Database* StudentDB = NULL;

// function to call when making changes to 
void set_database(struct Database* db) {
	StudentDB = db;
}
// allow retrieval of StudentDB within other functions in the other files
struct Database* get_database() {
	return StudentDB;
}

Columns map_column(char* header_name) {	// from header of column in input file, figure out which StudentRecord col it corresponds to
	//clean_input(header_name);	// remove whitespaces trailing, leading
	join_words(header_name);

	//printf("Header name: %s\n", header_name);
	
	if (_stricmp(header_name, "id") == 0) {
		return COL_ID;
	}
	if (_stricmp(header_name, "name") == 0) {
		return COL_NAME;
	}
	if (_stricmp(header_name, "programme") == 0) { //zktodo: add || course
		return COL_PROGRAMME;
	}
	if (_stricmp(header_name, "mark") == 0) {	//zktodo: add || grade, score
		return COL_MARK;
	}

	return COL_OTHER;
}

int validate_datapoint(char* datapoint, int column_id) {
	switch (column_id) {
		case COL_ID:
			// 7 digits only, and only digits, no negative
			// id range? no ids after current year so 26xxxxx
			// no dupe id
			break;
		case COL_NAME:
			// only letters, spaces, hyphens, apostrophes
			// A-Za-z \- ' (no numbers, special chars)
			// auto capitalise?
			// consecutive spaces
			break;
		case COL_PROGRAMME:
			// allowed programmes list
			// auto capitalise?
			// consecutive spaces
			break;
		case COL_MARK:
			// 0.0 to 100.0 
			// round off to closest 1dp
			// no negative
			// grading?
			break;
	}

	return 0; // datapoint pass checks
}

int parse_headers(char* header_line, struct Database* StudentDB) {
	clean_input(header_line);
	char header_line_copy[50];
	strcpy_s(header_line_copy, strlen(header_line) + 1, header_line);

	int column_count = 0;
	char* context = NULL;
	char* header;
	// printf("header copy: %s\n", header_line_copy);
	
	int column_max = 6;		// initial max columns for malloc, increase if needed
	//StudentDB->column_count = column_max;		// store for ease of looping
	StudentDB->columns = malloc(sizeof(struct ColumnMap) * column_max); // allocate memory based on no. of cols 
	//StudentDB->column_names = malloc(sizeof(char*) * column_max);	// allocate memory based on no. of cols 

	if (StudentDB->columns == NULL) {
		printf("Memory allocation for StudentDB->columns failed.\n");
		return 1;
	}

	// zktodo: add code for realloc if exceeds column_max

	// iterate thru headers on the header_line (tab delimiter)
	for (header = strtok_s(header_line_copy, "\t", &context);
		header != NULL;
		header = strtok_s(NULL, "\t", &context)) 
	{
		// printf("header %d: %s\n", column_count, header);

		// hold on to column headers from input file for printing later
		//StudentDB->columns[column_count].header_name = malloc(strlen(header) + 1);	// allocate memory for each column name in header_line
		strcpy_s(StudentDB->columns[column_count].header_name, strlen(header) + 1, header);

		// store header width for deciding no. of spaces to print (will increase if any datapoints are longer)
		StudentDB->columns[column_count].max_width = strlen(header);

		// map header to expected columns
		StudentDB->columns[column_count].column_id = map_column(header);

		//printf("Mapped column %d: %s to id %d\n", column_count, StudentDB->columns[column_count].header_name, StudentDB->columns[column_count].column_id);
		
		/*switch (StudentDB->columns[column_count].column_id) {
		case COL_ID: printf("ID\n"); break;
		case COL_NAME: printf("Name\n"); break;
		case COL_PROGRAMME: printf("Programme\n"); break;
		case COL_MARK: printf("Mark\n"); break;
		}*/

		column_count++;
	}

	if (column_count == 0) {
		printf("No column headers were found. Please try again.\n"); //zktodo: maybe if this is the case, default to id, name, programme, mark? (kinda lazy tho)
	}
	StudentDB->column_count = column_count;

	if (StudentDB->columns == NULL) {
		printf("Memory allocation for column mapping failed.\n");
		return 1; // error occured
	}

	return 0;	// 0 means no error occured
}

int parse_datarow(char* data_line, struct Database* StudentDB, struct Student* current_student) {
	clean_input(data_line);

	char* context = NULL;
	char* datapoint;
	int column_index = 0;

	for (datapoint = strtok_s(data_line, "\t", &context);
		datapoint != NULL;
		datapoint = strtok_s(NULL, "\t", &context))
	{
		int column_id = StudentDB->columns[column_index].column_id;
		if (validate_datapoint(datapoint, column_id)) {
			printf("Data point failed verification.\n");
			continue;
		}

		// fill in current_student data based on header mapping found
		switch (column_id) {
		case COL_ID:
			sscanf_s(datapoint, "%d", &current_student->id);
			break;
		case COL_NAME:
			strcpy_s(current_student->name, strlen(datapoint) + 1, datapoint);
			break;
		case COL_PROGRAMME:
			strcpy_s(current_student->programme, strlen(datapoint) + 1, datapoint);
			break;
		case COL_MARK:
			sscanf_s(datapoint, "%f", &current_student->mark);
			break;
		case COL_OTHER:
			break;
		}
		// measure if saved max_width is surpassed, and edit if it is
		if (StudentDB->columns[column_index].max_width < strlen(datapoint)) {
			StudentDB->columns[column_index].max_width = strlen(datapoint);
		}

		//printf("Parsed row %d: %s\n", column_index, datapoint);
		column_index++;
	}

	return 0;	// 0 means no error occured
}


struct Database* load_data(FILE *file) {
	int capacity = 4;
	int memory = sizeof(struct Student) * capacity;	// allow space for 4 Student struct array members
	
	struct Database* StudentDB = malloc(sizeof(struct Database));	
	
	//i doubt the allocated  memory needs to change cuz the records part is a ptr
	if (StudentDB == NULL) {
		printf("Memory allocation for StudentDB failed\n");
		return NULL;
	}

	// allocate memory for initial members of StudentRecord 
	// (needed because no. of members if decided by file input)
	StudentDB->StudentRecord = malloc(memory);
	if (StudentDB->StudentRecord == NULL) {
		printf("Memory allocation for StudentRecord failed.\n");
		return NULL;
	}

	int line_counter = 0;
	char line_buffer[255];
	int student_index = 0;

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less

	while (fgets(line_buffer, sizeof(line_buffer), file)) {
		//printf("Line: %s\n", line_buffer);
		line_counter++;
		//printf("Line number %d\n", line_counter);

		if (line_counter == 1) {
			sscanf_s(line_buffer, "Database Name: %[a-zA-Z _]", StudentDB->databaseName, (unsigned int)sizeof(StudentDB->databaseName));
			printf("table name: %s\n", StudentDB->databaseName);
		}
		else if (line_counter == 2) {
			sscanf_s(line_buffer, "Authors: %[a-zA-Z _,]", StudentDB->authors, (unsigned int)sizeof(StudentDB->authors));
			printf("table name: %s\n", StudentDB->authors);
		}
		else if (line_counter == 4) {
			//printf("check line 3: %s\n", line_buffer);
			sscanf_s(line_buffer, "Table Name: %[a-zA-Z _]", StudentDB->tableName, (unsigned int)sizeof(StudentDB->tableName));
			printf("table name: %s\n", StudentDB->tableName);
		}
		else if (line_counter == 5) {
			// parse headers, check against StudentDB->columns to see if all there
			// those not there, have to track

			if (parse_headers(line_buffer, StudentDB)) {
				printf("error occured\n");
			}
		}

		if (line_counter <= 5) {
			continue;
		}
		

		// zktodo: insert code to reallocate memory if exceeds
		/*if (student_index >= capacity) {
			capacity *= 2;
			struct Student* temp = realloc(StudentRecord, capacity);

			if (temp == NULL) {
				printf("Memory reallocation failed");
				free(StudentRecord);
				break;
			}
			else {
				StudentRecord = temp;
			}
		}*/

		if (parse_datarow(line_buffer, StudentDB, &record[student_index])) {
			printf("Error parsing data row at line %d\n", line_counter);
		}
		else {
			printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
			student_index,
			record[student_index].id,
			record[student_index].name,
			record[student_index].programme,
			record[student_index].mark);
		}

		student_index++;
	}

	StudentDB->memory = memory;
	StudentDB->size = student_index;
	// altho index starts from 0, no need to student_index +1 cuz i student_index++ at the end of the loop anyway

	return StudentDB;
}