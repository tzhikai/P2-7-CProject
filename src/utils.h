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


#endif // !INPUT_H
