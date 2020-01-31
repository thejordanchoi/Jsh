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

	while(1){
		printf("$ ");
		// Command cmd;
		// Command* cmd_ptr = &cmd; //pointer to Command cmd
		Command* cmd_ptr = (Command*)malloc(sizeof(Command));

		read_line(cmd_ptr);
		parse(cmd_ptr);

		if(execute(cmd_ptr) < 0){
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
		if(strcmp(token, "<") == 0){
			// input redirect
			//infile_position = position + 1;
			cmd->infile = strtok(NULL, " \n\t\r");
		}
		else if(strcmp(token, ">") == 0){
			// output redirect
			//outfile_position = position + 1;
			cmd->outfile = strtok(NULL, " \n\t\r");
		}
		else{
			cmd->args[position] = token;
			++position;
		}
		token = strtok(NULL, " \n\t\r");
		
	}

	cmd->args[position] = NULL;
	cmd->argc = position;

	return;
}

int execute(Command* cmd){
	//basic command:
	if(strcmp(cmd->args[0],"quit") == 0){
		return -1;
	}

	pid_t pid = fork();
	if(pid < 0){
		perror("fork() error\n");
		exit(-1);
	}
	else if(pid == 0){
		//child process

		int save_stdout = dup(1); //save std out
		int save_stdin = dup(0); //save std in
		printf("save stdout: %d", save_stdout);
		printf("save stdin: %d", save_stdin);

		if(cmd->outfile){
			int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			if(fd < 0){
				perror("open (write) error\n");
			}
			dup2(fd,1);
			close(fd);
		}
		if(cmd->infile){
			int fd = open(cmd->infile, O_RDONLY);
			if(fd < 0){
				perror("open (read) error\n");
			}
			dup2(fd,0);
			close(fd);
		}

		//execute command
		if(execvp(cmd->args[0], cmd->args) == -1){
			perror("execv error\n");
		}

		//replace stdout / in
		close(1);
		close(0);
		dup2(save_stdin, 0);
		dup2(save_stdout, 1);

		exit(0);
	}
	else{
		// parent
		wait(NULL);
	}

	return 0;
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

