#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
	if (_stricmp(header_name, "programme") == 0 ||
		_stricmp(header_name, "course") == 0) {
		return COL_PROGRAMME;
	}
	if (_stricmp(header_name, "mark") == 0 ||
		_stricmp(header_name, "marks") == 0 ||
		_stricmp(header_name, "score") == 0) {
		return COL_MARK;
	}

	return COL_OTHER;
}

// since ID is like the primary key of the db, must be unique and correct
int validate_id(char* id, int row_number, struct Database* StudentDB) {
	int year = 25;	// zktodo: put this in struct? or make this auto calc from curr year?
	int id_value = atoi(id);

	for (int i = 0; i < strlen(id); i++) {
		if (!isdigit(id[i])) {
			printf("Row %d, ID %s contains invalid character '%c'. Skipping row.\n", row_number, id, id[i]);
			return 1;
		}
	}

	if (strlen(id) != 7) {		// must be 7 digits
		printf("Row %d, ID %s has length of %d, must be 7. Skipping row.\n", row_number, id, (int)strlen(id));
		return 1;
	}	
	if (id_value < 0 ||							// eg 2600000 onwards not allowed
		id_value >= ((year + 1) * 100000)) {		
		printf("Row %d, ID %s is outside of valid ID range. Skipping row.\n", row_number, id);
		return 1;
	}		

	for (int student_index = 0; student_index < StudentDB->size; student_index++) {
		if (StudentDB->StudentRecord[student_index].id == id_value) {
			printf("Row %d, ID %d is already in use", row_number, id_value);

			// print out the name if name columns is included and name is not NULL
			if (StudentDB->StudentRecord[student_index].name != NULL) {
				printf(" by %s", StudentDB->StudentRecord[student_index].name);
			}

			printf(". Skipping row.\n");

			return 1;
		}
	}

	return 0;	// passed validation checks
}

void validate_name(char* name, int row_number) {

	char* name_copy = strdup(name);

	char* read_ptr = name;
	char* write_ptr = name;
	int capitalise_next = 1;	// start at 1 cuz first letter is capitalised

	while (*read_ptr != '\0') {		
		if (isalpha(*read_ptr) ||	// all characters typically allowed in a name
			*read_ptr == '\'' ||
			*read_ptr == '/' ||
			*read_ptr == '-' ||
			*read_ptr == ' ')
		{
			if (!isalpha(*read_ptr)) {	// eg John Souls, Jimtwo S/O Jim, Jim-Jim
				capitalise_next = 1;
			}

			if (isalpha(*read_ptr)) {
				if (capitalise_next) {
					*write_ptr++ = toupper(*read_ptr);
					capitalise_next = 0;
				}
				else {					// uncapitalise those not supposed to be cap
					*write_ptr++ = tolower(*read_ptr);
				}
			}
			else if (*read_ptr == ' ' && *(read_ptr+1) == ' ') {	// throws away space if next char is also space
				// do nothing
			}
			else {
				*write_ptr++ = *read_ptr;
			}
			
		}

		read_ptr++;
	}
	*write_ptr = '\0';

	// this is last in case name is all invalid chars (eg !@#) and becomes empty
	if (strlen(name) == 0) {
		strcpy_s(name, sizeof(name), "N/A");
		printf("Row %d, %s contains no valid characters.\n", row_number, name_copy);
	}
}

void validate_programme(char* programme, int row_number) {
	char* valid_programmes[] = {
		"Software Engineering",
		"Computer Science",
		"Digital Supply Chain"
	};

	int programme_matched = 0;
	for (int i = 0; i < 3 && programme_matched == 0; i++) {
		if (_stricmp(programme, valid_programmes[i]) == 0) {
			programme_matched = 1;
		}
	}
	if (!programme_matched) {
		strcpy_s(programme, sizeof(programme), "N/A");
		printf("Row %d, programme is not valid.\n", row_number);
	}
}

float validate_mark(char* mark, int row_number) {
	float mark_value = atof(mark);

	if (strlen(mark) == 0) {
		printf("Row %d, Mark is empty.\n", row_number);

		return -1.0f;	// -1 is an impossible value since it falls out of range so it means invalid here
	}

	if (mark_value < 0 || mark_value > 100) {
		printf("Row %d, Mark outside of range %.1f.\n", row_number, mark_value);

		return -1.0f;
	}

	// round off to 1dp (*10 gets rid of first dp, round off, then /10 gives it back)
	mark_value = roundf(mark_value * 10.0f) / 10.0f;
	return mark_value;
}

int parse_headers(char* header_line, struct Database* StudentDB) {
	clean_input(header_line);
	char header_line_copy[50];
	strcpy_s(header_line_copy, strlen(header_line) + 1, header_line);

	int id_found = 0;
	int column_count = 0;
	char* context = NULL;
	char* header;
	// printf("header copy: %s\n", header_line_copy);
	
	int column_max = 4;		// initial max columns for malloc, increase if needed
	//StudentDB->column_count = column_max;		// store for ease of looping
	// StudentDB->columns = malloc(sizeof(struct ColumnMap) * column_max); 
	StudentDB->columns = calloc(column_max, sizeof(struct ColumnMap));	// allocate memory based on no. of cols 
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
		if (column_count >= column_max) {
			column_max *= 2;
			struct ColumnMap* new_columns = realloc(StudentDB->columns, sizeof(struct ColumnMap) * column_max);

			if (new_columns == NULL) {
				printf("Memory reallocation for StudentDB->columns failed.\n");
				free(StudentDB->columns);
				return 1;
			}
			// move pointer over to new memory
			StudentDB->columns = new_columns;

			// 0 the new memory for the rest of the cols
			memset(&StudentDB->columns[column_count], 0, sizeof(struct ColumnMap) * (column_max - column_count));
		}

		// printf("header %d: %s\n", column_count, header);

		// hold on to column headers from input file for printing later
		//StudentDB->columns[column_count].header_name = malloc(strlen(header) + 1);	// allocate memory for each column name in header_line
		strcpy_s(StudentDB->columns[column_count].header_name, strlen(header) + 1, header);

		// store header width for deciding no. of spaces to print (will increase if any datapoints are longer)
		StudentDB->columns[column_count].max_width = strlen(header);

		// map header to expected columns
		StudentDB->columns[column_count].column_id = map_column(header);
		if (StudentDB->columns[column_count].column_id == COL_ID) {
			id_found = 1;
		}
		/*printf("DEBUG: Column %d: header='%s' -> mapped to %d\n",
			column_count, header, StudentDB->columns[column_count].column_id);*/

		//printf("Mapped column %d: %s to id %d\n", column_count, StudentDB->columns[column_count].header_name, StudentDB->columns[column_count].column_id);
		
		/*switch (StudentDB->columns[column_count].column_id) {
		case COL_ID: printf("ID\n"); break;
		case COL_NAME: printf("Name\n"); break;
		case COL_PROGRAMME: printf("Programme\n"); break;
		case COL_MARK: printf("Mark\n"); break;
		}*/

		column_count++;
	}

	if (!id_found) {
		printf("No ID column found. Please try again.\n");
		return 1;
	}

	if (column_count == 0) {
		printf("No column headers were found. Please try again.\n"); //zktodo: maybe if this is the case, default to id, name, programme, mark? (kinda lazy tho)
		return 1;
	}
	StudentDB->column_count = column_count;

	if (StudentDB->columns == NULL) {
		printf("Memory allocation for column mapping failed/Columns NULL.\n");
		return 1; // error occured
	}

	return 0;	// 0 means no error occured
}

int parse_datarow(char* data_line, struct Database* StudentDB, struct Student* current_student, int row_number) {
	clean_input(data_line);

	char* context = NULL;
	char* datapoint;
	int column_index = 0;

	// loop thru to test id validity
	char dataline_copy[255];
	strcpy_s(dataline_copy, sizeof(dataline_copy), data_line);
	for (datapoint = strtok_s(dataline_copy, "\t", &context);
		datapoint != NULL;
		datapoint = strtok_s(NULL, "\t", &context))
	{
		clean_input(datapoint);
		if (StudentDB->columns[column_index].column_id == COL_ID) {
			if (validate_id(datapoint, row_number, StudentDB)) {
				// id is invalid
				/*printf("ID IS INVALID!\n");*/
				return 1;
			}
		}
		column_index++;
	}

	// Reset variables for actual loop thru
	column_index = 0;
	//datapoint = NULL;
	context = NULL;
	float temp;
	for (datapoint = strtok_s(data_line, "\t", &context);
		datapoint != NULL;
		datapoint = strtok_s(NULL, "\t", &context))
	{
		clean_input(datapoint);
		int column_id = StudentDB->columns[column_index].column_id;
		/*if (validate_datapoint(datapoint, column_id)) {
			printf("Data point failed verification.\n");
			continue;
		}*/

		// fill in current_student data based on header mapping found
		switch (column_id) {
			case COL_ID:	// no validation cuz alr done
				sscanf_s(datapoint, "%d", &current_student->id);
				break;
			case COL_NAME:
				validate_name(datapoint, row_number);	// proper capitalisation, removes duped spaces
				strcpy_s(current_student->name, strlen(datapoint) + 1, datapoint);
				break;
			case COL_PROGRAMME:
				validate_name(datapoint, row_number);	// proper capitalisation, remove duped spaces as well (zktodo: change name)
				validate_programme(datapoint, row_number);
				strcpy_s(current_student->programme, strlen(datapoint) + 1, datapoint);
				break;
			case COL_MARK:
				current_student->mark = validate_mark(datapoint, row_number);
				break;
			case COL_OTHER:	// no validation
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

	//struct Database* StudentDB = malloc(sizeof(struct Database));	
	struct Database* StudentDB = calloc(1, sizeof(struct Database));	// use calloc in case any fields are read before assigning
	
	//i doubt the allocated  memory needs to change cuz the records part is a ptr
	if (StudentDB == NULL) {
		printf("Memory allocation for StudentDB failed\n");
		return NULL;
	}

	int capacity = 8; // initial capacity, will increase if needed

	// allocate memory for initial members of StudentRecord 
	// (needed because no. of members if decided by file input)
	StudentDB->StudentRecord = calloc(capacity, sizeof(struct Student));
	if (StudentDB->StudentRecord == NULL) {
		printf("Memory allocation for StudentRecord failed.\n");
		free_database(StudentDB);	// free this to prevent leak
		return NULL;
	}

	StudentDB->capacity = capacity;

	int line_counter = 0;
	char line_buffer[255];
	int student_index = 0;

	while (fgets(line_buffer, sizeof(line_buffer), file)) {
		//printf("Line: %s\n", line_buffer);
		line_counter++;

		//printf("Line number %d\n", line_counter);

		if (line_counter == 1) {
			sscanf_s(line_buffer, "Database Name: %[a-zA-Z _]", StudentDB->databaseName, (unsigned int)sizeof(StudentDB->databaseName));
			//printf("Database Name: %s\n", StudentDB->databaseName);
		}
		else if (line_counter == 2) {
			sscanf_s(line_buffer, "Authors: %[a-zA-Z _,]", StudentDB->authors, (unsigned int)sizeof(StudentDB->authors));
			//printf("Authors: %s\n", StudentDB->authors);
		}
		else if (line_counter == 4) {
			//printf("check line 3: %s\n", line_buffer);
			sscanf_s(line_buffer, "Table Name: %[a-zA-Z _]", StudentDB->tableName, (unsigned int)sizeof(StudentDB->tableName));
			//printf("Table Name: %s\n", StudentDB->tableName);
		}
		else if (line_counter == 5) {
			// parse headers, check against StudentDB->columns to see if all there
			// those not there, have to track

			if (parse_headers(line_buffer, StudentDB)) {
				//printf("error occured\n");
				return NULL;
			}
		}

		if (line_counter <= 5) {
			continue;
		}
		
		// reallocate memory for StudentRecord if capacity gets exceeded
		if (student_index >= capacity) {
			capacity *= 2;	// increase limit if exceeded, times 2 seems to be standard method
			struct Student* new_records = realloc(StudentDB->StudentRecord, capacity * sizeof(struct Student));

			if (new_records == NULL) {
				printf("StudentRecord memory reallocation failed!\n");
				free_database(StudentDB);
				return NULL;
			}

			StudentDB->StudentRecord = new_records;	// point to new_records instead
			StudentDB->capacity = capacity;

			// 0 the new memory too
			memset(&StudentDB->StudentRecord[student_index], 0, (capacity - student_index) * sizeof(struct Student));
		}

		if (parse_datarow(line_buffer, StudentDB, &StudentDB->StudentRecord[student_index], line_counter - 5)) { //line_counter - 5 cuz row 6 is first data row
			//printf("Error parsing data row at line %d\n", line_counter);
			continue;	// irreparable error (id invalid)
		}
		else {
			printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
			student_index,
			StudentDB->StudentRecord[student_index].id,
			StudentDB->StudentRecord[student_index].name,
			StudentDB->StudentRecord[student_index].programme,
			StudentDB->StudentRecord[student_index].mark);
		}

		student_index++;
		// altho index starts from 0, no need to student_index +1 cuz it ++s itself right above this
		StudentDB->size = student_index;	// update every loop so validate_id can loop correctly
	}

	//StudentDB->size = student_index;

	return StudentDB;
}

void free_database(struct Database* db) {
	if (db != NULL) {
		if (db->StudentRecord != NULL) {
			free(db->StudentRecord);
		}
		if (db->columns != NULL) {
			free(db->columns);
		}
		free(db);
	}
}

// Jaison addition
// Duplicate ColumnMap
struct ColumnMap* cpyColumnMap(const struct ColumnMap* src, int count) {
	if (src == NULL || count <= 0) {
		return NULL;
	}
	struct ColumnMap* dest = malloc(count * sizeof(struct ColumnMap));
	if (dest == NULL) return NULL;

	for (int i = 0; i < count; i++) {
		dest[i].column_id = src[i].column_id;
		strcpy_s(dest[i].header_name, sizeof(src->header_name), src[i].header_name);
		dest[i].max_width = src[i].max_width;
	}
	return dest;
}

// Duplicate Database details (excluding StudentRecord)
struct Database* cpyDatabaseDetails(const struct Database* src, struct Database* dest) {
		if (src == NULL || dest == NULL) {
		return NULL;
	}
	strcpy_s(dest->databaseName, sizeof(dest->databaseName), src->databaseName);
	strcpy_s(dest->authors, sizeof(dest->authors), src->authors);
	strcpy_s(dest->tableName, sizeof(dest->tableName), src->tableName);
	strcpy_s(dest->filepath, sizeof(dest->filepath), src->filepath);
	dest->capacity = src->capacity;
	dest->column_count = src->column_count;
	dest->columns = cpyColumnMap(src->columns, src->column_count);
	return dest;
}