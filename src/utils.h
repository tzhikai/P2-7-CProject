#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "data.h"

// function prototypes
void clean_input(char command[]);

void join_words(char input[]);

struct HeaderValuePair {
	Columns column_id;
	char datapoint[50];
};

int get_column(Columns col_id, struct Database* StudentDB);

int extract_extrainput_id(
	int* id_ptr,
	char* extrainput,
	struct Database* StudentDB,
	struct HeaderValuePair* hvp_array,
	bool is_new		// represents INSERT or UPDATE	(true = INSERT, false = UPDATE)
);

int extract_extrainput_values(
	struct HeaderValuePair* hvpair,
	char* extrainput,
	struct Database* StudentDB
);

#define MAX_UNDOS 5
#define MAX_CMD_LENGTH 255

struct UndoStack {
	char commands[MAX_UNDOS][MAX_CMD_LENGTH];
	int oldest;
	int cmd_count;
	bool pause_inserts;	// without this, all the commands run during undo add themselves to the undo stack (and then it undoes the undo)
};

void set_undostack(struct UndoStack* undos);
struct UndoStack* get_undostack();
void insert_undostack(char* command);
bool use_undostack(char* retrieved_command);
bool preview_undostack(int preview_num);

#endif // !UTILS_H
