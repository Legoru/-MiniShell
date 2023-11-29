#define COMMAND_LINE_SIZE 1024
#define MAX_ARGS 32
#define N_JOBS 64
#define _MAX_PATH 260
#define PROMPT '$'

#define CREAM "\033[38;2;245;222;179m"
#define RED "\033[1;31m"
#define ORANGE "\033[1;34m"
#define BASE_COLOR "\033[0m"

#define nivel1 0
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

#define nivel2 0
int internal_cd(char **args);
int internal_export(char **args);

#define nivel3 1


#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

struct info_job {
    pid_t pid;
    char estado;
    char cmd[COMMAND_LINE_SIZE];
};