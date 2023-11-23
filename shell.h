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
#define nivel2 1
#define nivel3 0

#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct info_job {
    pid_t pid;
    char estado;
    char cmd[COMMAND_LINE_SIZE];
};