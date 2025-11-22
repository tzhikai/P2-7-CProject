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

#endif // !INPUT_H
