#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_LEN 1024
#define MAX_PATHS 100
#define MAX_PATH_LEN 1024
#define MAX_ARGS 100

char search_path[MAX_PATHS][MAX_PATH_LEN] = {"/bin"};
void print_error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}
void run_cd(char *args[]) {
	if (args[1] == NULL || args[2] != NULL)
		print_error();

	else {
		if (chdir(args[1]) != 0) // chdir failing
			print_error();
	}
}

void set_path(char *args[]) {
	int count = 0;

	for (int i = 1; args[i] != NULL; i++) {
		char *path = strtok(args[i], " ");
		while (path != NULL)
		{
			if (count == MAX_PATHS) {
				// Path limit exceeeded
				print_error();
				return;
			}
			strcpy(search_path[count], path);
			count++;
			path = strtok(NULL, " ");
		}
	}
	search_path[count][0] = '\0';
}

bool built_in(const char *cmd) {
	return strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0 ||
	       strcmp(cmd, "path") == 0;
}

void exec_cmd(char *args[], const char *output) {
	int stdout_copy = dup(STDOUT_FILENO);

	if (output) {
		//redirection
		//file permissions
		int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0) {
			//open error
			print_error();
			return;
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	pid_t child_pid = fork();
	if (child_pid == -1) {
		// fork error
		print_error();
		exit(1);
	}
	else if (child_pid == 0) {
		char full_path[MAX_PATH_LEN];
		bool found = false;

		for (int i = 0; search_path[i][0] != '\0'; i++) {
			snprintf(full_path, sizeof(full_path), "%s/%s", search_path[i], args[0]);

			if (access(full_path, X_OK) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			// fprintf(stderr, "Command not found: %s\n", args[0]);
			print_error();
			exit(1);
		}

		execv(full_path, args);
		// execv error
		print_error();
		exit(1);
	}
	else {
		int status;
		waitpid(child_pid, &status, 0);
	}

	dup2(stdout_copy, STDOUT_FILENO);
	close(stdout_copy);
}

void parse_cmd(char *input, char *args[], int *arg_n, char **output_file) {
    *arg_n = 0;
    *output_file = NULL;
    char *token = strtok(input, " ");
    bool redir_flag = false;
    while (token != NULL) {
            if (strcmp(token, ">") == 0) {
                //should maybe change so it breaks
                //also replace stderrs with generic err msg from doc
                if (redir_flag) {
                    // "Multiple redirection operators are not allowed\n"
                    print_error();
                    return;
                }
                redir_flag = true;

                token = strtok(NULL, " ");
                if (token == NULL) {
                    // No output file specified\n
                    print_error();
                    return;
                }
                if (*output_file != NULL) {
                    // "Multiple output files are not allowed\n
                    print_error();
                    return;
                }
            *output_file = token;
        } else {
            args[*arg_n] = token;
            (*arg_n)++;
        }
        token = strtok(NULL, " ");
    }
    args[*arg_n] = NULL;
}

void exec_parallel_cmd(char *cmd[], int cmd_num){
    pid_t pids[MAX_ARGS];

    for (int i = 0; i < cmd_num; i++) {
        char *args[MAX_ARGS];
        char *output_file;
        int arg_count;

        parse_cmd(cmd[i], args, &arg_count, &output_file);

        if (arg_count > 0) {
            if (built_in(args[0])) {
                if (strcmp(args[0], "exit") == 0) {
                    if (arg_count > 1) {
                        // Too many arguments"
                        print_error();
                    } else {
                        exit(0);
                    }
                } else if (strcmp(args[0], "cd") == 0) {
                    run_cd(args);
                } else if (strcmp(args[0], "path") == 0) {
                    set_path(args);
                }
            } else {
                pids[i] = fork();
                if (pids[i] == 0) {
                    exec_cmd(args, output_file);
                    exit(0);
                } else if (pids[i] < 0) {
                    print_error();
                }
            }
        }
    }

    for (int i = 0; i < cmd_num; i++) {
        if (pids[i] > 0) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

void parse_input(char *input, char *commands[], int *num_commands) {
    *num_commands = 0;
    char *token = strtok(input, "&");
    while (token != NULL) {
        commands[*num_commands] = token;
        (*num_commands)++;
        token = strtok(NULL, "&");
    }
}

int main(int MainArgc, char *MainArgv[]) {
	bool interactive_mode = true;
	// bool parallel = false;
	FILE *input_stream = stdin;

	if (MainArgc == 2) {
		interactive_mode = false;
		input_stream = fopen(MainArgv[1], "r");

		if(!input_stream) {
			print_error();
			exit(1);
		}
	}
	else if (MainArgc > 2) {
		print_error();
		exit(1);
	}
	char input[MAX_LEN];
	char *args[MAX_ARGS];
	int cmd_num;

	while (1) {
		// char cwd[MAX_PATH_LEN];
		// getcwd(cwd, sizeof(cwd));
		if (interactive_mode) {
			// printf("witsshell (%s)> ", cwd);
			printf("witsshell> ");
			// fflush(stdout);
		}

		if (fgets(input, sizeof(input), input_stream) == NULL)
			break; // eof

		input[strlen(input) - 1] = '\0';

		parse_input(input, args, &cmd_num);
		exec_parallel_cmd(args, cmd_num);
		// > operator
		// & operator check

	}
	if (!interactive_mode) {
		fclose(input_stream);
	}

	return(0);
}
