#ifndef DATA_H
#define DATA_H

#include "commands.h"

typedef enum {		// enum to identify column types
	COL_ID,
	COL_NAME,
	COL_PROGRAMME,
	COL_MARK,
	COL_OTHER
} Columns;

struct ColumnMap {				// stores information about each column
	Columns column_id;			// type of column (ID, Name, etc.)
	char header_name[50];		// column name from file
	int max_width;				// the highest strlen of everything in the col (headers/datapoints)
};

struct Student {
	int id;					// eg 2500123
	char name[50];			// eg Fake Name
	char programme[50];		// eg Computer Science
	float mark;				// eg 82.1
};

struct Database {
	struct Student* StudentRecord;	// points to Student struct array
	char filepath[50];				// filepath of user input (for saving)
	int capacity;						// curr max amt of students allocated for (doubles whenever needed)
	int size;						// number of students in StudentRecord

	char databaseName[20];		// database name extracted from input file
	char authors[20];				// authors' names extracted from input file
	char tableName[20];				// table name extracted from input file
	struct ColumnMap* columns;		// points to ColumnMap struct array
	int column_count;				// amt of cols in input file
};

struct Database* load_data(FILE *file);		// loads data from file into database
void free_database(struct Database* db);	// frees allocated memory

void set_database(struct Database* db);		// sets current active database
struct Database* get_database();			// gets current active database

Columns map_column(char* header_name);		// converts header name to column name

int id_search(int id);						// searches for student by id, returns index

int validate_id(char* id, int row_number, struct Database* StudentDB, CmdAction cmd);		// validates ID field
int validate_name(char* name, int row_number, CmdAction cmd);								// validates name field
int validate_programme(char* programme, int row_number, CmdAction cmd);						// validates programme field
float validate_mark(char* mark, int row_number, CmdAction cmd);								// validates mark field

int parse_headers(char* header_line, struct Database* StudentDB);							// reads and processes header line
int parse_datarow(char* data_line, struct Database* StudentDB, struct Student* current_student, int row_number); // parse_headers

void print_headers(struct Database* StudentDB);												// prints all columnn headers
void print_datarow(struct Database* StudentDB, int student_index);							// prints one student row


int get_student_field_len(struct Student* s, struct ColumnMap* col);						// gets length of specific student field
int recalc_column_max(struct Database* db, struct ColumnMap* col);							// recalclates max width of a column
void update_width(struct Database* db, int row_idx, CmdAction action);						// updates column widths based on action

bool add_student(struct Student newStudent);												// adds a new student to the database
bool save_database(struct Database* db, const char* filepath);								// saves database to file

#endif // !DATA_H