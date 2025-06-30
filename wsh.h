#ifndef WSH_H
#define WSH_H

#include <dirent.h>
#include <errno.h>	
#include <fcntl.h>	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>	
//Global Defaults
#define ARGS_MAXIMUM 10
#define COMMAND_MAXIMUM 512
#define HISTORY_MAXIMUM 5

//This struct comprises the History List
//It will hold a list of previously used commands
typedef struct History {
	char *commands[HISTORY_MAXIMUM];
	int command_count;		
	int maximum_size;	
} History;

extern History history_list;	

//This struct comprises the Shell Variable List
//If will hold shell variables when created by the local command
typedef struct ShellVariables {
	char *name;
	char *value;
	struct ShellVariables *next;
} ShellVariables;

extern ShellVariables *variables;

//This variable will hold the value the program will return with incase the program
//ends from the end of the main method. It should return 0 in most situations, unless we
//run a command that isnt recognized by the system (TEST 11)
extern int exit_value;

//These Instantiate the Built-In methods and functionality
//of the shell
void execute_exit(char *args[]);
void execute_cd(char *args[]);
void execute_export(char *command);
void execute_local(char *command);
void execute_vars();
void execute_history(char *args[]);
void execute_ls();

//These Instantiate the Non Built-In methods and functionality
//of the shell. Noteworthly, they handle the ability to call 
//basic shell functions not explicitly covered by wsh shell using
//fork() and execv()
void execute_shell_command(char *args[]);
char* retrieve_command_path(const char *command);
void execute_fork_and_execv(const char *path, char *args[]);
void execute_redirection(char *input_file, char *output_file, char *error_file, int append);

//These Instantiate the helper methods for the History Command
//Specifically, these two methods assist in adding and retrieving
//commands from the History struct
void add_to_history(const char *command);
const char* retrieve_from_history(int n);

//These Instantiate the helper methods that are used more generally
//by the general shell main loop and command processing
int basic_comparison(const void *a, const void *b);
char *retrieve_shell_variable(const char *name);
void substitute_command_variables(char *command);

#endif 
