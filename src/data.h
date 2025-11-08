#ifndef DATA_H



struct Student {
	int id;					// eg 2500123
	char name[50];			// eg Fake Name
	char programme[50];		// eg Computer Science
	float mark;				// eg 82.1
};

struct Database {
	struct Student* StudentRecord;	// points to Student struct array
	int memory;						// amt of allocated memory
	int size;						// number of students in StudentRecord
};

struct Database* load_data(FILE *file);


void set_database(struct Database* db);
struct Database* get_database();

#endif // !DATA_H