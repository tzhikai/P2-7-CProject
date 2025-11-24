#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "commands.h"
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

int id_search(int id) {
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL) {
		return -1;
	}

	for (int i = 0; i < StudentDB->size; i++) {
		if (StudentDB->StudentRecord[i].id == id) {
			return i;	// returns index of student w matching (unique) id
		}
	}

	return -1;	// could not find the id in StudentRecord

}

// since ID is like the primary key of the db, must be unique and correct
int validate_id(char* id, int row_number, struct Database* StudentDB, CmdAction cmd) {
	int year = 25;	// zktodo: put this in struct? or make this auto calc from curr year?
	int id_value = atoi(id);
	int id_length = strlen(id);

	int result = 0;
	char error[50];

	for (int i = 0; i < id_length; i++) {
		if (!isdigit(id[i])) {
			//printf("Row %d, ID %s contains invalid character '%c'. Skipping row.\n", row_number, id, id[i]);
			snprintf(error, sizeof(error), "ID \"%s\" contains invalid character '%c'", id, id[i]);
			result = 1;
			break;
			//return 1;
		}
	}

	if (id_length != 7) {		// must be 7 digits
		//printf("Row %d, ID %s has length of %d, must be 7. Skipping row.\n", row_number, id, (int)id_length);
		snprintf(error, sizeof(error), "ID \"%s\" has length of %d, must be 7", id, (int)id_length);
		result = 1;
		//return 1;
	}	
	if (id_value < 0 ||			// eg 2600000 onwards not allowed
		id_value >= ((year + 1) * 100000)) {		
		//printf("Row %d, ID %s is outside of valid ID range. Skipping row.\n", row_number, id);
		snprintf(error, sizeof(error), "ID \"%s\" is outside of valid ID range", id);
		result = 1;
		//return 1;
	}		

	for (int student_index = 0; student_index < StudentDB->size; student_index++) {
		if (StudentDB->StudentRecord[student_index].id == id_value) {
			//printf("Row %d, ID %d is already in use", row_number, id_value);
			snprintf(error, sizeof(error), "ID \"%d\" is already in use", id_value);

			// print out the name if name columns is included and name is not NULL
			if (StudentDB->StudentRecord[student_index].name != NULL) {
				//printf(" by %s", StudentDB->StudentRecord[student_index].name);
				strncat_s(error, sizeof(error), " by ", _TRUNCATE);
				strncat_s(error, sizeof(error), StudentDB->StudentRecord[student_index].name, _TRUNCATE);
			}

			//printf(". Skipping row.\n");

			result = 2; // slightly different to differentiate different uses (validate_id > 0 = invalid new id, validate_id < 2 = invalid access id)
			break;
			//return 2;	
		}
	}
	//printf("result: %d", result);
	// printing out the error message (if any)
	if (cmd == CMD_INSERT || cmd == CMD_OPEN) {
		if (result != 0) {
			print_error(cmd, row_number, error, true);
		}
	}
	/*else if (cmd == CMD_UPDATE || cmd == CMD_DELETE) {
		if (result != 2) {
			print_error(cmd, row_number, error, true);
		}
	}*/

	return result;	// passed validation checks
}

int validate_name(char* name, int row_number, CmdAction cmd) {

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
		char error[50];
		snprintf(error, sizeof(error), "\"%s\" contains no valid characters", name_copy);

		print_error(cmd, row_number, error, false);

		strncpy_s(name, sizeof(name), "N/A", _TRUNCATE);
		return 1;
	}
	free(name_copy);
	return 0;
}

int validate_programme(char* programme, int row_number, CmdAction cmd) {
	struct Database* db = get_database();
	int col_id = get_column(COL_PROGRAMME);
	
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
		char error[50];
		snprintf(error, sizeof(error), "%s \"%s\" is not valid", db->columns[col_id].header_name, programme);

		print_error(cmd, row_number, error, false);

		strncpy_s(programme, sizeof(programme), "N/A", _TRUNCATE);	// change to N/A if not valid
		return 1;
	}
	return 0;
}

float validate_mark(char* mark, int row_number, CmdAction cmd) {
	
	struct Database* db = get_database();
	int col_id = get_column(COL_MARK);
	
	float mark_value = atof(mark);
	float result = 0.0;
	char error[50];


	if (strlen(mark) == 0) {
		//printf("Row %d, Mark is empty.\n", row_number);
		snprintf(error, sizeof(error), "%s is empty", db->columns[col_id].header_name);

		result = -1.0f;	// -1 is an impossible value since it falls out of range so it means invalid here
	}

	if (mark_value < 0 || mark_value > 100) {
		//printf("Row %d, Mark outside of range %.1f.\n", row_number, mark_value);
		snprintf(error, sizeof(error), "%s \"%.1f\" outside of range", db->columns[col_id].header_name, mark_value);

		result = -1.0f;
	}

	if (result != 0.0) {
		print_error(cmd, row_number, error, false);
		return result;
	}

	// round off to 1dp (*10 gets rid of first dp, round off, then /10 gives it back)
	mark_value = roundf(mark_value * 10.0f) / 10.0f;

	return mark_value;
}

// for use in INSERT, UPDATE, DELETE (these change datapoints)
void recalc_col_widths(struct Database* StudentDB, struct Student* recordAdded, struct Student* recordToDelete) {
	// if recordAdded is not NULL, we
	
	if (StudentDB == NULL || StudentDB->columns == NULL) {
		return;	//no change
	}

	
}

int parse_headers(char* header_line, struct Database* StudentDB) {
	clean_input(header_line);
	char header_line_copy[50];
	strncpy_s(header_line_copy, strlen(header_line) + 1, header_line, _TRUNCATE);

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
		strncpy_s(StudentDB->columns[column_count].header_name, strlen(header) + 1, header, _TRUNCATE);

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
	strncpy_s(dataline_copy, sizeof(dataline_copy), data_line, _TRUNCATE);
	for (datapoint = strtok_s(dataline_copy, "\t", &context);
		datapoint != NULL;
		datapoint = strtok_s(NULL, "\t", &context))
	{
		clean_input(datapoint);
		if (StudentDB->columns[column_index].column_id == COL_ID) {
			if (validate_id(datapoint, row_number, StudentDB, CMD_OPEN)) {
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
			case COL_NAME: // invalid names (eg. 123) will be replaced with N/A
				validate_name(datapoint, row_number, CMD_OPEN);	// proper capitalisation, removes duped spaces
				strncpy_s(current_student->name, strlen(datapoint) + 1, datapoint, _TRUNCATE);
				break;
			case COL_PROGRAMME: //invalid programmes (eg. Veterinarian) will be replaced with N/A
				validate_name(datapoint, row_number, CMD_OPEN);	// proper capitalisation, remove duped spaces as well (zktodo: change name)
				validate_programme(datapoint, row_number, CMD_OPEN);
				strncpy_s(current_student->programme, strlen(datapoint) + 1, datapoint, _TRUNCATE);
				break;
			case COL_MARK:
				current_student->mark = validate_mark(datapoint, row_number, CMD_OPEN);
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
			set_database(StudentDB); // purpose is to secure the header names within StudentDB to be retrieved during parse_datarows
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
			/*printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
			student_index,
			StudentDB->StudentRecord[student_index].id,
			StudentDB->StudentRecord[student_index].name,
			StudentDB->StudentRecord[student_index].programme,
			StudentDB->StudentRecord[student_index].mark);*/
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
		strncpy_s(dest[i].header_name, sizeof(src->header_name), src[i].header_name, _TRUNCATE);
		dest[i].max_width = src[i].max_width;
	}
	return dest;
}

// Duplicate Database details (excluding StudentRecord)
struct Database* cpyDatabaseDetails(const struct Database* src, struct Database* dest) {
		if (src == NULL || dest == NULL) {
		return NULL;
	}
	strncpy_s(dest->databaseName, sizeof(dest->databaseName), src->databaseName, _TRUNCATE);
	strncpy_s(dest->authors, sizeof(dest->authors), src->authors, _TRUNCATE);
	strncpy_s(dest->tableName, sizeof(dest->tableName), src->tableName, _TRUNCATE);
	strncpy_s(dest->filepath, sizeof(dest->filepath), src->filepath, _TRUNCATE);
	dest->capacity = src->capacity;
	dest->column_count = src->column_count;
	dest->columns = cpyColumnMap(src->columns, src->column_count);
	return dest;
}


// INSERT and QUERY FUNCTIONS
// Kim basic insert of student ids,name,programms and the marks into the system
// Users are able to query the studetn via either IDs,names or programms

// Function to add a new student to the database
bool add_student(struct Student newStudent) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: Please OPEN a database first.\n");
		return false;
	}

	if (db->StudentRecord == NULL) {
		printf("Student Records is NULL.\n");
		return false;
	}

	// Expand array if capacity reached
	//if (db->size >= db->capacity) {
	if ((db->size + 1) >= db->capacity) {
		int newCapacity = db->capacity * 2;
		struct Student* resized = realloc(db->StudentRecord, newCapacity * sizeof(struct Student));
		if (resized == NULL) {
			printf("Memory reallocation failed while inserting new student.\n");
			return false;
		}
		db->StudentRecord = resized;
		db->capacity = newCapacity;
	}

	db->StudentRecord[db->size] = newStudent;
	db->size++;

	printf("CMS: Successfully added new student (ID=%d, Name=%s, Programme=%s, Mark=%.1f)\n",
		newStudent.id, newStudent.name, newStudent.programme, newStudent.mark);
	return true;
}

// Function to search for a student by ID, name, or programme
void query_student(const char* keyword) {
	struct Database* db = get_database();
	if (db == NULL) {
		printf("CMS: Please OPEN the database first.\n");
		return;
	}

	char lowered[100];
	strncpy_s(lowered, sizeof(lowered), keyword, _TRUNCATE);
	for (int i = 0; lowered[i]; i++) lowered[i] = tolower(lowered[i]);

	bool found = false;

	for (int i = 0; i < db->size; i++) {
		char name[100], programme[100];
		strncpy_s(name, sizeof(name), db->StudentRecord[i].name, _TRUNCATE);
		strncpy_s(programme, sizeof(programme), db->StudentRecord[i].programme, _TRUNCATE);
		for (int j = 0; name[j]; j++) name[j] = tolower(name[j]);
		for (int j = 0; programme[j]; j++) programme[j] = tolower(programme[j]);

		char id_str[20];
		sprintf_s(id_str, sizeof(id_str), "%d", db->StudentRecord[i].id);

		if (strstr(name, lowered) || strstr(programme, lowered) || strstr(id_str, lowered)) {
			printf("ID: %d | Name: %s | Programme: %s | Mark: %.1f\n",
				db->StudentRecord[i].id,
				db->StudentRecord[i].name,
				db->StudentRecord[i].programme,
				db->StudentRecord[i].mark);
			found = true;
		}
	}

	if (!found) {
		printf("CMS: No records found for \"%s\".\n", keyword);
	}
}

bool save_database(struct Database* db, const char* filepath) {
	FILE* file = NULL;
	fopen_s(&file, filepath, "w");
	if (file == NULL) {
		printf("Unable to open file for saving: %s\n", filepath);
		return false;
	}

	fprintf(file, "Database Name: %s\n", db->databaseName);
	fprintf(file, "Authors: %s\n\n", db->authors);
	fprintf(file, "Table Name: %s\n", db->tableName);

	// Headers
	for (int c = 0; c < db->column_count; c++) {
		fprintf(file, "%s", db->columns[c].header_name);
		if (c < db->column_count - 1)
			fprintf(file, "\t");
	}
	fprintf(file, "\n");

	// Records
	for (int i = 0; i < db->size; i++) {
		fprintf(file, "%d\t%s\t%s\t%.1f\n",
			db->StudentRecord[i].id,
			db->StudentRecord[i].name ? db->StudentRecord[i].name : "N/A",
			db->StudentRecord[i].programme ? db->StudentRecord[i].programme : "N/A",
			db->StudentRecord[i].mark);

	}

	fclose(file);
	printf("CMS: Database saved successfully to %s\n", filepath);
	return true;
}
