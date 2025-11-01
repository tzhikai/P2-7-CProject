#ifndef COMMANDS_H		
#define COMMANDS_H

struct commandSplit {
	char callphrase[50];	// the extracted operation from user input
	char* context;	// points to the curr position in the command string (after extracted part)
	char* copy;		// copy of user input
};


// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	int wordCount;			// temp, will likely replace with dynamically calculating it
	void (*function)();
};

// function prototypes
void open_fn(char** file);	// passed in from commandSplit's context 
void showall_fn();

void run_command(char command[]);


// external references
extern struct operation operations[];


#endif // !COMMANDS_H



