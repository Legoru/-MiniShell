#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define COMMAND_LINE_SIZE 1024
#define MAX_ARGS 32
#define N_JOBS 64
#define _MAX_PATH 260
#define PROMPT '$'

#define CREAM "\033[38;2;245;222;179m"
#define RED "\033[1;31m"
#define ORANGE "\033[1;34m"
#define BASE_COLOR "\033[0m"

#define nivel1 1
char* read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs();
int internal_fg(char **args);
int internal_bg(char **args);

#define DEBUGN2 1
int internal_cd(char **args);
int internal_export(char **args);

#define DEBUGN3 1

#define DEBUGN4 1
void reaper(int signum);
void ctrlc(int signum);

#define DEBUGN5 1
int is_background(char** args);
int jobs_list_add(pid_t pid, char estado, char* cmd);
int jobs_list_remove(int pos);
int jobs_list_find(pid_t pid);

#define DEBUGN6 1
int internal_fg(char** args);
int internal_bg(char** args);
int is_output_redirection(char** args);

struct info_job {
    pid_t pid;
    char estado;
    char cmd[COMMAND_LINE_SIZE];
};