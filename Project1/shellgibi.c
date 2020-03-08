#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>

const char *sysname = "shellgibi";


enum return_codes {
    SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};
struct command_t {
    char *name;
    bool background;
    bool auto_complete;
    int arg_count;
    char **args;
    char *redirects[3]; // in/out redirection
    struct command_t *next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command) {
    int i = 0;
    printf("Command: <%s>\n", command->name);
    printf("\tIs Background: %s\n", command->background ? "yes" : "no");
    printf("\tNeeds Auto-complete: %s\n", command->auto_complete ? "yes" : "no");
    printf("\tRedirects:\n");
    for (i = 0; i < 3; i++)
        printf("\t\t%d: %s\n", i, command->redirects[i] ? command->redirects[i] : "N/A");
    printf("\tArguments (%d):\n", command->arg_count);
    for (i = 0; i < command->arg_count; ++i)
        printf("\t\tArg %d: %s\n", i, command->args[i]);
    if (command->next) {
        printf("\tPiped to:\n");
        print_command(command->next);
    }


}

/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command) {
    if (command->arg_count) {
        for (int i = 0; i < command->arg_count; ++i)
            free(command->args[i]);
        free(command->args);
    }
    for (int i = 0; i < 3; ++i)
        if (command->redirects[i])
            free(command->redirects[i]);
    if (command->next) {
        free_command(command->next);
        command->next = NULL;
    }
    free(command->name);
    free(command);
    return 0;
}

/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt() {
    char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
    return 0;
}

/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command) {
    const char *splitters = " \t"; // split at whitespace
    int index, len;
    len = strlen(buf);
    while (len > 0 && strchr(splitters, buf[0]) != NULL) // trim left whitespace
    {
        buf++;
        len--;
    }
    while (len > 0 && strchr(splitters, buf[len - 1]) != NULL)
        buf[--len] = 0; // trim right whitespace

    if (len > 0 && buf[len - 1] == '?') // auto-complete
        command->auto_complete = true;
    if (len > 0 && buf[len - 1] == '&') // background
        command->background = true;

    char *pch = strtok(buf, splitters);
    command->name = (char *) malloc(strlen(pch) + 1);
    if (pch == NULL)
        command->name[0] = 0;
    else
        strcpy(command->name, pch);

    command->args = (char **) malloc(sizeof(char *));

    int redirect_index;
    int arg_index = 0;
    char temp_buf[1024], *arg;
    while (1) {
        // tokenize input on splitters
        pch = strtok(NULL, splitters);
        if (!pch) break;
        arg = temp_buf;
        strcpy(arg, pch);
        len = strlen(arg);

        if (len == 0) continue; // empty arg, go for next
        while (len > 0 && strchr(splitters, arg[0]) != NULL) // trim left whitespace
        {
            arg++;
            len--;
        }
        while (len > 0 && strchr(splitters, arg[len - 1]) != NULL) arg[--len] = 0; // trim right whitespace
        if (len == 0) continue; // empty arg, go for next

        // piping to another command
        if (strcmp(arg, "|") == 0) {
            struct command_t *c = malloc(sizeof(struct command_t));
            int l = strlen(pch);
            pch[l] = splitters[0]; // restore strtok termination
            index = 1;
            while (pch[index] == ' ' || pch[index] == '\t') index++; // skip whitespaces

            parse_command(pch + index, c);
            pch[l] = 0; // put back strtok termination
            command->next = c;
            continue;
        }

        // background process
        if (strcmp(arg, "&") == 0)
            continue; // handled before

        // handle input redirection
        redirect_index = -1;
        if (arg[0] == '<')
            redirect_index = 0;
        if (arg[0] == '>') {
            if (len > 1 && arg[1] == '>') {
                redirect_index = 2;
                arg++;
                len--;
            } else redirect_index = 1;
        }
        if (redirect_index != -1) {
            command->redirects[redirect_index] = malloc(len);
            strcpy(command->redirects[redirect_index], arg + 1);
            continue;
        }

        // normal arguments
        if (len > 2 && ((arg[0] == '"' && arg[len - 1] == '"')
                        || (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
        {
            arg[--len] = 0;
            arg++;
        }
        command->args = (char **) realloc(command->args, sizeof(char *) * (arg_index + 1));
        command->args[arg_index] = (char *) malloc(len + 1);
        strcpy(command->args[arg_index++], arg);
    }
    command->arg_count = arg_index;
    return 0;
}

void prompt_backspace() {
    putchar(8); // go back 1
    putchar(' '); // write empty over
    putchar(8); // go back 1 again
}

/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command) {
    int index = 0;
    char c;
    char buf[4096];
    static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);


    //FIXME: backspace is applied before printing chars
    show_prompt();
    int multicode_state = 0;
    buf[0] = 0;
    while (1) {
        c = getchar();
        // printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

        if (c == 9) // handle tab
        {
            buf[index++] = '?'; // autocomplete
            break;
        }

        if (c == 127) // handle backspace
        {
            if (index > 0) {
                prompt_backspace();
                index--;
            }
            continue;
        }
        if (c == 27 && multicode_state == 0) // handle multi-code keys
        {
            multicode_state = 1;
            continue;
        }
        if (c == 91 && multicode_state == 1) {
            multicode_state = 2;
            continue;
        }
        if (c == 65 && multicode_state == 2) // up arrow
        {
            int i;
            while (index > 0) {
                prompt_backspace();
                index--;
            }
            for (i = 0; oldbuf[i]; ++i) {
                putchar(oldbuf[i]);
                buf[i] = oldbuf[i];
            }
            index = i;
            continue;
        } else
            multicode_state = 0;

        putchar(c); // echo the character
        buf[index++] = c;
        if (index >= sizeof(buf) - 1) break;
        if (c == '\n') // enter key
            break;
        if (c == 4) // Ctrl+D
            return EXIT;
    }
    if (index > 0 && buf[index - 1] == '\n') // trim newline from the end
        index--;
    buf[index++] = 0; // null terminate string

    strcpy(oldbuf, buf);

    parse_command(buf, command);

    // print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
    return SUCCESS;
}

void combine_path(char *, char *, char *);

int process_command(struct command_t *command, int pipeFromParent[]);

int execv_command(struct command_t *command);

int main() {
    while (1) {
        struct command_t *command = malloc(sizeof(struct command_t));
        memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

        int code;
        code = prompt(command);
        if (code == EXIT) break;

        code = process_command(command, NULL);
        if (code == EXIT) break;

        free_command(command);
    }

    printf("\n");
    return 0;
}

int process_command(struct command_t *command, int pipeFromParent[]) {

    if (pipeFromParent != NULL) {
        close(pipeFromParent[1]); // we will only use read end
    }

    int r;
    if (strcmp(command->name, "") == 0) {
        if (pipeFromParent != NULL) {
            close(pipeFromParent[0]);
        }
        return SUCCESS;
    }

    if (strcmp(command->name, "exit") == 0) {
        if (pipeFromParent != NULL) {
            close(pipeFromParent[0]);
        }
        return EXIT;
    }

    if (strcmp(command->name, "cd") == 0) {
        if (command->arg_count > 0) {
            r = chdir(command->args[0]);
            if (r == -1)
                printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
            if (pipeFromParent != NULL) {
                close(pipeFromParent[0]);
            }
            return SUCCESS;
        }
    }

    pid_t pid = fork();
    if (pid == 0) // child
    {
        // Handle redirecting

        int num_redirects_from_stdout = 0;

        if (command->redirects[1] != NULL) {
            num_redirects_from_stdout++;
        }

        if (command->redirects[2] != NULL) {
            num_redirects_from_stdout++;
        }

        if (command->next) {
            num_redirects_from_stdout++;
        }

        // if stdout is redirected to more than one file/pipe, store it in a temp file and copy later
        int stdout_redirected_to_multiple_files = num_redirects_from_stdout > 1;

        // <: input is read from a file
        if (command->redirects[0] != NULL) {
            freopen(command->redirects[0], "r", stdin);
        } else if (pipeFromParent != NULL) {
            dup2(pipeFromParent[0], STDIN_FILENO);
        }

        // >: output is written to a file, in write mode
        if (!stdout_redirected_to_multiple_files && command->redirects[1] != NULL) {
            freopen(command->redirects[1], "w", stdout);
        }
            // >>: output is written to a file, in append mode
        else if (!stdout_redirected_to_multiple_files && command->redirects[2] != NULL) {
            freopen(command->redirects[2], "a", stdout);
        }

        if (!stdout_redirected_to_multiple_files && command->next) {

        }

        if (stdout_redirected_to_multiple_files) {
            char temp_filename[16 + 1];
            tmpnam(temp_filename);

            pid_t pid2 = fork();
            if (pid2 == 0) {
                // grandchild
                FILE *tmp_file = fopen(temp_filename, "w");
                dup2(fileno(tmp_file), STDOUT_FILENO);
                return execv_command(command);
            } else {
                // wait for grandchild to finish and copy temp file to redirected files
                int status;
                waitpid(pid2, &status, 0);
                FILE *temp_fp = fopen(temp_filename, "r");

                FILE *trunc_fp = NULL, *append_fp = NULL;

                if (command->redirects[1] != NULL) {
                    trunc_fp = fopen(command->redirects[1], "w");
                }

                if (command->redirects[2] != NULL) {
                    append_fp = fopen(command->redirects[2], "a");
                }
                char *fileContent;

                fseek(temp_fp, 0, SEEK_END);
                fileContent = (char *) malloc(sizeof(*fileContent) * ftell(temp_fp));
                fseek(temp_fp, 0, SEEK_SET);

                fread(&fileContent, sizeof(fileContent), 1, temp_fp);
                fclose(temp_fp);

                if (command->redirects[1] != NULL) {
                    fwrite(&fileContent, sizeof(fileContent), 1, trunc_fp);
                    fclose(trunc_fp);
                }
                if (command->redirects[2] != NULL) {
                    fwrite(&fileContent, sizeof(fileContent), 1, append_fp);
                    fclose(append_fp);
                }
                if (command->next) {
                    int parentToChildPipe[2];
                    if (pipe(parentToChildPipe)) {
                        perror("Parent -> Child Pipe failed");
                    }

                    pid_t childPID = fork();
                    if (childPID == 0) {
                        // child will run this part
                        process_command(command->next, parentToChildPipe);
                    } else if (childPID < 0) {
                        /* The fork failed. */
                        perror("Fork failed");
                    } else {
                        // parent will run this part
                        // TODO: what happens if size of file is > 64kb
                        close(parentToChildPipe[0]); // Close read end of parent -> child pipe
                        write(parentToChildPipe[1], &fileContent, sizeof(fileContent));
                        close(parentToChildPipe[1]); // Close write end of parent -> child pipe
                    }

                    free(fileContent);
                    remove(temp_filename);
                    return SUCCESS;
                }
            }
            // we don't need to redirect to multiple files
            // directly run the execv command
        } else {
            return execv_command(command);
        }
    } else {
        if (!command->background) {
            waitpid(pid, NULL, 0); // wait for child process to finish
        }
        return SUCCESS;
    }

    // TODO: your implementation here

    printf("-%s: %s: command not found\n", sysname, command->name);
    return UNKNOWN;
}

int execv_command(struct command_t *command) {
    // increase args size by 2
    command->args = (char **) realloc(
            command->args, sizeof(char *) * (command->arg_count += 2));

    // shift everything forward by 1
    for (int i = command->arg_count - 2; i > 0; --i)
        command->args[i] = command->args[i - 1];

    // set args[0] as a copy of name
    command->args[0] = strdup(command->name);
    // set args[arg_count-1] (last) to NULL
    command->args[command->arg_count - 1] = NULL;

    // execvp(command->name, command->args); // exec+args+path
    char *path = getenv("PATH");
    char *path_tokenizer = strtok(path, ":");
    while (path_tokenizer != NULL) {
        char full_path[strlen(path_tokenizer) + strlen(command->name) + 1];
        combine_path(full_path, path_tokenizer, command->name);
        execv(full_path, command->args);
        path_tokenizer = strtok(NULL, ":");
    }

    // If we reach here, we couldn't find the command on path
    exit(UNKNOWN);
}

void combine_path(char *result, char *directory, char *file) {
    if ((directory == NULL || strlen(directory) == 0) && (file == NULL || strlen(file) == 0)) {
        strcpy(result, "");
    } else if (file == NULL || strlen(file) == 0) {
        strcpy(result, directory);
    } else if (directory == NULL || strlen(directory) == 0) {
        strcpy(result, file);
    } else {
        char directory_separator[] = "/";
        const char *directory_last_character = directory;
        while (*(directory_last_character + 1) != '\0')
            directory_last_character++;
        int directory_contains_separator_at_the_end = strcmp(directory_last_character, directory_separator) == 0;
        strcpy(result, directory);
        if (!directory_contains_separator_at_the_end)
            strcat(result, directory_separator);
        strcat(result, file);
    }
}