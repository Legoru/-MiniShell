#include "shell.h"

static struct info_job jobs_list[N_JOBS];
static char mi_shell[COMMAND_LINE_SIZE];
static int n_job = 0;
static char mensaje[2200];

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

int is_background(char **args)
{
    int actual = 0;
    while (args[actual] != NULL)
    {
        if (args[actual][strlen(args[actual]) - 1] == '&')
        {
            return 1;
        }
        actual++;
    }
    return 0;
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
        else
        {
            break;
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
#if DEBUGN2
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
    if (args[1] == NULL)
    {
        fprintf(stderr, "internal_source() -> numero argumentos incorrecto\n");
        return EXIT_FAILURE;
    }
    FILE *fp = fopen(args[1], "r");
    if (fp == NULL)
    {
        perror("No se pudo abrir el fichero");
        return EXIT_FAILURE;
    }
    char line[COMMAND_LINE_SIZE];
    while (fgets(line, COMMAND_LINE_SIZE, fp) != NULL)
    {
        printf("%s\n", line);
        if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')
        {
            line[strlen(line) - 1] = '\0';
        }
        else
        {
            strcat(line, "\0");
        }
        fflush(fp);
#if DEBUGN3
        printf("internal_source() -> LINE: %s\n", line);
#endif
        execute_line(line);
    }
    if (fclose(fp) == EXIT_FAILURE)
    {
        perror("No se pudo cerrar el fichero");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int internal_jobs(char **args)
{
    int pos = 0;
    while (pos <= n_job)
    {
        if (jobs_list[pos].pid != 0)
        {
            printf("[%d] %d \t %c \t %s\n",
                   pos, jobs_list[pos].pid, jobs_list[pos].estado, jobs_list[pos].cmd);
        }
        pos++;
    }
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
#if DEBUGN2
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
    if (args[0] == NULL)
    {
        return EXIT_SUCCESS;
    }
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

void reaper(int signum)
{
    int ended;
    int status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status)) {
            #if DEBUGN4
                sprintf(mensaje,"[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n",ended, jobs_list[0].cmd,WEXITSTATUS(status));
                write(2,mensaje,strlen(mensaje));
            #endif
        }
        if (WIFSIGNALED(status)) {
            #if DEBUGN4
                sprintf(mensaje,"[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n",ended, jobs_list[0].cmd, WTERMSIG(status));
                write(2,mensaje,strlen(mensaje));
            #endif
        }
        if (ended == jobs_list[0].pid)
        {
#if DEBUGN4
            sprintf(mensaje, "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n", ended, jobs_list[0].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje));
#endif
            // Reseteamos jobs_list[0]
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'N';
            memset(jobs_list[0].cmd, '\0', sizeof(jobs_list[0].cmd));
        }
        else
        {
            int pos = jobs_list_find(ended);
            sprintf(mensaje, "reaper() -> Proceso %d (%s) finalizado\n", ended, jobs_list[pos].cmd);
            jobs_list_remove(pos);
        }
    }
    signal(SIGCHLD, reaper);
}

void ctrlc(int signum)
{
#if DEBUGN4
    sprintf(mensaje, "\nctrlc()→ Soy el proceso con PID %d (%s) el proceso en foreground es %d (%s)\n",
            getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    write(2, mensaje, strlen(mensaje));
#endif
    if (jobs_list[0].pid > 0)
    {
        if (jobs_list[0].cmd != mi_shell)
        {
#if DEBUGN4
            sprintf(mensaje, "ctrlc()→ Señal SIGTERM enviada por %d a %d\n", getpid(), jobs_list[0].pid);
            write(2, mensaje, strlen(mensaje));
#endif
            kill(jobs_list[0].pid, SIGTERM);
        }
        else
        {
#if DEBUGN4
            sprintf(mensaje, "ctrlc()→ Señal %d no enviada por %d debido a que no hay proceso en foreground", SIGTERM, getpid());
            write(2, mensaje, strlen(mensaje));
#endif
        }
    }
    else
    {
#if DEBUGN4
        fprintf(stderr, "Señal SIGTERM no enviada por %d (%s) debido a que no hay proceso en foreground\n",
                getpid(), mi_shell);
#endif
    }
    signal(SIGINT, ctrlc);
}

void ctrlz()
{
    signal(SIGTSTP, ctrlz);
    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell))
        {
            kill(jobs_list[0].pid, SIGSTOP);
#if DEBUGN5
            sprintf(mensaje, "ctrlz() -> Envida señal %d a %d (%s) por %d (%s)", SIGSTOP,
                    jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
#endif
            jobs_list[0].estado = 'D';
            jobs_list_add(jobs_list[0].pid, jobs_list[0].estado, jobs_list[0].cmd);

            // Reseteamos los datos del proceso en foreground, ya
            // que el proceso que estaba en foreground ahora está detenido
            jobs_list[0].pid = 0;
            jobs_list[0].estado = 'N';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        }
        else
        {
            sprintf(mensaje,
                    "ctrlz() -> Señal %d no enviada al ser el proceso en foreground el minishell", SIGSTOP);
        }
    }
    else
    {
        sprintf(mensaje,
               "ctrlz() -> Señal %d no enviada al no haber proceso en foregorund", SIGSTOP);
    }
}

int execute_line(char *line)
{
    char sup_line[COMMAND_LINE_SIZE];
    memset(sup_line, '\0', sizeof(sup_line));
    strcpy(sup_line, line);
    char *args[MAX_ARGS];
    if (parse_args(args, line) == EXIT_FAILURE)
    {
        perror("Error al parsear argumentos");
        return EXIT_FAILURE;
    }
    if (sizeof(args) < 0)
    {
        printf("No hay argumentos");
        return EXIT_FAILURE;
    }
    if (check_internal(args) == EXIT_SUCCESS)
    {
        return EXIT_SUCCESS;
    }
    // Es un comando externo
    int bg = is_background(args);
    int pid = fork();
    if (pid == 0)
    { // es el hijo
#if DEBUGN3
        printf("execute_line() --> PID hijo: %d (%s)\n", getpid(), sup_line);
#endif
        // manejamos señales
        signal(SIGCHLD, reaper);
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        execvp(args[0], args);
        printf("%s: No se encontro la orden\n", args[0]);
        exit(-1);
    }
    else
    { // es el padre con el pid del hijo
#if DEBUGN3
        printf("execute_line() --> PID padre: %d (%s)\n", getpid(), mi_shell);
#endif
        if (!bg)
        {
            jobs_list[0].pid = pid;
            jobs_list[0].estado = 'E';
            strcpy(jobs_list[0].cmd, sup_line);
            // espera a que llegue una señal
            pause();
        }
        else
        {
            jobs_list_add(pid, 'E', sup_line);
        }
        // ESPERA AHORA EL REAPER
    }
    // RESTEA AHORA REAPER

    return EXIT_SUCCESS;
}

int jobs_list_add(pid_t pid, char estado, char *cmd)
{
    n_job++;
    if (n_job != N_JOBS)
    {
        jobs_list[n_job].pid = pid;
        jobs_list[n_job].estado = estado;
        strcpy(jobs_list[n_job].cmd, cmd);
#if DEBUGN5
        printf("[%d] %d \t %c \t %s\n",
               n_job, pid, estado, cmd);
#endif
    }
    else
    {
        printf("jobs_list_add() -> Demasiados trabajos, no se pudo añadir\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int jobs_list_find(pid_t pid)
{
    int pos = 0;
    while (jobs_list[pos].pid != pid)
    {
        pos++;
    }
    return pos;
}

int jobs_list_remove(int pos)
{
    if (pos <= n_job)
    {
        jobs_list[pos] = jobs_list[n_job];
        n_job--;
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "FORMATO INCORRECTO --> ./nivelN nombre_shell\n");
        return EXIT_FAILURE;
    }
    // Anclamos manejadores a señales
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);
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
            if (feof(stdin))
            {
                printf("\r\nNos fuimos, hasta la proxima\n");
                exit(0);
            }
        }
        if (line != NULL && strlen(line) > 0)
        {
            line[strlen(line) - 1] = '\0';
            execute_line(line);
        }
        free(line);
    }
}