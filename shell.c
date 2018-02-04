#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define MAX_SIZE 50
/*
 * CS 475 HW1: Shell
 * http://www.jonbell.net/gmu-cs-475-spring-2018/homework-1/
 */
int execute_input(char**);
char** parse_input(char*);
char* read_input();
extern int errno;

int isEOF = 0;

int debug = 1;

int main(int argc, char **argv) {
	/* No error yet */
	errno = 0;
	// divide this into 3 parts:
	// read in input
	// parse the input (break it up by delim)
	// execute the input
	
	/* store the history data*/
	//char* history_data;
	/* store the user input */
	char* input;
	int exit = 0; // will only return 1 when exit is entered or EOF
	char** parsed_args;
	
	while(!exit){
		printf("475$");
		fflush(stdout);
		input = read_input();
		// if an EOF is read from input, break out for clean up
		if(isEOF) break;
		parsed_args = parse_input(input);
		//exit = execute_input(parsed_args);
	}
	// clean up after finish
	free(input);
	free(parsed_args);
}

char* read_input(){
	int buffer_size = MAX_SIZE;
	char* user_input = calloc(buffer_size, sizeof(char));	
	if(!user_input){
		if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
		else fprintf(stderr, "error: %s\n", "Failed malloc allocation!");
	}
	int index = 0;
	char c;
	do{
		c = getchar();
		// if EOF (ctrl + d) detected, return buffer for clean up
		if(c == EOF){
			isEOF = 1;
			return user_input;
		}
		// if exceed caps, increase limit
		if(index > buffer_size-1){
			buffer_size *= 2;
			user_input = realloc(user_input,buffer_size);			
		}
		user_input[index++] = c;
	}while(c != '\n');
	// null terminate the string
	user_input[index] = '\0';
	return user_input;	
}

char** parse_input(char* input){
	// max number of arguments allow + command (127 + 1 = 128)
	int max_len = 128;
	int index = 0;
	char** parsed = calloc(max_len, sizeof(char*));
	if(!parsed){
		if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
		else fprintf(stderr, "error: %s\n", "Failed malloc allocation!");
	}
	// space is the delimiter
	const char delim[2] = " ";
	char* token = strtok(input,delim);	
	while(token != NULL){
		// any attempt to store would be exceeding the limits
		if(index == max_len-1)
			fprintf(stderr, "error: %s\n", "Too many arguments!");
		// store the token
		parsed[index++] = token;
		token = strtok(NULL,delim);		
	}
	return parsed;	
}

