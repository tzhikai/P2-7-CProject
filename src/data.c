#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

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
	
	//struct Student* StudentRecord = (struct Student*)malloc(initial_size);

	if (StudentDB->StudentRecord == NULL) {
		printf("Memory allocation for StudentRecord failed.\n");
		return NULL;
	}

	int line_counter = 0;
	char line_buffer[255];
	int student_index = 0;

	struct Student* record = StudentDB->StudentRecord;	// shortcut to type less

	while (fgets(line_buffer, sizeof(line_buffer), file)) {
		//printf("Line: %s", line_buffer);
		line_counter++;
		//printf("Line number %d\n", line_counter);
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

		// %[a-zA-Z ] = any alphabetical, case insensitive, and spaces
		sscanf_s(line_buffer, "%d\t%[a-zA-Z ]\t%[a-zA-Z ]\t%f", 
							&record[student_index].id,	//casting cuz sscan_f expects unsigned int
							record[student_index].name, (unsigned int)sizeof(record[student_index].name),
							record[student_index].programme, (unsigned int)sizeof(record[student_index].programme),
							&record[student_index].mark);


		printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
			student_index,
			record[student_index].id,
			record[student_index].name,
			record[student_index].programme,
			record[student_index].mark);
		student_index++;
	}

	StudentDB->memory = memory;
	StudentDB->size = student_index + 1; //+1 because index starts from 0 (helps w looping)

	return StudentDB;
}