#ifndef COMMANDS_H		
#define COMMANDS_H

#include <stdbool.h>

typedef enum { CMD_INSERT, CMD_UPDATE, CMD_DELETE , CMD_OPEN} CmdAction;

// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	int wordCount;			// temp, will likely replace with dynamically calculating it
	bool (*function)(char* context);
};

// hy test functions
bool summary_fn(char* context);
void update_width(struct Database* db, int row_idx, CmdAction action);

// function prototypes
bool open_fn(char* filename);	// context in this means filename 
bool showall_fn(char* context);
bool delete_fn(char* context);
bool sort_fn(char* context);
bool undo_fn(char* context);
bool update_fn(char* context);
bool insert_fn(char* context);
bool save_fn(char* context);
bool summary_fn(char* context);
bool query_fn(char* context);
bool newfile_fn(char* context);

bool run_command(char command[]);


#endif // !COMMANDS_H



