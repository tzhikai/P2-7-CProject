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

// Ascending Name Sort
int compnameup(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	//return StudentA->name[0] - StudentB->name[0];
	return strcmp(StudentA->name, StudentB->name);	//zkchange: name[0] means Tim and Tom are the same
}

// Descending Name Sort
int compnamedown(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	//return StudentB->name[0] - StudentA->name[0];
	return strcmp(StudentB->name, StudentA->name); //zkchange: name[0] means Tim and Tom are the same
}

// Ascending Programme Sort
int compprogrammeup(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	//return StudentA->name[0] - StudentB->name[0];
	return strcmp(StudentA->programme, StudentB->programme);	//zkchange: name[0] means Tim and Tom are the same
}

// Descending Programme Sort
int compprogrammedown(const void* a, const void* b) {
	const struct Student* StudentA = (const struct Student*)a;
	const struct Student* StudentB = (const struct Student*)b;
	//return StudentB->name[0] - StudentA->name[0];
	return strcmp(StudentB->name, StudentA->programme); //zkchange: name[0] means Tim and Tom are the same
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