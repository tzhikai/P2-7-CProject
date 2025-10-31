#ifndef COMMANDS_H		




// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	void (*function)();
};

// function prototypes
void open_fn();
void showall_fn();

void run_command(char command[]);


// external references
extern struct operation operations[];


#endif // !COMMANDS_H



