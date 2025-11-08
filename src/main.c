#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "commands.h"
#include "utils.h"
#include "data.h"


int main() {

	char command[50];

	printf("Booting up . . .\n");


	while (1) {
		//printf("What would you like to do?: ");
		printf("P2_7: ");
		fgets(command, sizeof(command), stdin);

		// removes leading, trailing whitespaces (incl. \n given by fgets), makes everything uppercase
		clean_input(command);

		// if user inputs nothing/only whitespace, ask again
		if (command[0] == '\0') {
			printf("Received empty command.\n");
			continue;
		}

		// end program if asked (first 4 chars = exit, case insentive)
		if (_strnicmp(command, "EXIT", 4) == 0) {
			break;
		}



		// tries to find the correct function to run based on user input, run_command returns true if worked, false if failed
		/*if (!run_command(command)) {
			printf("Command given not recognised. Please try again.\n");
		}*/

		run_command(command);

		/*printf("Finished %s command\n", command);*/
	}

	printf("Closing program ... bye!");
	return 0;
}