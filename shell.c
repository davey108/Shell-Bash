#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUFLEN 1024
/*
 * CS 475 HW1: Shell
 * http://www.jonbell.net/gmu-cs-475-spring-2018/homework-1/
 */
int execute_input(char* );
char* parsed_args(char* );
char* read_input();
extern int errno;

int main(int argc, char **argv) {
	/* No error yet */
	errno = 0;
	// divide this into 3 parts:
	// read in input
	// parse the input (break it up by delim)
	// execute the input
	
	/* store the history data*/
	char* history_data;
	/* store the user input */
	char* input;
	int exit = 0; // will only return 1 when exit is entered or EOF
	
	while(!exit){
		printf("475$");
		fflush(stdout);
		input = read_input();
		parsed_args = parse_input(input);
		exit = execute_input(parsed_args);		
	}
	// leaving while loop meaning exit/EOF so clean up
	free(input);
	free(parsed_args);
}

char* read_input(){
	int buf_len = BUFLEN;
	char* user_input = malloc(buf_len * sizeof(char));
	if(!user_input){
		if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
		else fprintf(stderr, "error: %s\n", "Malloc cannot allocate space!");
		exit(EXIT_FAILURE);
	}
	
	
}
