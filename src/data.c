#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"



void load_data(FILE *file) {

	int initial_size = sizeof(struct Student) * 5;	// allow space for 5 Student struct array members
	
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
		field_counter = 0;
		context = NULL;

		for (line_ptr = strtok_s(line_buffer, "\t", &context);	// assign line_ptr for loop condition and for use in loop
			line_ptr;											// check if line_ptr is NULL -> means nothing else in line_buffer
			line_ptr = strtok_s(NULL, "\t", &context))			// moves line_ptr to next word
		{
			printf("Line ptr: %s\n", line_ptr);
			fields[field_counter] = line_ptr;
			field_counter++;
		}

		StudentRecord[student_index].id = atoi(fields[0]);
		strcpy_s(StudentRecord[student_index].name, sizeof(StudentRecord[student_index].name), fields[1]);
		strcpy_s(StudentRecord[student_index].programme, sizeof(StudentRecord[student_index].programme), fields[2]);
		StudentRecord[student_index].mark = atof(fields[3]);

		student_index++;
	}

	printf("StudentRecord 1: %d\n", StudentRecord[0].id);
	printf("StudentRecord 1: %s\n", StudentRecord[0].name);
	printf("StudentRecord 1: %s\n", StudentRecord[0].programme);
	printf("StudentRecord 1: %f\n", StudentRecord[0].mark);

	printf("StudentRecord 1: %d\n", StudentRecord[1].id);
	printf("StudentRecord 1: %s\n", StudentRecord[1].name);
	printf("StudentRecord 1: %s\n", StudentRecord[1].programme);
	printf("StudentRecord 1: %f\n", StudentRecord[1].mark);

	printf("StudentRecord 1: %d\n", StudentRecord[2].id);
	printf("StudentRecord 1: %s\n", StudentRecord[2].name);
	printf("StudentRecord 1: %s\n", StudentRecord[2].programme);
	printf("StudentRecord 1: %f\n", StudentRecord[2].mark);
	return;
}