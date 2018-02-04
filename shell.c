#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/*
 * CS 475 HW1: Shell
 * http://www.jonbell.net/gmu-cs-475-spring-2018/homework-1/
 */
int execute_input(char**);
char** parse_input(char*);
char* read_input();
extern int errno;

// testing methods
int debug = 1;
void test_print(char** args);

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
		parsed_args = parse_input(input);
		//exit = execute_input(parsed_args);
		
		// should do some clean up after we are done with it
		free(input);
		free(parsed_args);
	}
}

char* read_input(){
	char* user_input = NULL;
	size_t buf_len = 0;
	// let getline does the allocation
	if(!getline(&user_input,&buf_len,stdin)){
		if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
		else fprintf(stderr, "error: %s\n", "Failed allocation or unable to read any bytes!");
	}
	return user_input;	
}

char** parse_input(char* input){
	// max number of arguments allow + command (127 + 1 = 128)
	int max_len = 128;
	int index = 0;
	char** parsed = malloc(max_len * sizeof(char*));
	if(!parsed){
		if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
		else fprintf(stderr, "error: %s\n", "Failed malloc allocation!");
	}
	// space is the delimiter
	const char* delim = " ";
	char* token = strtok(input,delim);	
	while(token != NULL){
		// any attempt to store would be exceeding the limits
		if(index == max_len-1)
			fprintf(stderr, "error: %s\n", "Too many arguments!");
		// store the token
		parsed[index++] = token;
		token = strtok(NULL,delim);		
	}
	// shrink the space alloted if #args is less
	if(index < max_len){
		parsed = realloc(parsed,index);
		if(!parsed){
			if(errno) fprintf(stderr, "error: %s\n", strerror(errno));
			else fprintf(stderr, "error: %s\n", "Failed malloc allocation!");
		}		
	}
	return parsed;	
}

// only for testing!
void test_print(char** args){
	char* temp = *args;
	while(*temp != '\0'){
		printf("%c",(*temp++));	
		fflush(stdout);
	}
}
