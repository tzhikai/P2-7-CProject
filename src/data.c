#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

// static variable to store the student records data
static struct Database* StudentDB = NULL;

// function to call when making changes to 
void set_database(struct Database* db) {
    StudentDB = db;
}
// allow retrieval of StudentDB within other functions in the other files
struct Database* get_database() {
    return StudentDB;
}

struct Database* load_data(FILE* file) {
    int capacity = 4;
    int memory = sizeof(struct Student) * capacity; // allow space for 4 Student struct array members

    struct Database* StudentDB = malloc(sizeof(struct Database));

    if (StudentDB == NULL) {
        printf("Memory allocation for StudentDB failed\n");
        return NULL;
    }

    // allocate memory for initial members of StudentRecord 
    StudentDB->StudentRecord = malloc(memory);
    if (StudentDB->StudentRecord == NULL) {
        printf("Memory allocation for StudentRecord failed.\n");
        free(StudentDB);
        return NULL;
    }

    int line_counter = 0;
    char line_buffer[255];
    int student_index = 0;

    struct Student* record = StudentDB->StudentRecord; // shortcut to type less

    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        line_counter++;

        // skip headers (first two lines) and blank lines
        if (line_counter <= 2 || strlen(line_buffer) < 3)
            continue;

        // reallocate memory if needed
        if (student_index >= capacity) {
            capacity *= 2;
            struct Student* temp = realloc(record, sizeof(struct Student) * capacity);
            if (temp == NULL) {
                printf("Memory reallocation failed.\n");
                free(record);
                free(StudentDB);
                return NULL;
            }
            record = temp;
            StudentDB->StudentRecord = record;
        }

        // flexible parsing for tab OR space separated files
        int result = sscanf_s(line_buffer, "%d\t%49[^\t]\t%49[^\t]\t%f",
            &record[student_index].id,
            record[student_index].name, (unsigned int)sizeof(record[student_index].name),
            record[student_index].programme, (unsigned int)sizeof(record[student_index].programme),
            &record[student_index].mark);

        if (result != 4) {
            result = sscanf_s(line_buffer, "%d %49[^\t] %49[^\t] %f",
                &record[student_index].id,
                record[student_index].name, (unsigned int)sizeof(record[student_index].name),
                record[student_index].programme, (unsigned int)sizeof(record[student_index].programme),
                &record[student_index].mark);
        }

        if (result == 4) {
            printf("Successfully read student %d: ID=%d, Name=%s, Programme=%s, Mark=%.1f\n",
                student_index,
                record[student_index].id,
                record[student_index].name,
                record[student_index].programme,
                record[student_index].mark);
            student_index++;
        }
        else {
            printf("Skipping unreadable line %d: %s\n", line_counter, line_buffer);
        }
    }

    StudentDB->memory = memory;
    StudentDB->size = student_index;
    printf("Total records loaded: %d\n", student_index);
    return StudentDB;
}

// Insert/add a new student
void add_student(struct Database* db, struct Student newStudent) {
    if (db == NULL) {
        printf("Database not loaded.\n");
        return;
    }

    struct Student* temp = realloc(db->StudentRecord, sizeof(struct Student) * (db->size + 1));
    if (temp == NULL) {
        printf("Memory reallocation failed during insert.\n");
        return;
    }

    db->StudentRecord = temp;
    db->StudentRecord[db->size] = newStudent;
    db->size++;

    
    set_database(db);

    printf("CMS: A new record with ID=%d is successfully inserted.\n", newStudent.id);

    // auto-save to file 
    char filepath[250] = "src\\CMS.txt";
    save_database(db, filepath);
}

// Find student by ID
struct Student* find_student_by_id(struct Database* db, int id) {
    if (db == NULL || db->StudentRecord == NULL) {
        return NULL;
    }

    for (int i = 0; i < db->size; i++) {
        if (db->StudentRecord[i].id == id) {
            return &db->StudentRecord[i];
        }
    }
    return NULL;
}

// Save database back to file
void save_database(struct Database* db, const char* filepath) {
    if (db == NULL || db->StudentRecord == NULL) {
        printf("No data to save.\n");
        return;
    }

    FILE* file_ptr;
    fopen_s(&file_ptr, filepath, "w");
    if (file_ptr == NULL) {
        printf("Failed to open %s for saving.\n", filepath);
        return;
    }

    fprintf(file_ptr, "StudentRecords\n");
    fprintf(file_ptr, "ID\tName\tProgramme\tMark\n");
    for (int i = 0; i < db->size; i++) {
        fprintf(file_ptr, "%d\t%s\t%s\t%.1f\n",
            db->StudentRecord[i].id,
            db->StudentRecord[i].name,
            db->StudentRecord[i].programme,
            db->StudentRecord[i].mark);
    }

    fclose(file_ptr);
    printf("CMS: The database file is successfully saved to %s\n", filepath);
}
