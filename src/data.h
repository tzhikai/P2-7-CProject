#ifndef DATA_H
#define DATA_H

#include "commands.h"

typedef enum {
	COL_ID,
	COL_NAME,
	COL_PROGRAMME,
	COL_MARK,
	COL_OTHER
} Columns;

struct ColumnMap {
	Columns column_id;			
	char header_name[50];		
	int max_width;				// the highest strlen of everything in the col (headers/datapoints)
};

struct Student {
	int id;					// eg 2500123
	char name[50];			// eg Fake Name
	char programme[50];		// eg Computer Science
	float mark;				// eg 82.1
};

struct Database {	// zktodo: reorganise by section w comments
	struct Student* StudentRecord;	// points to Student struct array
	char filepath[50];				// filepath of user input (for saving)
	int capacity;						// curr max amt of students allocated for (doubles whenever needed)
	int size;						// number of students in StudentRecord

	char databaseName[20];		// database name extracted from input file
	char authors[20];				// authors' names extracted from input file
	char tableName[20];				// table name extracted from input file
	//char* columns[20];					// list of all expected columns
	struct ColumnMap* columns;		// points to ColumnMap struct array
	//char** column_names;			// list of column names extracted from input file (to be printed)
	int column_count;				// amt of cols in input file
};

struct Database* load_data(FILE *file);
void free_database(struct Database* db);

void set_database(struct Database* db);
struct Database* get_database();

Columns map_column(char* header_name);

int id_search(int id);

int validate_id(char* id, int row_number, struct Database* StudentDB, CmdAction cmd);
int validate_name(char* name, int row_number, CmdAction cmd);
int validate_programme(char* programme, int row_number, CmdAction cmd);
float validate_mark(char* mark, int row_number, CmdAction cmd);

int parse_headers(char* header_line, struct Database* StudentDB);
int parse_datarow(char* data_line, struct Database* StudentDB, struct Student* current_student, int row_number);

// jaison
struct ColumnMap* cpyColumnMap(const struct ColumnMap* src, int count);
struct Database* cpyDatabaseDetails(const struct Database* src, struct Database* dest);

// hy
struct Summary {
	float average;
	float highest;
	float lowest;
	int highestIndex;
	int lowestIndex;
};

bool save_database(struct Database* db, const char* filepath);

#endif // !DATA_H