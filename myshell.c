#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


// Function to execute a single command
void executeCommand(char **s) { // s is array of tokens
    // Handle 'cd' command
    if (strcmp(s[0], "cd") == 0) {
	    if (s[1] == NULL) {
		// No argument → go to HOME
		char *home = getenv("HOME");
		if (home == NULL) home = "/";  // fallback
			if (chdir(home) != 0) {
			    printf("Shell: Incorrect command\n");
			}
	    } else {
		// Argument exists → try to change to that path
		if (chdir(s[1]) != 0) {
		    printf("Shell: Incorrect command\n");
		}
	    }
	    return;
	}


    // Fork a new process to execute the command
    if (fork() == 0) {
        signal(SIGTSTP, SIG_DFL);  // Reset SIGTSTP signal handling
        signal(SIGINT, SIG_DFL);   // Reset SIGINT signal handling
        execvp(s[0], s);          // Execute the command
        printf("Shell: Incorrect command\n"); // Error message for command execution failure
        exit(1);  // Exit child process
    } else {
        wait(NULL);  // lets wait for our child process to finish
    }
}

// Function to execute multiple commands in parallel
void executeParallelCommands(char **s, int i) {
    int start = 0;
    for (int j = 0; j < i; j++) {
        if (strcmp(s[j], "&&") == 0 || j == i - 1) {
            // End of one command
            int end = (strcmp(s[j], "&&") == 0) ? j : j + 1;
            s[end] = NULL;  // terminate this argv segment

            if (fork() == 0) {
                execvp(s[start], &s[start]);
                perror("execvp failed");
                exit(1);
            }
            start = j + 1; // move to next command
        }
    }

    // wait for all children
    while (wait(NULL) > 0);
}

// Function to execute multiple commands sequentially
void executeSequentialCommands(char **s, int i) {
    int start = 0; // to keep track of start of each command
    for (int j = 0; j <= i; j++) {
        if (s[j] == NULL || strcmp(s[j], "##") == 0) {
            s[j] = NULL;  // mark end of one command

            if (start < j) {
                // Handle "cd" separately in parent
                if (strcmp(s[start], "cd") == 0) {
                    if (s[start+1] != NULL) {
                        if (chdir(s[start+1]) != 0) {
                            printf("Shell: Incorrect command\n");
                        }
                    } else {
                        printf("Shell: Incorrect command\n");
                    }
                } else {
                    if (fork() == 0) {
                        execvp(s[start], &s[start]);
                        printf("Shell: Incorrect command\n");
                        exit(1);
                    } else {
                        wait(NULL);
                    }
                }
            }
            start = j + 1; // move to next command after ##
        }
    }
}

// Function to handle command output redirection
void executeCommandRedirection(char **s, int i) {
    int redirectIndex = -1;
    int append = 0;

    // find redirection symbol
    for (int j = 0; j < i; j++) {
        if (strcmp(s[j], ">") == 0) {
            redirectIndex = j;
            append = 0;
            break;
        }
        if (strcmp(s[j], ">>") == 0) {
            redirectIndex = j;
            append = 1;
            break;
        }
    }

    if (redirectIndex == -1 || s[redirectIndex+1] == NULL) { 
        printf("Shell: Incorrect command\n");
        return;
    }

    char *filename = s[redirectIndex+1]; //store the filename in string 
    s[redirectIndex] = NULL;  // cut off command before ">"
    
    if (fork() == 0) {
        int fd;
        if (append)
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0) {
            printf("Shell: Cannot open file\n");
            exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);

        execvp(s[0], s);
        printf("Shell: Incorrect command\n");
        exit(1);
    } else {
        wait(NULL);
    }
}


void executePipelinedCommands(char **s, int i) {
    int start = 0;
    int pipefd[2], in_fd = 0; // tells us current command shoud read frmm

    for (int j = 0; j <= i; j++) {
        if (s[j] == NULL || strcmp(s[j], "|") == 0) {
            s[j] = NULL;  

            
            if (j != i) pipe(pipefd); // we dont want to create for the last command 

            if (fork() == 0) {
                dup2(in_fd, 0);              // read end from previous command
                if (j != i) dup2(pipefd[1], 1); // write end to next command

                if (j != i) close(pipefd[0]); // read end is closed
                execvp(s[start], &s[start]);
                printf("Shell: Incorrect command\n");
                exit(1);
            } else {
                wait(NULL);   
                if (j != i) {
                    close(pipefd[1]); // close write end in parent
                    in_fd = pipefd[0]; // next command will read from here
                }
                start = j + 1; 
            }
        }
    }
}


void handle_sigint() {
}

void handle_sigtstp() {
    
}

int main() {
    printf("Welcome to the Shell! Type 'exit' to quit.\n");
    while (1) {
        // Setup signal handlers
        signal(SIGINT, handle_sigint);    // Handle Ctrl + C->terminated our process
        signal(SIGTSTP, handle_sigtstp);  // Handle Ctrl + Z->suspends the process
        
        char buf[100];
        
        // prints the shell prompt
        if (getcwd(buf, sizeof(buf))) {
            printf("%s$ ", buf);
        } else {
            printf("Shell: Incorrect command\n");
        }

        // User input
        char *inp_buf;
        size_t buf_size = 50;
        size_t characters;
        inp_buf = (char*) malloc(sizeof(char) * buf_size);
        
        if (inp_buf == NULL) {
            printf("Shell: Incorrect command\n");
            break;
        }

        characters = getline(&inp_buf, &buf_size, stdin);
        char *tokens[10]; //tokens
        int i = 0; // to keep sizze of tokens
        
        // Tokenize the input string
        while ((tokens[i] = strsep(&inp_buf, " ")) != NULL) {
            if (strlen(tokens[i]) == 0) continue; // Skip empty tokens in the start in the end and in the middle 
            i++;
        }

        // Remove newline characters from tokens
        for (int j = 0; j < i; j++) {
            tokens[j] = strsep(&tokens[j], "\n");
        }

        // Handle exit command
        if (i > 0 && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        int seq = 0, par = 0, red = 0; // for checkng sequence , parallel or redirect 
	int pipe = 0;
	for (int j = 0; j < i; j++) {
	    if (strcmp(tokens[j], "##") == 0) seq = 1;
	    if (strcmp(tokens[j], "&&") == 0) par = 1;
	    if (strcmp(tokens[j], ">") == 0 || strcmp(tokens[j], ">>") == 0) red = 1;
	    if (strcmp(tokens[j], "|") ==0) pipe = 1;
	}

	if (par) {
	    executeParallelCommands(tokens, i);
	} else if (seq) {
	    executeSequentialCommands(tokens, i);
	} else if (red) {
	    executeCommandRedirection(tokens, i);
	} else if(pipe){
		executePipelinedCommands(tokens, i);
	}
		
	else {
	    executeCommand(tokens);
	}

        free(inp_buf); // Free allocated memory for input buffer
    }
    return 0;
}
