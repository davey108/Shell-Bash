#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SIZE 50
#define HISTORY_SIZE 1024
/*
 * CS 475 HW1: Shell
 * http://www.jonbell.net/gmu-cs-475-spring-2018/homework-1/
 */
void execute_input(char**,int*);
char** parse_input(char*);
char* read_input();
char*** write_history(char***, char**,int*, int*);
void clean_history(char***,int);
extern int errno;

int isEOF;
int debug = 1;

int main(int argc, char **argv) {
	/* No error yet */
	errno = 0;
	/* No ctrl + d yet*/
	isEOF = 0;
	// divide this into 3 parts:
	// read in input
	// parse the input (break it up by delim)
	// execute the input
	
	/* store the history data: array of arrays of string
	 * history_data: [ each element | points to | an array of strings |....]
	 *                                    v  -> each element points its own arr_strings
	 * arr_strings:  [ each element | points to | a string (char array) |....]
	 *                                    v -> each element points to its own arr_char
	 * char_array:    [ each | element | is | a | char|...]*/
	 
	/* Initialize history length to its max size which might change*/
	int hist_length = HISTORY_SIZE;
	char*** history_data = calloc(hist_length, sizeof(char**));
	if(!history_data){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	
	int hist_index = 0;
	/* store the user input */
	char* input;
	int exit = 0; // will only return 1 when exit is entered or EOF
	char** parsed_args;
	
	while(!exit){
		printf("475$");
		fflush(stdout);
		input = read_input();
		// if an EOF is read from input, break out for clean up
		if(isEOF){
			break;
		}
		parsed_args = parse_input(input);
		history_data = write_history(history_data,parsed_args,&hist_index,&hist_length);
		execute_input(parsed_args,&exit);
		// clean up after finish w/ each input (space for next one)
		free(input);
		free(parsed_args);
	}
    clean_history(history_data,hist_index);	
}

char* read_input(){
	int buffer_size = MAX_SIZE;
	char* user_input = calloc(buffer_size, sizeof(char));	
	if(!user_input){
		fprintf(stderr, "error: %s\n", strerror(errno));
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
			if(!user_input){
				fprintf(stderr, "error: %s\n", strerror(errno));
			}
		}
		user_input[index++] = c;
	}while(c != '\n');
	// null terminate the string
	user_input[index] = '\0';
	return user_input;	
}

char** parse_input(char* input){
	// max number of arguments allow + command (127 + 1 + 1 (for NULL) = 129)
	int max_len = 129;
	int index = 0;
	char** parsed = calloc(max_len, sizeof(char*));
	if(!parsed){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	char* token = strtok(input," ");	
	while(token != NULL){
		// any attempt to store would be exceeding the limits (128)
		if(index == max_len-2)
			fprintf(stderr, "error: %s\n", "Too many arguments!");
		// store the token
		parsed[index++] = token;
		token = strtok(NULL," ");		
	}
	// fill in that null for exec
	parsed[index] = NULL;
	return parsed;	
}
/* Pseudocode:
 * parse first argument;
 * int pid;
 * pid = fork()
 * if(pid > 0)
 *	 print error message
 * else if (pid == child){
 * 	 if(first argument != exit and cd and history)
 *		call execvp on first arg and rest of arg
 *	 else{
 *		scan first arg
 *		call appropriate function on first arg
 *	 }
 *  else // implies that pid is parent
 *		wait(&status) // wait for child to finish
 */ 
	 
void execute_input(char** args, int* exit_flag){
	int status;
	char* special_command[3];
	// list of special command
	special_command[0] = "exit";
	special_command[1] = "cd";
	special_command[2] = "history";
	int pid = fork();
	if(pid > 0){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	else if (pid == 0){
		// do non-built in
		if(strcmp(args[0],special_command[0])!= 0 && strcmp(args[0],special_command[1])!= 0 && strcmp(args[0],special_command[2])!= 0)
			execvp(args[0],args);
		// do built in (exit, cd, history)
		else{
			// if exit, returns 0
			if(strcmp(args[0],special_command[0]) == 0)
				*exit_flag = 1;
			
		}
	}
	else{
		wait(&status);
	}	
}
/* Write to history and return the location where history array points to in memory*/
char*** write_history(char*** hist, char** args,int* latest_index, int* history_length){
	char*** history = hist;
	int index = *latest_index;
	int hist_len = *history_length;
	// if the index is exceeding bound, expand the array
	if(*latest_index == *history_length){
		// increase size cap for length
		*history_length = hist_len*2;
		history = realloc(history,*history_length);
	}
	int i;
	// allocate to max length of allowed inputs
	// and allocate 1 extra for NULL later (for execute)
	history[index] = calloc(129,sizeof(char**));
	if(!history[index]){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	for(i = 0; i < 128; i++){
		int string_len = strlen(args[i]);
		// +1 for null terminator
		history[index][i] = calloc(string_len+1,sizeof(char));
		if(!history[index][i]){
			fprintf(stderr, "error: %s\n", strerror(errno));
		}
		strcpy(history[index][i],args[i]);
	}
	// fill in the null for later exec
	history[index][128] = NULL;
	// update the new index
	index++;
	*latest_index = index;
	return history;	
}

/* Free up the history array
 * for every array of string in history
 *    for every string in array of string
          free(string)
	  free(array of string)
 * return
 */
void clean_history(char*** history,int length){
	int i,j;
	int string_array_len = 128;
	for(i = 0; i < length; i++){
		for(j = 0; j < string_array_len; j++)
			free(history[i][j]);
		free(history[i]);
	}
	free(history);
	return;
}




