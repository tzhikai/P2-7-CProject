#ifndef DATA_H

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
	int memory;						// amt of allocated memory
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


void set_database(struct Database* db);
struct Database* get_database();

Columns map_column(char* header_name);

int validate_datapoint(char* datapoint, int column_id);

int parse_headers(char* header_line, struct Database* StudentDB);
int parse_datarow(char* data_line, struct Database* StudentDB, struct Student* current_student);

#endif // !DATA_H