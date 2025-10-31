#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "commands.h"
#include "utils.h"


int main() {
	
	char command[20];

	printf("Booting up . . .\n");

	while (1) {
		printf("What would you like to do?: ");
		fgets(command, sizeof(command), stdin);

		// removes leading, trailing whitespaces (incl. \n given by fgets), makes everything uppercase
		clean_input(command);
		
		// if user inputs nothing/only whitespace, ask again
		if (command[0] == '\0') {
			printf("Received empty command.\n");
			continue;
		}

		// end program if asked
		if (strcmp(command, "EXIT") == 0) {
			break;
		}

		run_command(command);

		/*printf("Finished %s command\n", command);*/
	}

	printf("Closing program ... bye!");
	return 0;
}