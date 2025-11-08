#ifndef DATA_H
#define DATA_H

#include <stdio.h>


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

struct Database* load_data(FILE* file);


void set_database(struct Database* db);
struct Database* get_database();

void add_student(struct Database* db, struct Student newStudent); 
struct Student* find_student_by_id(struct Database* db, int id); //query the exist students through IDs
void save_database(struct Database* db, const char* filepath);  //save insert data into database
#endif // !DATA_H