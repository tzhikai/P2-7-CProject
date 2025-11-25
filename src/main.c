#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <crtdbg.h>

#include "commands.h"
#include "utils.h"
#include "data.h"

static void readDisclaimer() {
	FILE* file = fopen("src\\DISCLAIMER.txt", "r");
	if (file == NULL) {
		printf("The Disclaimer is missing.\nGroup P2_7 comprising of members:\nTan Zhi Kai\nLee Hong Yih\nKim Beom Su\nJaison Tan\n\nswears that we did not cheat or plagiarise others' works in the making of this program.\n");
		return;
	}

	char line_buffer[256];

	while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
		printf("%s", line_buffer);
	}

	fclose(file);
}

int main() {
	
	char command[100];

	printf("Booting up . . .\n");
	readDisclaimer();

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	create_undostack();

	while (1) {
		

		/*_CrtSetBreakAlloc(94);*/

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
	_CrtDumpMemoryLeaks();
	return 0;
}