#ifndef DATA_H



struct Student {
	int id;					// eg 2500123
	char name[50];			// eg Fake Name
	char programme[50];		// eg Computer Science
	float mark;				// eg 82.1
};


struct Student* load_data(FILE *file);


void set_database(struct Student* db);
struct Student* get_database();
int studentcount();

#endif // !DATA_H