#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"



void load_data(FILE *file) {
	int capacity = 4;
	int initial_size = sizeof(struct Student) * capacity;	// allow space for 5 Student struct array members
	
	// allocate memory for initial members of StudentRecord 
	// (needed because no. of members if decided by file input)
	struct Student* StudentRecord = malloc(initial_size);
	//struct Student* StudentRecord = (struct Student*)malloc(initial_size);

	int line_counter = 0;
	char line_buffer[255];
	char* line_ptr;
	char* context = NULL;
	int student_index = 0;

	int field_counter = 0;
	char* fields[10];


	while (fgets(line_buffer, sizeof(line_buffer), file)) {
		//printf("Line: %s", line_buffer);
		line_counter++;
		printf("Line number %d\n", line_counter);
		if (line_counter <= 5) {
			continue;
		}

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

		sscanf_s(line_buffer, "%d\t%[a-zA-Z ]\t%[a-zA-Z ]\t%f", &StudentRecord[student_index].id,
								StudentRecord[student_index].name, sizeof(StudentRecord[student_index].name),
								StudentRecord[student_index].programme, sizeof(StudentRecord[student_index].programme),
								&StudentRecord[student_index].mark);


		printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
			student_index,
			StudentRecord[student_index].id,
			StudentRecord[student_index].name,
			StudentRecord[student_index].programme,
			StudentRecord[student_index].mark);
		student_index++;
	}

	return StudentRecord;
}