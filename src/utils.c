#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "data.h"
#include "commands.h"


static struct UndoStack UndoCmdsStatic;
static struct UndoStack* UndoCmds = NULL;

// function to call when making changes to 
void set_undostack(struct UndoStack* undos) {
	UndoCmds = undos;
}
// allow retrieval
struct UndoStack* get_undostack() {
	return UndoCmds;
}

// cleans up user input command before checking if it can be executed
void clean_input(char command[]) {

	if (command == NULL || command[0] == '\0') {
		return;
	}

	// printf("Before: command: %s, size: %d\n", command, strlen(command));

	// loop for trailing whitespaces
	// create a pointer to the end of the command string, move to remove whitespaces
	char* last_char = command + strlen(command) - 1;	// command = memory addr of first letter, -1 to skip null operator
	while (last_char >= command && isspace(*last_char)) {
		last_char--;
	}
	*(last_char + 1) = '\0';	// last_char is last nonwhitespace char, insert null operator after

	// loop for leading whitespaces
	char* first_char = command;
	while (isspace(*first_char)) {	// loop will stop at null operator even if command is all whitespaces
		first_char++;
	}	// moves memory block starting from first non-whitespace char to command
	if (first_char != command) {
		memmove(command, first_char, strlen(first_char) + 1);	// +1 to include \0
	}

	// printf("After: command: %s, size: %d\n", command, strlen(command));

	return;
}

// removes spaces and _s within (& in front of & back of) a string (to join words tgt for comparing similar words)
void join_words(char input[]) {

	char* read_ptr = input;
	char* write_ptr = input;

	// read_ptr pointer goes thru input's memory addr
	while (*read_ptr != '\0') {

		// this rewrites write_ptr's value with read_ptr's value, but only when not _ or space
		if ((*read_ptr != '_') && (*read_ptr != ' ')) {
			*write_ptr++ = *read_ptr;
		}

		//write_ptr++;
		read_ptr++;
	}
	// if a char was overwritten, the string needs to end early
	*write_ptr = '\0';

}

// loops thru ColumnMap within StudentDB to find the index within column array from col_id like COL_MARK
int get_column(Columns col_id) {
	struct Database* StudentDB = get_database();

	if (StudentDB == NULL || StudentDB->columns == NULL) {
		return -1;
	}

	struct ColumnMap *cols = StudentDB->columns;

	for (int col_index = 0; col_index < StudentDB->column_count; col_index++) {
		if (cols[col_index].column_id == col_id) {
			return col_index;
		}
	}

	return -1;
}

void print_error(CmdAction cmd, int row_number, char error[], Columns col_type) {
	if (cmd == CMD_OPEN && row_number != -1) {
		printf("Row %d, ", row_number);
	}

	printf("%s.", error);

	if (cmd == CMD_OPEN) {
		if (col_type == COL_ID) {	// primary key got no excuse
			printf(" Skipping row.");
		}
		else if (col_type == COL_MARK) {
			printf(" Replaced with \"-1.0\".");
		}
		else {			// non primary key can just tolerate
			printf(" Replaced with \"N/A\".");
		}	
	}
	else {	// not open_fn
		printf(" Please try again.");
	}
	printf("\n");
}

int back_column(struct Database* StudentDB, int col_index) {
	for (int i = col_index - 1; i >= 0; i--) {
		if (StudentDB->columns[i].column_id != COL_ID && StudentDB->columns[i].column_id != COL_OTHER) {
			return i;
		}
	}
	return -1;
}

int extract_extrainput_id(int* id_ptr, char* extrainput, struct Database* StudentDB, struct HeaderValuePair* hvp_array, CmdAction cmd) {
	if (extrainput == NULL || extrainput[0] == '\0') {
		printf("Extra input is NULL or empty.\n");
		return 0;
	}
	
	char* cmd_ptr = NULL;
	char* context = NULL;

	if (extrainput != NULL && extrainput[0] != '\0') {
		char input_copy[100];
		
		errno_t result = strncpy_s(input_copy, sizeof(input_copy), extrainput, _TRUNCATE);



		//printf("context first: %s\n", input_copy);
		// correct would be UPDATE ID=<value>, this could mean UPDATE 2500123
		if (strchr(input_copy, '=') == NULL) {
			printf("Extra input invalid. Use <field>=<value>\n");
			return 0;
		}
		
		cmd_ptr = strtok_s(input_copy, "=", &context); // get first header (before =)

		clean_input(cmd_ptr);

		// check if first word/field is ID, since ID is primary key
		if (map_column(cmd_ptr) != COL_ID) {	// eg UPDATE NAME=John Souls	
			printf("Extra input invalid. First field must be ID.\n");
			return 0;
		}
		else {
			//printf("trial last\n");
			// able to split by string here because it is for ID, which is 7 consecutive digits only
			cmd_ptr = strtok_s(NULL, " ", &context);
			clean_input(cmd_ptr);
			;
			if (cmd_ptr != NULL) {
				switch (validate_id(cmd_ptr, -1, StudentDB, cmd)) {
				case 1:
					printf("Extra input invalid. ID invalid.\n");
					return 0;
				case 0:	// valid id, unused
					if (cmd == CMD_INSERT) {	// is this used for INSERT?
						*id_ptr = atoi(cmd_ptr);
						//printf("ID %s found, now is %d.\n", cmd_ptr, *id_ptr);
						break;
					}
					printf("Extra input invalid. ID not found.\n");
					return 0;
				case 2: // valid id, duplicate
					if (cmd == CMD_UPDATE || cmd == CMD_DELETE) {	// is this used for UPDATE or DELETE
						*id_ptr = atoi(cmd_ptr);
						//printf("ID %s found, now is %d.\n", cmd_ptr, *id_ptr);
						break;
					}
					printf("Extra input invalid. ID already in use.\n");
					return 0;
				}
			}
		}
	}
	else {
		//printf("initial extrainput is NULL\n");
		return 0;
	}

	if (cmd == CMD_DELETE) { // DELETE oneline does not have further fields like Name or Programme to find, can just return after finding id
		return 0;
	}

	if (cmd_ptr == NULL) {
		//printf("post loop is NULL\n");
		return 0;
	}
	

	
	// handle remaining fields, if any

	char* remaining = strstr(extrainput, cmd_ptr);
	if (remaining == NULL) {
		printf("No remaining fields left");
		return 0;
	}
	else {
		remaining += strlen(cmd_ptr);	// move past id
	}
	//printf("Remaning string: %s\n", remaining);

	int hvp_count = 0;
	//struct HeaderValuePair* hvp_array = NULL;
	/*struct HeaderValuePair hvp_array[10];
	memset(hvp_array, 0, sizeof(hvp_array));*/

	// create a list of header-value pairs (like python dictionary!) from the remaining cmd_ptr (if any)
	//const int max_pairs = StudentDB->column_count;	// can limit pair amt to number of cols
	//const int max_pairs = 10;
	//hvp_array = calloc(max_pairs, sizeof(struct HeaderValuePair));	// wont need to realloc since using max possible amt

	
	hvp_count = extract_extrainput_values(hvp_array, remaining, StudentDB, cmd);
	//printf("hvp_count = %d\n", hvp_count);
	return hvp_count;

}

int extract_extrainput_values(struct HeaderValuePair* hvpair, char* extrainput, struct Database* StudentDB, CmdAction cmd) {
	// input eg ID=2500321 Mark=85.5 Name=John Souls Programme=Digital Supply Chain
	// find = to get Mark, then check for = to see if theres another header, if not then rest is value, if so then remove everything after the last space

	if (extrainput == NULL) {
		//printf("no extra input\n");
		return 0;
	}
	else {
		clean_input(extrainput);
		//printf("values input: %s\n", extrainput);
	}

	char input_copy[100];
	strncpy_s(input_copy, sizeof(input_copy), extrainput, _TRUNCATE);

	int pair_count = 0;
	char header[20], value[50];

	char* context = NULL;
	char* extra_ptr = strtok_s(input_copy, " ", &context);	// extracts the first word

	int value_flag = 0, invalid_flag = 0;

	while (extra_ptr != NULL) {
		//printf("handling %s rn\n", extra_ptr);
		char* equal_ptr = strchr(extra_ptr, '=');	// finds = in word, if exists

		if (equal_ptr != NULL) {	// if = means its a header=value pair, if no = means its a continued value
			// means we were previously handling a pair, so now can save this and move on
			//printf("pointer %s has an equals\n", extra_ptr);
			if (value_flag) {
				clean_input(header);
				clean_input(value);
				Columns col_id = map_column(header);
				invalid_flag = 0;
				switch (col_id) {
					case COL_ID:
						if (validate_id(value, -1, StudentDB, cmd) != 2) {	// -1 cuz not from file, so no row number
							//printf("Extra field invalid. ID invalid.\n");
							invalid_flag = 1;
						}
						break;
					case COL_NAME:
						if (validate_name(value, -1, CMD_OPEN)) {
							invalid_flag = 1;
						}
						break;
					case COL_PROGRAMME:
						if (validate_name(value, -1, CMD_OPEN) || validate_programme(value, -1, CMD_OPEN)) {
							invalid_flag = 1;
						}
						break;
					case COL_MARK:
						if (validate_mark(value, -1, CMD_OPEN) < 0) {
							//printf("Extra field invalid. Mark invalid.\n");
							invalid_flag = 1;
						}
						break;
				}

				if (col_id != COL_OTHER && !invalid_flag) {	// if expected header
					hvpair[pair_count].column_id = col_id;
					strncpy_s(hvpair[pair_count].datapoint, sizeof(hvpair[pair_count].datapoint), value, _TRUNCATE);
					//printf("Saving pair of %s = %s\n", header, value);
					pair_count++;
				}
			}

			*equal_ptr = '\0';	// splits string into 2 parts, header and value
			strncpy_s(header, sizeof(header), extra_ptr, _TRUNCATE); // header is before the =
			char* value_ptr = equal_ptr + 1; // value is after the =
			strncpy_s(value, sizeof(value), value_ptr, _TRUNCATE);
			value_flag = 1;	// rmb that we are checking for continued values



		}
		else if (value_flag) {	// means continued value, concat it to end of prev value
			strcat_s(value, sizeof(value), " ");
			clean_input(extra_ptr);
			strcat_s(value, sizeof(value), extra_ptr);
		}

		extra_ptr = strtok_s(NULL, " ", &context);
	}

	// after loop, still got last pair remaining
	if (value_flag) {
		clean_input(value);
		Columns col_id = map_column(header);
		//printf("col_id debug: %d\n", col_id);

		if (hvpair == NULL) {
			printf("hvpair is NULL\n");
		}

		if (col_id != COL_OTHER) {	// if excpected header
			hvpair[pair_count].column_id = col_id;
			strncpy_s(hvpair[pair_count].datapoint, sizeof(hvpair[pair_count].datapoint), value, _TRUNCATE);
			//printf("Saving pair of %s = %s\n", header, value);
			pair_count++;
		}
	}

	

	//zkdebug
	/*for (int i = 0; i < pair_count; i++) {
		printf("Header: %d\n Value: %s\n", hvpair[i].column_id, hvpair[i].datapoint);
	}*/


	return pair_count;

	
}

void create_undostack() {
	//struct UndoStack* undos =  get_undostack();
	if (UndoCmds != NULL) {
		printf("UndoCmds alr exists.\n");
		return;
	}

	/*UndoCmds = calloc(1, sizeof(struct UndoStack));
	if (UndoCmds == NULL) {
		printf("Memory allocation for UndoCmds failed.\n");
		return;
	}
	printf("Memory allocated successfully at address: %p\n", UndoCmds);*/
	
	UndoCmds = &UndoCmdsStatic;
	UndoCmds->oldest = 0;
	UndoCmds->cmd_count = 0;
	UndoCmds->pause_inserts = false;

	set_undostack(UndoCmds);
}

// called at end of insert_fn, delete_fn, update_fn
void insert_undostack(char* command) {
	struct UndoStack* undos = get_undostack();

	if (undos == NULL) {
		//create_undostack();
		printf("UndoStack not created yet or is NULL.\n");
		return;
	}

	if (undos->pause_inserts) {
		//printf("Dont insert undo command into undostack.\n");
		return;
	}

	// checking for empty commands (not likely)
	if (command == NULL || command[0] == '\0') {
		return;
	}
	// if not yet at max capacity, we can just add to empty slots
	if (undos->cmd_count < MAX_UNDOS) {
		strncpy_s(undos->commands[undos->cmd_count], MAX_CMD_LENGTH, command, _TRUNCATE);
		undos->cmd_count++;	//increment as we add (modding this value gives us the position to insert new undos)
	}
	else {	// if full, we override the oldest undo
		strncpy_s(undos->commands[undos->oldest], MAX_CMD_LENGTH, command, _TRUNCATE);
		undos->oldest = (undos->oldest + 1) % MAX_UNDOS;	
		// we use % MAX_UNDOS cuz if 5 is limit, and oldest is 5, then 5+1 is out of range, so we % to bring it to 1
	}
}

bool use_undostack(char* retrieved_command) {
	struct UndoStack* undos = get_undostack();

	if (undos == NULL) {
		//create_undostack();
		printf("UndoStack not created yet or is NULL.\n");
		return false;
	}

	if (undos->cmd_count == 0) {
		printf("UndoStack is currently empty. There are no commands to run.\n");
		return false;
	}

	int undo_index = (undos->oldest + undos->cmd_count - 1) % MAX_UNDOS;
	// edit the retrieved_command ptr so i can use it outside this and not have to run it here
	strncpy_s(retrieved_command, MAX_CMD_LENGTH, undos->commands[undo_index], _TRUNCATE);

	undos->commands[undo_index][0] = '\0';	// we kill the used up command, not keeping it anymore

	undos->cmd_count--;	// since i used one, total is less

	return true;
}

bool preview_undostack(int preview_num) {
	struct UndoStack* undos = get_undostack();
	if (undos == NULL) {
		//create_undostack();
		printf("UndoStack not created yet or is NULL.\n");
		return false;
	}

	if (preview_num < 0 || preview_num >= undos->cmd_count || preview_num >= MAX_UNDOS) {
		//printf("debug: preview_num: %d, undos->cmd_count: %d, MAX_UNDOS: %d\n", preview_num, undos->cmd_count, MAX_UNDOS);
		//printf("No more undos to preview.\n");
		return false;
	}

	int preview_index = (undos->oldest + undos->cmd_count - 1 - preview_num) % MAX_UNDOS;

	printf("UNDO %d: %s\n", preview_num+1, undos->commands[preview_index]);
	
	return true;
}

