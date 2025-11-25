#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "data.h"
#include "commands.h"

// function prototypes
void clean_input(char command[]);			// removes leading/trailing whitespaces

void join_words(char input[]);				// removes spaces and _s within (& in front of & back of) a string (to join words tgt for comparing similar words)

struct HeaderValuePair {					// stores a column ID and its corresponding datapoint (value)
	Columns column_id;
	char datapoint[50];
};

int get_column(Columns col_id);														// returns index of column in StudentDB from col_id like COL_MARK
void print_error(CmdAction cmd, int row_number, char error[], Columns col_type);	// prints error messages based on command type
int back_column(struct Database* StudentDB, int col_index);							// gets previous valid column index before col_index

int extract_extrainput_id(						// extracts ID from extrainput string
	int* id_ptr,
	char* extrainput,
	struct Database* StudentDB,
	struct HeaderValuePair* hvp_array,
	CmdAction cmd								// represents INSERT or UPDATE	(true = INSERT, false = UPDATE)
);

int extract_extrainput_values(					// extracts header-value pairs from extrainput string
	struct HeaderValuePair* hvpair,				// array to store extracted header-value pairs
	char* extrainput,							// string containing extra input
	struct Database* StudentDB,					// database pointer
	CmdAction cmd								// represents INSERT or UPDATE
);

#define MAX_UNDOS 5
#define MAX_CMD_LENGTH 255

struct UndoStack {								// circular buffer implementation
	char commands[MAX_UNDOS][MAX_CMD_LENGTH];	// array of undo commands
	int oldest;									// index of the oldest command
	int cmd_count;								// current number of commands in the stack
	bool pause_inserts;							// without this, all the commands run during undo add themselves to the undo stack (and then it undoes the undo)
};

void set_undostack(struct UndoStack* undos);	// sets the global UndoStack pointer
struct UndoStack* get_undostack();				// gets the global UndoStack pointer
void insert_undostack(char* command);			// inserts a command into the UndoStack
bool use_undostack(char* retrieved_command);	// retrieves and removes the most recent command from the UndoStack
bool preview_undostack(int preview_num);		// previews a command from the UndoStack without removing it

#endif // !UTILS_H
