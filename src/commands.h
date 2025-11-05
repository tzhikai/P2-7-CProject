#ifndef COMMANDS_H		
#define COMMANDS_H

#include <stdbool.h>

struct commandSplit {
	char callphrase[50];	// the extracted operation from user input
	char* context;	// points to the curr position in the command string (after extracted part)
	char* copy;		// copy of user input
};


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


// external references
extern struct operation operations[];


#endif // !COMMANDS_H



