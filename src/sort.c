#include <stdio.h>

#include "data.h"

// Ascending ID Sort
int compidup(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	return StudentA->id - StudentB->id;
}

// Descending ID Sort
int compiddown(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	return StudentB->id - StudentA->id;
}

// Ascending Mark Sort
int compmarkup(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	if (StudentA->mark < StudentB->mark) return -1;
	if (StudentA->mark > StudentB->mark) return 1;
	return 0;
}

// Descending Mark Sort
int compmarkdown(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	if (StudentA->mark < StudentB->mark) return 1;
	if (StudentA->mark > StudentB->mark) return -1;
	return 0;
}