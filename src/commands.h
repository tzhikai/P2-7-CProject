#ifndef COMMANDS_H		
#define COMMANDS_H

#include <stdbool.h>

// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	int wordCount;			// temp, will likely replace with dynamically calculating it
	bool (*function)(char* context);
};

// function prototypes
bool open_fn(char* filename);	// context in this means filename 
bool showall_fn(char* context);
struct Database* delete_fn(char* context);
bool sort_fn(char* context);

bool run_command(char command[]);


#endif // !COMMANDS_H



