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

bool run_command(char command[]);

// hy test functions
bool save_fn(char* context);
bool summary_fn(char* context);
void saveDatabase(const char* filename);
void showSummary();


// external references
extern struct operation operations[];


#endif // !COMMANDS_H



