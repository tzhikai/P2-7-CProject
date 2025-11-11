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
};

struct Student {
	int id;					// eg 2500123
	char name[50];			// eg Fake Name
	char programme[50];		// eg Computer Science
	float mark;				// eg 82.1
};

struct Database {	// zktodo: reorganise by section w comments
	struct Student* StudentRecord;	// points to Student struct array
	int memory;						// amt of allocated memory
	int size;						// number of students in StudentRecord
	char tableName[20];				// table name extracted from input file
	//char* columns[20];					// list of all expected columns
	struct ColumnMap* columns;		// points to ColumnMap struct array
	//char** column_names;			// list of column names extracted from input file (to be printed)
	int column_count;				// amt of cols in input file
};

struct Database* load_data(FILE *file);


void set_database(struct Database* db);
struct Database* get_database();

#endif // !DATA_H