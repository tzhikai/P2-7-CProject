#include <stdio.h>

// example open function
void open_fn() {
	printf("Open sesame!!!\n");
}
// example show all function
void showall_fn() {
	printf("Pretend im listing stuff!!!\n");
};

// structure for the operation function
struct operation {
	char name[20];			// user input to call the fn
	void (*function)();
};

// array of available commands (all new ones go in here)
struct operation operations[] = {
	{"open", open_fn},
	{"show all", showall_fn}
};

int main() {
	
	char command[20];

	printf("Booting up . . .\n");

	while (1) {
		printf("What would you like to do?: ");
		scanf_s("%s", command, (unsigned)sizeof(command)); // need to cast to unsigned because scanf_s expects it from the sizeof

		printf("Running %s command\n", command);
		
		if (command == 'exit') {
			break;
		}



		printf("Finished %s command\n", command);
	}

	printf("Closing program ... bye!");
	return 0;
}