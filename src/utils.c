#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "data.h"

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


int extract_extrainput_id(int* id_ptr, char* extrainput, struct Database* StudentDB, struct HeaderValuePair* hvp_array) {
	if (extrainput == NULL || extrainput[0] == '\0') {
		printf("Extra input is NULL or empty.\n");
		return 0;
	}
	
	char* cmd_ptr = NULL;
	char* context = NULL;

	if (extrainput != NULL && extrainput[0] != '\0') {
		char input_copy[100];
		strcpy_s(input_copy, sizeof(input_copy), extrainput);
		printf("context first: %s\n", input_copy);
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
			printf("trial last\n");
			// able to split by string here because it is for ID, which is 7 consecutive digits only
			cmd_ptr = strtok_s(NULL, " ", &context);
			clean_input(cmd_ptr);
			;
			if (cmd_ptr != NULL) {
				switch (validate_id(cmd_ptr, -1, StudentDB)) {
				case 1:
					printf("Extra input invalid. ID invalid.\n");
					return 0;
				case 0:
					printf("Extra input invalid. ID not found.\n");
					return 0;
				case 2:
					*id_ptr = atoi(cmd_ptr);
					printf("ID %s found, now is %d.\n", cmd_ptr, *id_ptr);
					break;
				}
			}
		}
	}
	else {
		printf("initial extrainput is NULL\n");
		return 0;
	}


	if (cmd_ptr == NULL) {
		printf("post loop is NULL\n");
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
	printf("Remaning string: %s\n", remaining);

	int hvp_count = 0;
	//struct HeaderValuePair* hvp_array = NULL;
	/*struct HeaderValuePair hvp_array[10];
	memset(hvp_array, 0, sizeof(hvp_array));*/

	// create a list of header-value pairs (like python dictionary!) from the remaining cmd_ptr (if any)
	//const int max_pairs = StudentDB->column_count;	// can limit pair amt to number of cols
	//const int max_pairs = 10;
	//hvp_array = calloc(max_pairs, sizeof(struct HeaderValuePair));	// wont need to realloc since using max possible amt

	if (hvp_array == NULL) {
		printf("Memory allocation for hvp_array failed\n");
		return 0;
	}
	else {
		printf("it worked\n");
	}
	if (hvp_array == (void*)-1) {
		printf("calloc returned -1 (error)\n");
	}
	else if ((uintptr_t)hvp_array > 0xFFFFFFFF00000000) {
		printf("Pointer is in kernel space - INVALID!\n");
	}
	else {
		printf("all good\n");
	}

	hvp_count = extract_extrainput_values(hvp_array, remaining, StudentDB);
	printf("hvp_count = %d\n", hvp_count);
	return hvp_count;
	

	/*if (hvp_count > 0) {
		for (int i = 0; i < hvp_count; i++) {
			switch (hvp_array[i].column_id) {
			case COL_ID:
				s->id = atoi(hvp_array[i].datapoint);
				break;
			case COL_NAME:
				strcpy_s(s->name, sizeof(s->name), hvp_array[i].datapoint);
				break;
			case COL_PROGRAMME:
				strcpy_s(s->programme, sizeof(s->programme), hvp_array[i].datapoint);
				break;
			case COL_MARK:
				s->mark = atof(hvp_array[i].datapoint);
				break;
			case COL_OTHER:
				break;
			}
		}*/
		//free(remaining);
	//}

}

int extract_extrainput_values(struct HeaderValuePair* hvpair, char* extrainput, struct Database* StudentDB) {
	// input eg ID=2500321 Mark=85.5 Name=John Souls Programme=Digital Supply Chain
	// find = to get Mark, then check for = to see if theres another header, if not then rest is value, if so then remove everything after the last space

	if (extrainput == NULL) {
		printf("no extra input\n");
		return 0;
	}
	else {
		clean_input(extrainput);
		printf("values input: %s\n", extrainput);
	}

	char input_copy[100];
	strcpy_s(input_copy, sizeof(input_copy), extrainput);

	int pair_count = 0;
	char header[20], value[50];

	char* context = NULL;
	char* extra_ptr = strtok_s(input_copy, " ", &context);	// extracts the first word

	int value_flag = 0, invalid_flag = 0;

	while (extra_ptr != NULL) {
		printf("handling %s rn\n", extra_ptr);
		char* equal_ptr = strchr(extra_ptr, '=');	// finds = in word, if exists

		if (equal_ptr != NULL) {	// if = means its a header=value pair, if no = means its a continued value
			// means we were previously handling a pair, so now can save this and move on
			printf("pointer %s has an equals\n", extra_ptr);
			if (value_flag) {
				clean_input(header);
				clean_input(value);
				Columns col_id = map_column(header);
				invalid_flag = 0;
				switch (col_id) {
					case COL_ID:
						if (validate_id(value, -1, StudentDB) != 2) {	// -1 cuz not from file, so no row number
							//printf("Extra field invalid. ID invalid.\n");
							invalid_flag = 1;
						}
						break;
					case COL_NAME:
						validate_name(value, -1);
						break;
					case COL_PROGRAMME:
						validate_name(value, -1);
						validate_programme(value, -1);
						break;
					case COL_MARK:
						if (validate_mark(value, -1) < 0) {
							//printf("Extra field invalid. Mark invalid.\n");
							invalid_flag = 1;
						}
						break;
				}

				if (col_id != COL_OTHER && !invalid_flag) {	// if excpected header
					hvpair[pair_count].column_id = col_id;
					strcpy_s(hvpair[pair_count].datapoint, sizeof(hvpair[pair_count].datapoint), value);
					printf("Saving pair of %s = %s\n", header, value);
					pair_count++;
				}
			}

			*equal_ptr = '\0';	// splits string into 2 parts, header and value
			strcpy_s(header, sizeof(header), extra_ptr); // header is before the =
			char* value_ptr = equal_ptr + 1; // value is after the =
			strcpy_s(value, sizeof(value), value_ptr);
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
			strcpy_s(hvpair[pair_count].datapoint, sizeof(hvpair[pair_count].datapoint), value);
			printf("Saving pair of %s = %s\n", header, value);
			pair_count++;
		}
	}

	for (int i = 0; i < pair_count; i++) {
		printf("Header: %d\n Value: %s\n", hvpair[i].column_id, hvpair[i].datapoint);
	}

	//set_hvarray(hvpair);

	return pair_count;

	
}

