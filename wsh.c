#include "wsh.h"

//Global Variables (specified in wsh.h)
//By default, we start with nothing in the varible list, they will later be added
//by the local command when we need to
ShellVariables *variables = NULL;
//By default, the history list max size is 5, but this can be changed in the history functon
History history_list = { .command_count = 0, .maximum_size = HISTORY_MAXIMUM };
//By default, we will exit with 0, otherwise it should return 255 or -1 by Test 11
int exit_value = 0;

//Process Command will take in the string of command arguments and process them into
//tokens which the program can read. Then, it will use the tokens to call the methods
//of the requested commands
void process_command(char *command){
	//Strip new line from the end of command
	size_t len = strlen(command);
	if(command[len - 1] == '\n' && len > 0){
		command[len - 1] = '\0';
	}

	//Remove new lines from the end of the command
	//and remove leading whitespace at start
	while(*command == ' '){
		command++;
	}
	
	//Create a copy of the command path to add to history.
	//This needs to be done BEFORE the command is tokenized.
	//I ran into issues with Test 3 if I didnt do this...
	char *command_to_add = strdup(command);
	
	//Disregard all Comments
	if(*command == '#'){
		free(command_to_add);
		return;
	}
	//Disregard all New 
	if(*command == '\0'){
		free(command_to_add);
		return;
	}
	
	//Process the command so it can read it
	substitute_command_variables(command);

	char *args[ARGS_MAXIMUM];
	// Initialize variables for input, output, and error files
	char *input_file = NULL;
	char *output_file = NULL;
	char *error_file = NULL;
	int append = 0;
	int i = 0;

	// Tokenize command
	char *token = strtok(command, " ");
	while (token != NULL && i < ARGS_MAXIMUM - 1) {
    		if (token[0] == '>') {
       		 // '>' indicates to append
       			 if (token[1] == '>') {
            			output_file = token + 2;
            			append = 1;
        		} 
        		else {
            			output_file = token + 1;
            			append = 0;
        		}	
    		}
    		// Check for error redirection
    		else if (token[0] == '2' && token[1] == '>') {
        		error_file = token + 2;  // Get the file name for stderr redirection
    		}
    		// Handle input redirection
    		else if (token[0] == '<') {
       			input_file = token + 1;
    		} 
    		else {
        		args[i++] = token;  // Store command arguments
    		}
    		token = strtok(NULL, " ");
	}	
	args[i] = NULL;  // Null-terminate the args array
	
	free(token);
	
	//Call the requested command if we need to utilize
	//redirection functionality
	if(output_file != NULL || input_file != NULL || error_file != NULL){
		execute_redirection(input_file, output_file, error_file, append);
	}
	
	//Now, were ready to call a command
	//Based on what the leading argument is, call the matching mathod
	//to execute the desired
	if(strcmp(args[0], "exit") == 0){
		add_to_history(command_to_add);
		//jump to exit command
		free(command_to_add);
		execute_exit(args);
	} 
	else if(strcmp(args[0], "cd") == 0){
		add_to_history(command_to_add);
		free(command_to_add);
		//jump to cd command
		execute_cd(args);
	} 
	else if(strcmp(args[0], "export") == 0){
		add_to_history(command_to_add);
		free(command_to_add);
		//jump to export command
		execute_export(args[1]);
	} 
	else if(strcmp(args[0], "local") == 0){
		add_to_history(command_to_add);
		free(command_to_add);
		//jump to history command
		execute_local(args[1]);
	} 
	else if(strcmp(args[0], "vars") == 0){
		add_to_history(command_to_add);
		free(command_to_add);
		//jump to vars command
		execute_vars();
	} 
	else if(strcmp(args[0], "history") == 0){
		free(command_to_add);
		//jump to history command
		execute_history(args);
	} 
	else if(strcmp(args[0], "ls") == 0){
		add_to_history(command_to_add);
		free(command_to_add);
		//jump to ls command
		execute_ls();
	} 
	//If none of the Built-In Commands matched
	//We will call execute command to process it
	else{
		add_to_history(command_to_add);
		free(command_to_add);
		execute_shell_command(args);
	}
}


// BUILT IN COMMANDS


//This method handles the exit command
//Upon recieving the exit command and no arguments,
//this method will gracefully close the shell with the exit(0) command
void execute_exit(char *args[]){
	if(args[1] != NULL){
		fprintf(stderr, "exit: Too many Arguments\n");
		return;
	}
  	free(variables);
  	for (int i = 0; i < history_list.command_count; i++) {
    		if (history_list.commands[i] != NULL) {
        		//Free each command slot
        		free(history_list.commands[i]); 
        	//Avoid dangling pointers
        	history_list.commands[i] = NULL; 
    		}
	}
	exit(0);
}

//This method handles the cd command
//This method will always take 1 argument
//It will change the currently displayed directory in the shell
void execute_cd(char *args[]){
	if(args[2] != NULL){
		fprintf(stderr, "cd: Wrong number of arguments\n");
		return;
	}
	if(args[1] == NULL){
		fprintf(stderr, "cd: Wrong number of arguments\n");
		return;
	}
	if(chdir(args[1]) != 0){
		perror("cd");
	}
}

//This method will handle the export command
//It will either create or assign variable VAR as an enviroment variable
void execute_export(char *command){
	//Tokenize the input name and value from the command
	char *name = strtok(command, "=");
	char *value = strtok(NULL, " ");
	
	//If a valid name and value is given, create the new enviroment variable
	if(value && name){
		setenv(name, value, 1);	
	} 
	//If a valid name and value is not given, print out an error to the shell
	else{
		fprintf(stderr, "export: Invalid format, expected VAR=value\n");
	}
}

//This method will handle the local command
//It will either create or assign variable VAR as a shell variable
void execute_local(char *command){
	//Tokenize the input into name and value
	char *name = strtok(command, "=");
	char *value = strtok(NULL, " ");
	
	//If a valid name and value is given, create the new shell variable
	//This will utilize a seperate helper method to do so
	if(value && name){
		ShellVariables *vars = variables;
		//Cycle through the shell variables and find the
		//next empty node in the list
		while(vars != NULL){
			if(strcmp(vars -> name, name) == 0){
				char *temp_val = strdup(value);
				free(vars -> value);
				vars -> value = temp_val;
				free(temp_val);
				return;
			}
			//set the previous to point to the new variable
			vars = vars -> next;
		}
		
		//Set up the new variables
		ShellVariables *variable_new = malloc(sizeof(ShellVariables));
		char *temp_value = strdup(value);
		char *temp_name = strdup(name);
		//It needs to have its name, value and the next node set to NULL
		variable_new -> next = NULL;
		variable_new -> value = strdup(value);
		variable_new -> name = strdup(name);
		//Be sure to free strdup variables once done :)
		free(temp_value);
		free(temp_name);
		
		//If the variable list doesnt exist yet, create it
		if(variables == NULL){
			variables = variable_new;
		} 
		//If the variable list does exist, we add the new node into 
		//the correct spot
		else{
			ShellVariables *temporary = variables;
			while(temporary -> next != NULL){
				temporary = temporary->next;
			}	
			temporary -> next = variable_new;
		}	
	} 
	
	//no value is given, print out error and return
	else{
		fprintf(stderr, "local: Invalid format, expected VAR=value\n");
	}
}

//This method will handle the vars command
//As a partner to the env utility program, this method will print
//the local shell variables and their values in insertion order
void execute_vars(){
	ShellVariables *vars = variables;
	//Cycle through the list of shell variables
	//and print them out to the shell
	while(vars != NULL){
		//Note - we print in the format "Name=Value"
		printf("%s=%s\n", vars -> name, vars -> value);
		//Move on to the next variable
		vars = vars -> next;
	}	
}

//This method will handle the history command
//The history method will be able to have numerous functions including...
//Print out the latest 5 commands used, flexable history size and the ability to 
//call commands from the history list
void execute_history(char *args[]){
	//Function 1 - Print the History List
	//If there are no arguments, we just want to print out the history list
	if(args[1] == NULL){
		//Cycle through the history list and print out the commands
		for(int j = 0; j <= history_list.command_count - 1; j++){
			printf("%d) %s\n", j + 1, history_list.commands[j]);
		}	
	} 
	//Function 2 - Set a New History List Size
	//If we have the set argument, we will set a new history size
	else if(strcmp(args[1], "set") == 0){
		int new_size = atoi(args[2]);
		//If the new size if less than the size of the previous,
		//we need to evict the contents of the history list above the new size
		if(new_size < history_list.command_count){
			for(int k = new_size; k <= history_list.command_count - 1; k++){
				free(history_list.commands[k]);
			}
			//Once we're done, we need to adjust the amount of commands
			//in history to the new total size
			history_list.command_count = new_size;
		}
		//Now, we need to set the history list's max size to that of the new size
		history_list.maximum_size = new_size;
	} 
	//Function 3 - Execute a Command from the History List
	//If we made it this far, we want to execute a command from history
	else{
		int index = atoi(args[1]);
		//Use the helper method to retrieve the command from the correct
		//index in the history list
		const char *retrieved_command = retrieve_from_history(index);
		//If the command was never set and doesnt exit, print error and do nothing
		if(retrieved_command == NULL){
			fprintf(stderr, "history: No such command in history\n");
		}
		//If the command exists, we call it like we had from the shell
		if(retrieved_command != NULL){
			char *command_temp = strdup(retrieved_command);
			printf("Executing: %s\n", retrieved_command);
			//Run the requested command specified in history
			process_command(command_temp);	
			free(command_temp);
		} 
	}
}

//This method will handle the ls command
//This method will print out a list of the current directory content when requested
void execute_ls(){
	DIR *dir;
	struct dirent *entry;
	int file_count = 0;
	char *file_list[100];
	
	//Prepair the current directory
	dir = opendir(".");
	if(dir == NULL){
		perror("ls");
		return;
	}
	//Read in directory contents into file list
	while((entry = readdir(dir)) != NULL){
		//Ignore hidden files (files beginning with '.')
		if(entry -> d_name[0] == '.') {
			continue;
		}
		file_list[file_count] = strdup(entry -> d_name);
		++file_count;
	}
	//close out directory once were done
	closedir(dir);
	//Sort and print the directory contents to the shell
	qsort(file_list, file_count, sizeof(char *), basic_comparison);
	for(int k = 0; k <= file_count - 1; k++){
		printf("%s\n", file_list[k]);
		//Once we're done, we need to free the list to prevent leaks
		free(file_list[k]);
	}
}


// Methods for NON Built-In Commands


//This method handles the highest level of executing a non built-in shell command.
//It will take in the specified arguments and use the arguments to call a shell command. 
void execute_shell_command(char *args[]){
	//is the command valid?
	if(access(args[0], X_OK) == 0){
		execute_fork_and_execv(args[0], args);
		return;
	}
	//Search for the command in the directory
	//If found, run it!
	char *full_path = retrieve_command_path(args[0]);
	if(full_path != NULL){
		execute_fork_and_execv(full_path, args);
	} 
	//If here, we were unable to locate the command
	//Send an error message and do nothing
	else{
		exit_value = -1;
	}
}

//When a non built-in command is found in the directory,
//we will create a copy of the process with fork and we will run the 
//requested command with execv
void execute_fork_and_execv(const char *path, char *args[]){
	pid_t pid = fork();
	//Did fork work?
	if(pid < 0){
		perror("Fork Failed");
	} 
	//when fork is valid, the child process is created successfully
	//we will then call execv to run the new command with its path and arguments
	else if(pid == 0){
		execv(path, args);
		//Did execv work?
		perror("Command Execution failed");
		exit(1);
	} 
	//the parent process will now wait until the child process finishes the
	//requested command. Once finished, the parent process will resume.
	else{
		//Now, in the parent process, we need to wait for the child
		//process to conclude before the parent resumes
		wait(NULL);
	}
}

//This method will handle retrieving the command path
//so execv can call the new command
char* retrieve_command_path(const char *command){
	//Get the value of the path searching variable
	char *path_env = getenv("PATH");
	if(path_env == NULL){
		return NULL;	
	}
	//Now, make a copy of the PATH variable and we will now
	//split the PATH variable into its individual directories
	char *path_copy = strdup(path_env);
	char *dir = strtok(path_copy, ":");
	static char full_path[COMMAND_MAXIMUM];
	
	//Finally, we need to search the directories for our command
	while(dir != NULL){
		snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
		//If a matching path is found, we return it!
		if(access(full_path, X_OK) == 0){
			free(path_copy);
			return full_path;	
		}
		//If not found yet, we split the token and continue
		dir = strtok(NULL, ":");
	}
	//The command couldnt be located, free the path and return
	free(path_copy);
	return NULL;	
}

//This method will handle redirection functionality.
//Redirections will allow the file handles of commands be duplicated, opened, closed,
//made to refer to different files, and change destination files for command reads and writes (Bash Reference Manual)
//Redirection will occur in the event an input or output file exist
void execute_redirection(char *input_file, char *output_file, char *error_file, int append) {
    // Handle input redirection (stdin)
    if (input_file != NULL) {
        int fd = open(input_file, O_RDONLY);
        if (fd == -1) {
            perror("Error opening input file");
            exit(1);
        }
        // Redirect stdin to input file
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    // Handle output redirection (stdout)
    if (output_file != NULL) {
        int fd;
        // Open file for writing, either truncating or appending
        if (!append) {
            fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        if (fd == -1) {
            perror("Error opening output file");
            exit(1);
        }
        // Redirect stdout to output file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    // Handle error redirection (stderr)
    if (error_file == NULL && input_file != NULL) {
        // Create output filename based on the input filename
        char out_file[256];
        strncpy(out_file, input_file, sizeof(out_file) - 1);
        out_file[sizeof(out_file) - 1] = '\0'; // Null-terminate
        
        // Remove the extension if it exists
        char *ext = strrchr(out_file, '.');
        if (ext != NULL) {
            *ext = '\0';  // Remove extension
        }
        strcat(out_file, "-out"); // Append -out to the filename
        
        // Open the -out file for writing
        int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Error opening error output file");
            exit(1);
        }
        // Redirect stderr to the -out file
        dup2(fd, STDERR_FILENO);
        close(fd);
    } else if (error_file != NULL) {
        // If an error file is specified, handle it
        int fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Error opening error output file");
            exit(1);
        }
        // Redirect stderr to the user-specified error output file
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}




// Methods for History Command


//This Method will handle adding a new command to the history list
//If the command is already there, it wont repeat it. If it isnt,
//it will add it to the list and update its values accordingly
void add_to_history(const char *command){
	//Check if its already in the history list
	if(history_list.command_count > 0 && strcmp(history_list.commands[0], command) == 0){
		return;		
	}
	//If the command isnt in the history list, we continue
	//If the history list is full, we need to evict the oldest one
	//and replace it with the new command
	if(history_list.maximum_size == history_list.command_count){
		free(history_list.commands[history_list.maximum_size - 1]);
		for(int j = history_list.maximum_size - 1; j > 0; j--){
			history_list.commands[j] = history_list.commands[j - 1];
		}
	} 
	//if the history list isnt already full, we can add the new command
	//without modifying the history list
	else{
		for(int k = history_list.command_count; k > 0; k--){
			history_list.commands[k] = history_list.commands[k - 1];
		}
		//because the list wasnt full, we update the number of commands in the list
		++history_list.command_count;
	}
	//We now insert the new command into the now empty most recent spot
	history_list.commands[0] = strdup(command);
}


//This method will be used when we want to execute a command from memory
//It will access command and return the specified command
const char* retrieve_from_history(int n){
	if(n <= history_list.command_count && n > 0){
		return history_list.commands[n - 1];
	}
        return NULL;	
}


// Helper Methods for General Use By Program


//Used when conducting LS as it does a quick sort of directory content
//to do comparison between items
int basic_comparison(const void *a, const void *b){
	return strcmp(*(const char **)a, *(const char **)b);
}

//Used in substitute variable method.
//It checks though the existing shell variables and determines if
//the passed in variable already exists
char *retrieve_shell_variable(const char *name){
	//Retrieve a copy of the variable list
	ShellVariables *vars = variables;
	while(vars != NULL){
		//If a match is found, return it
		if(strcmp(vars -> name, name) == 0){
			return vars -> value;
		}
		vars = vars -> next;
	}
	//Will return NULL if the variable isnt found
	return NULL;	
}

//This method will handle the substitution of variables after
//process command is finished reading in the command and before
//it prepares to execute it
void substitute_command_variables(char *command){
	char command_buffer[COMMAND_MAXIMUM];
	char *buff = command_buffer;
	char *com = command;
	//While there are still commands in the buffer...
	while(*com){
		//$ denotes the start of a variable
		if(*com == '$'){
			//read in the variable
			char var_name[COMMAND_MAXIMUM];
			char *v = var_name;
			++com;
			//Continue to increment unless *com is a space or end line
			while(*com && *com != ' ' && *com != '\0'){
				*v++ = *com++;
			}
			*v = '\0';
			//Check the shell for existing command
			char *value = getenv(var_name);
			if(value == NULL){
				value = retrieve_shell_variable(var_name);
			}
			if(value){
				while(*value) {
					*buff++ = *value++;
				}
			}
		} 
		else{
			*buff++ = *com++;
		}
	}
	//Once finsihed, add on an end line to finish the command
	*buff = '\0';
	strcpy(command, command_buffer);	
}


//The main method in this program will determine if the shell is in batch or
//interactive mode. Then, it will process the commands as necassary
int main(int argc, char *argv[]){
	char *path = getenv("PATH");
	char command[COMMAND_MAXIMUM];
	if(strlen(path) == 0){
		setenv("PATH", "/bin", 1);	
	}
	if(path == NULL){
		setenv("PATH", "/bin", 1);	
    		printf("PATH=/bin");
	}
  	setenv("PATH", "/bin", 1);
	//If two arguments exist, this is BATCH mode
	if(argc == 2){
		//Open and read batch file
		FILE *file = fopen(argv[1], "r");
		if(file == NULL) {
			perror("Error opening batch file");
			return -1;
		}
		while (fgets(command, sizeof(command), file) != NULL){
			process_command(command);
		}
		fclose(file);
	} 
	//If only 1 argument exists, this is INTERACTIVE mode
	else if(argc == 1){
		while(1){
			//print out curser to shell display
			printf("wsh> ");
			fflush(stdout);
			if(fgets(command, sizeof(command), stdin) == NULL){
				break;	
			}
			process_command(command);
		}
	} 
	//If input isnt valid, specify usage and return -1
	else{
		fprintf(stderr, "Usage: %s [batch-file]\n", argv[0]);
		return -1;
	}
	//If all is good, we return 0
	return exit_value;
}
