#include "shell.h"

static struct info_job jobs_list[N_JOBS];
static char mi_shell[COMMAND_LINE_SIZE];

void imprimir_prompt()
{
    char *USER = getenv("USER");
    // char *HOME = getenv("HOME");
    char CWD[_MAX_PATH];

    if (!(getcwd(CWD, sizeof(CWD)) != NULL))
    {
        perror("getcwd() error");
    }
    // TODO: LIMPIAR HOME

    printf("%s%s:~%s%s%c%s", ORANGE, USER, CREAM, CWD, PROMPT, BASE_COLOR);
}

// cuando devuelve error?
int parse_args(char **args, char *line)
{
    int num_tokens = 0;
    char *del = " \t\n\r";
    char *token = strtok(line, del);
    while (token != NULL)
    {
        if (token[0] != '#')
        {
#if nivel1
            printf("%s - %d\n", token, num_tokens);
#endif
            args[num_tokens] = token;
            num_tokens++;
        }
        token = strtok(NULL, del);
    }
    args[num_tokens] = NULL;
#if nivel1
    printf("Total tokens: %d\n", num_tokens);
#endif
    return EXIT_SUCCESS;
}

int internal_cd(char **args)
{
    if (args[1] == NULL)
    {
        if (chdir(getenv("HOME")) != 0)
        {
            perror("internal_cd home error");
        }
    }
    else if (args[2] == NULL)
    {
        if (chdir(args[1]) != 0)
        {
            perror("internal_cd error");
        }
    }
    else
    {
        // TODO: implementar cd avanzado
        if (strchr(args[1], '\'') || strchr(args[1], '"'))
        {
        }
    }
#if nivel2
    char CWD[_MAX_PATH];
    if (!(getcwd(CWD, sizeof(CWD)) != NULL))
    {
        perror("getcwd() error");
    }
    printf("internal_cd() --> %s\n", CWD);
#endif
    return EXIT_SUCCESS;
}

int internal_bg(char **args)
{
    printf("LLAMASTE A BG\n");
    return EXIT_SUCCESS;
}

int internal_fg(char **args)
{
    printf("LLAMASTE A FG\n");
    return EXIT_SUCCESS;
}

int internal_source(char **args)
{
    printf("LLAMASTE A SOURCE\n");
    return EXIT_SUCCESS;
}

int internal_jobs(char **args)
{
    printf("LLAMASTE A JOBS\n");
    return EXIT_SUCCESS;
}

int internal_export(char **args)
{
    if (args[2] != NULL)
    {
        fprintf(stderr, "internal_export() -> numero argumentos incorrecto\n");
        return EXIT_FAILURE;
    }
    char *key;
    char *new_value;
    char *del = "=";
    char *token = strtok(args[1], del);
    if (token == NULL)
    {
        fprintf(stderr, "internal_export() -> no hay clave\n");
        return EXIT_FAILURE;
    }
    key = token;
    new_value = strtok(NULL, "");
    if (new_value == NULL)
    {
        fprintf(stderr, "internal_export() -> valor erroneo\n");
        return EXIT_FAILURE;
    }
#if nivel2
    char *last_value = getenv(key);
    printf("internal_export() -> nombre: %s\n", key);
    printf("internal_export() -> antiguo valor: %s\n", last_value);
    printf("internal_export() -> nuevo valor: %s\n", new_value);
#endif
    if (setenv(key, new_value, 1) != 0)
    {
        perror("internal_export() -> setenv");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int check_internal(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        internal_cd(args);
    }
    else if (strcmp(args[0], "export") == 0)
    {
        internal_export(args);
    }
    else if (strcmp(args[0], "source") == 0)
    {
        internal_source(args);
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        internal_fg(args);
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        internal_bg(args);
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        printf("Hasta la proxima!\n");
        exit(0);
    }
    else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int execute_line(char *line)
{
    char *sup_line = line;
    char *args[MAX_ARGS];
    if (parse_args(args, line) == EXIT_FAILURE)
    {
        perror("Error al parsear argumentos");
        return EXIT_FAILURE;
    }
    if (sizeof(args) < 0)
    {
        perror("No hay argumentos");
        return EXIT_FAILURE;
    }
    if (check_internal(args) == EXIT_SUCCESS)
    {
        return EXIT_SUCCESS;
    }
    // Es un comando externo
    int pid = fork();
    if (pid == 0)
    { // es el hijo
#if nivel3
        printf("execute_line() --> PID hijo: %d (%s)\n", getpid(), args[0]);
#endif
        execvp(args[0], args);
        printf("%s: No se encontro la orden\n", args[0]);
        exit(-1);
    }
    else
    { // es el padre con el pid del hijo
#if nivel3
        printf("execute_line() --> PID padre: %d (%s)\n", getpid(), mi_shell);
#endif
        int status;
        jobs_list[0].pid = pid;
        jobs_list[0].estado = 'E';
        strcpy(jobs_list[0].cmd, sup_line);

        int child_pid = wait(&status);

        if (WIFEXITED(status))
        {
            #if nivel3
                printf("execute_line() --> Proceso hijo %d (%s) finalizado con exit(), status: %d\n",child_pid, args[0], status);
            #endif
        }
        else if (WIFSTOPPED(status))
        {
            #if nivel3
                printf("execute_line() --> Proceso hijo %d (%s) fue bloqueado, status: %d\n",child_pid, args[0], status);
            #endif
        }

        //Reseteamos jobs_list[0]
        jobs_list[0].pid = 0;
        jobs_list[0].estado = 'N';
        memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "FORMATO INCORRECTO --> ./nivelN nombre_shell\n");
        return EXIT_FAILURE;
    }
    // Copiamos valor dado por consola a mi_shell actualizamos job_list[0]
    strcpy(mi_shell, argv[1]);
    jobs_list[0].pid = 0;
    jobs_list[0].estado = 'N';
    memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
    while (1)
    {
        char *line = malloc(COMMAND_LINE_SIZE);
        imprimir_prompt();
        if (fgets(line, COMMAND_LINE_SIZE, stdin) == NULL)
        {
            printf("\n%sNO FUIMOOO%s", ORANGE, BASE_COLOR);
            exit(0);
        }
        if (line != NULL)
        {
            line[strlen(line) - 1] = '\0';
            execute_line(line);
        }
        free(line);
    }
}