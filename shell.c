#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_HISTORY_SIZE 100
#define MAX_SIZE 50
/*
 * Author: Khang Chau Vo
 * CS 475 HW1: Shell
 * http://www.jonbell.net/gmu-cs-475-spring-2018/homework-1/
 */
void execute_input(char**,int*,char**,int*);
char** parse_input(char*);
char* read_input();
void write_history(char**, char**,int*);
void clean_history(char**,int);
void change_directory(char**);
void print_history(char**,int);
int check_history_number(char**);
void free_parse(char**);
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
	 
	char* history_data[MAX_HISTORY_SIZE];
	memset(history_data,0,MAX_HISTORY_SIZE);
	int hist_index = 0;
	/* store the user input */
	char* input;
	int exit = 0; // will only return 1 when exit is entered or EOF
	char** parsed_args;
	
	while(!exit){
		printf("475$");
		fflush(stdout);
		input = read_input();
		fflush(stdout);
		// if an EOF is read from input, break out for clean up
		if(isEOF){
			break;
		}
		write_history(history_data,&input,&hist_index);
		parsed_args = parse_input(input);		
		execute_input(parsed_args,&exit,history_data,&hist_index);
		// clean up after finish
		free(input);
		free(parsed_args);
	}
    clean_history(history_data,hist_index);
}
/* Read the user input and resize the array as needed then return the input*/
char* read_input(){
	int buffer_size = MAX_SIZE;
	char* user_input = calloc(buffer_size, sizeof(char));	
	if(!user_input){
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
	}		
	int index = 0;
	char c = getchar();
	while(c != '\n'){
		if(c == EOF){
			isEOF = 1;
			return user_input;
		}
		if(index > buffer_size-1){
			buffer_size *= 2;
			user_input = realloc(user_input,buffer_size);	
			if(!user_input){
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(1);
			}
		}
		user_input[index++] = c;
		c = getchar();
	}
	// null terminate the string
	user_input[index] = '\0';
	return user_input;	
}

/* parse the user input and return the parsed array */
char** parse_input(char* input){
	int input_length = strlen(input);
	char* user_input = calloc(input_length+1,sizeof(char));
	if(!user_input){
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
	}
	strcpy(user_input,input);
	// max number of arguments allow + command (128 + 1 + 1 (for NULL) = 130)
	int max_len = 130;
	int index = 0;
	char** parsed = calloc(max_len, sizeof(char*));
	if(!parsed){
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(1);
	}
	char* token = strtok(user_input," ");
	while(token != NULL){
		// any attempt to store would be exceeding the limits (128)
		if(index == max_len-2){
			fprintf(stderr, "error: %s\n", "too many arguments");
			free_parse(parsed);
			// reinitialize 
			parsed = calloc(max_len,sizeof(char*));
			if(!parsed){
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(1);
			}
			return parsed;
		}
		int token_length = strlen(token);
		parsed[index] = calloc(token_length+1,sizeof(char));
		strcpy(parsed[index],token);
		index++;
		token = strtok(NULL," ");	
	}
	// fill in that null for exec
	parsed[index] = NULL;
	free(user_input);
	return parsed;	
}
/* Pseudocode:
 * parse first argument;
 * if(first argument == exit or cd or history){
	 call appropriate function
     return;
 }
 * int pid;
 * pid = fork()
 * if(pid < 0)
 *	 print error message
 * else if (pid == child){
 *	 call execvp on first arg and rest of arg
 *  else // implies that pid is parent
 *		wait(&status) // wait for child to finish
 */ 
	 
void execute_input(char** args, int* exit_flag, char** history, int* hist_index){
	if(args[0] == NULL)
		return;
	int status;
	char* special_command[3];
	// list of special command
	special_command[0] = "exit";
	special_command[1] = "cd";
	special_command[2] = "history";	
	// if exit
	if(strcmp(args[0],special_command[0]) == 0){
		*exit_flag = 1;
		return;
	}
	// change directory
	else if(strcmp(args[0],special_command[1]) == 0){
		change_directory(args);
		return;
	}
	// history
	else if(strcmp(args[0],special_command[2]) == 0){
		// only history
		if(args[1] == NULL){
			print_history(history,*hist_index);
			return;
		}
		// clear
		else if(strcmp(args[1], "-c") == 0){
			clean_history(history,100);
			*hist_index = 0;
			return;
		}
		else{
			int result = check_history_number(args);
			if(result != -1){
				char ** history_parsed_args = parse_input(history[result]);
				write_history(history,&history[result],hist_index);
				execute_input(history_parsed_args,exit_flag,history,hist_index);
				return;
			}
		}
	}
		
	int pid = fork();
	if(pid < 0){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	else if (pid == 0){
		// do non-built in
		if(execvp(args[0],args) < 0){
			fprintf(stderr, "error: %s\n", strerror(errno));
			exit(1);
		}
	}	
	else{
		wait(&status);
	}
	return;
}
/* Write to history and return the location where history array points to in memory
 * if(command was blank line)
	 return;
 * if(command was history){
	 check the second arg
	 if(second arg is a number between 0-99){
		 write to history from index 2 to end
	 }
 }
 * if (index that we are trying to write to equals to or exceed 100 cap){
	 free the 0th index of history
	 for(i = 0; i < 99; i++)
		 history[i] = history[i+1];
	 reset index to last index
   }
   else{
	calloc the history index that we are writing to
   }
   write to that index
   index ++
   return;*/
void write_history(char** hist, char** args,int* latest_index){
	int temp_len = strlen(*args);
	char* temp = malloc(temp_len+1);
	if(!temp)
		fprintf(stderr, "error: %s\n", strerror(errno));
	strcpy(temp,*args);
	// if a newline is entered
	if(*args == NULL)
		return;
	// if the command was history, do not write
	if(strcmp(*args,"") == 0)
		return;
	if(strcmp(*args,"history") == 0)
		return;
	// if contains history
	if(strstr(*args,"history"))
		return;
	int index = *latest_index;
	int i;
	// if we are exceeding cap, then shift everything up by 1
	if(index == MAX_HISTORY_SIZE){
		clean_history(hist,1);
		for(i = 0; i < MAX_HISTORY_SIZE-1; i++)
			hist[i] = hist[i+1];
		index = MAX_HISTORY_SIZE-1;
	}
	/* might scrap*/
	int len = strlen(*args);
	hist[index] = calloc(len+1,sizeof(char));
	if(!hist[index]){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	strcpy(hist[index],temp);
	hist[index][len] = '\0';
	// write to the index
	//hist[index] = *args;
	// update the new index
	free(temp);
	index++;
	*latest_index = index;
	return;	
}

/* Free up the string in the history array
 * for every array of string in history
 *    for every string in array of string
          free(string)
	  free(array of string)
 * return
 */
void clean_history(char** history,int length){
	int i;
	for(i = 0; i < length; i++){
		if(history[i] == NULL)
			break;
		free(history[i]);
	}
	return;
}

/* Change directory command*/
void change_directory(char** args){
	if(args[1] == NULL){
		fprintf(stderr, "error: %s\n", "usage: cd [dir]!");
		return;
	}
	if(chdir(args[1]) != 0){
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	return;
}

/* Execute any command relates to history
 * if(args[1] is empty)
	 print history
   else if(args[1] is clear)
	 free(history)
   else if(args[1] is a number)
	   check if the number is between 0 and 100 (exclusive)
		if the number is between 0 and 100 (exclusive){
			write to history
			call to execute with the args
		}
		else{
			do nothing
		}
 */		
void print_history(char** history,int length){
	char ** hist_mover = history;
	int i = 0;
	for(i = 0; i < length; i++){
		printf("%d: %s\n",i,hist_mover[i]);
	}
		
}
/* if args[0] is history command{
	if(arg[1] is string 0)
		return 0;
	else
		t = (int)arg[1];
		if(t is between 1 and 9){
			if(arg[2] == NULL)
				return t;
		}
		else{
			p = (int) arg[2]
			if(p is between 0 and 9)
				return t*10 + p;
		}
		
	}
 * }
*/
int check_history_number(char** args){
	int t,p;
	if(strcmp(args[0],"history") == 0){
		if(args[1][0] == '0'){
			return 0;
		}
		else{
			t = (int)args[1][0] - 48;
			if(t >= 1 && t <= 9){
				p = (int)args[1][1]-48;
				if(p >= 0 && p <= 9)
					return t*10+p;
				else
					return t;
			}
			
		}
	}
	return -1;
}

void free_parse(char** parsed_args){
	while(*parsed_args){
		parsed_args++;
		free(*parsed_args);
	}
	free(parsed_args);
		
}






