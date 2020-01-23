#include <string.h>
#include <stdio.h>
#include <stdlib.h> //malloc
#include <unistd.h> 
#include<fcntl.h>

#define BUFFER_SIZE 80;

struct Command{
	char* raw_input;
	char** args;
	int argc;

	char* infile;
	char* outfile;
};
typedef struct Command Command;

void read_line(Command* cmd);
void parse(Command* cmd);
int execute(Command* cmd);

int main(int argc, char* argv[]){
	char* welcome_msg = "Welcome to Jsh, Jordan's Shell\n";
	printf("%s", welcome_msg);

	int status = 1;

	while(status){
		printf("$ ");
		Command cmd;
		Command* cmd_ptr = &cmd; //pointer to Command cmd

		read_line(cmd_ptr);
		parse(cmd_ptr);

		status = execute(cmd_ptr);

		if(status == 0){
			break;
		}
		//might want to free memory in cmd here
	}

	return EXIT_SUCCESS;
}

void read_line(Command* cmd){ //this function can also be implemented with getline (way easier)
	int buffer_size = BUFFER_SIZE;
	cmd->raw_input = malloc(sizeof(char) * buffer_size);
	int position = 0;
	int c;

	//malloc check
	if(!cmd->raw_input){
		perror("malloc error\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		if(position == buffer_size){
			perror("exceeded memory allocated error\n");
			exit(EXIT_FAILURE);
		}
		c = getchar(); //same as getc(stdin)

		if(c == EOF || c == '\n'){ //user can invoke eof with ctrl+z
			cmd->raw_input[position] = '\0';
			return;
		}
		else{
			cmd->raw_input[position] = c;
		}
		++position;
	}
}

void parse(Command* cmd){
	int buffer_size = BUFFER_SIZE;
	int max_arg = buffer_size / 2;
	int position = 0;
	//char** args = malloc(sizeof(char*) * max_arg);
	cmd->args = malloc(sizeof(char*) * max_arg);
	char* token;

	//malloc check
	if(!(cmd->args)){
		perror("malloc error\n");
		exit(EXIT_FAILURE);
	}

	int infile_position = -1;
	int outfile_position = -1;
	token = strtok(cmd->raw_input, " \n\t\r");

	while(token != NULL){
		cmd->args[position] = token;
		if(strcmp(token, "<") == 0){
			// input redirect
			infile_position = position + 1;
		}
		if(strcmp(token, ">") == 0){
			// output redirect
			outfile_position = position + 1;
		}
		token = strtok(NULL, " \n\t\r");
		++position;
	}

	cmd->args[position] = NULL;
	cmd->argc = position;
	if(infile_position != -1){
		cmd->infile = cmd->args[infile_position];
	}
	if(outfile_position != -1){
		cmd->outfile = cmd->args[outfile_position];
	}
	return;
}

int execute(Command* cmd){
	//basic command:
	if(strcmp(cmd->args[0],"quit") == 0){
		return 0;
	}

	//redirection handling:
	int tempin = dup(0);
	int tempout = dup(1);
	int fdout;
	int fdin;

	if(cmd->outfile){
		fdout = open(cmd->outfile, O_WRONLY);
		dup(fdout);
	}
	if(cmd->infile){
		fdin = open(cmd->infile, O_RDONLY);
		dup(fdin);
	}

	pid_t pid;
	int status;
	pid = fork();
	if(pid == 0){
		// child process
		//printf("child pid: %i\n", pid);
		if(execvp(cmd->args[0], cmd->args) == -1){
			perror("execv error\n");
		}
	}
	else if(pid < 0){
		// failed
		perror("fork error\n");
	}
	else{
		// parent
		//printf("parent pid: %i\n", pid);
		waitpid(pid, &status, 0); // WNOHANG parent can run concurrently as child
	}



	return 1;
}

// input 80 char limit

// all commands are in the form 
//		COMMAND [arg1] [arg2] ... [argn]

// print result to screen

// quit command

//general commands should be executed in a child process
//which is spawned by the shell process using a fork command
//Be sure to reap all terminated child processes

//IO Redirection
// > and <

//Background commands
//When a user wants a command to be executed int he background,
//an '&' character is added to the end of the command line

