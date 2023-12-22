#include "shell.h"

static struct info_job jobs_list[N_JOBS];
static char mi_shell[COMMAND_LINE_SIZE];
static int n_job = 0;
static char mensaje[2200];

/**
 * @brief imprime el prompt por pantalla inicial
 * 
 */
void imprimir_prompt()
{
    char *USER = getenv("USER");
    char CWD[_MAX_PATH];

    if (!(getcwd(CWD, sizeof(CWD)) != NULL))
    {
        perror("getcwd() error");
    }

    printf("%s%s:~%s%s%c%s", ORANGE, USER, CREAM, CWD, PROMPT, BASE_COLOR);
}

/**
 * @brief devuelve si un cmd de un job esta o no en background
 * 
 * @param args el cmd
 * @return int estado de la funcion
 */
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

/**
 * @brief parsea la entrada por consola en diferentes
 * argumentos separados por espacios
 * @param args array donde se introduciran los argumentos
 * @param line linea de la que se parseara
 * @return int estado de la funcion
 */
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

/**
 * @brief Comando interno que viene a sustituir cd, redirecciona 
 * el directorio al especificado por consola
 * 
 * @param args cd "directorio" 
 * @return int estado 
 */
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

/**
 * @brief implementacion del comando source
 * 
 * @param args source nombre_archivo
 * @return int estado de la funcion
 */
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

/**
 * @brief imprime todos los jobs que hay actualmente en un
 * formato parecido al de linux
 * 
 * @param args jobs
 * @return int estado de la funcion, no puede dar error
 */
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

/**
 * @brief comando export implementado en nuestro shell
 * 
 * @param args export CLAVE=VALOR
 * @return int estado de la funcion
 */
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

/**
 * @brief funcion fg adaptada a nuestro shell
 * 
 * @param args fg numero_job
 * @return int estado de la funcion
 */
int internal_fg(char **args)
{
    int pos = 0;
    if (!args[1] || args[2])
    {
        printf("internal_fg() -> Sintaxis incorrecta: fg pos\n");
        return EXIT_FAILURE;
    }
    pos = atoi(args[1]);
    if (pos > n_job || pos == 0)
    {
        printf("internal_fg() -> Número de trabajo incorrecto\n");
        return EXIT_FAILURE;
    }
    if (jobs_list[pos].estado == 'D')
        kill(jobs_list[pos].pid, SIGCONT);

    jobs_list[0].pid = jobs_list[pos].pid;
    jobs_list[0].estado = 'E';
    if (jobs_list[pos].cmd[strlen(jobs_list[pos].cmd) - 1] == '&')
    {
        jobs_list[pos].cmd[strlen(jobs_list[pos].cmd) - 1] = '\0';
        printf("%s\n", jobs_list[pos].cmd);
    }
    strcpy(jobs_list[0].cmd, jobs_list[pos].cmd);

    jobs_list_remove(pos);

    printf("%s\n", jobs_list[0].cmd);

    while (jobs_list[0].pid != 0)
    {
        pause();
    }

    return EXIT_SUCCESS;
}

/**
 * @brief implementacion interna del comando bg
 * 
 * @param args bg numero_job
 * @return int estado de la funcion
 */
int internal_bg(char **args)
{
    int pos = 0;
    if (!args[1] || args[2])
    {
        printf("internal_bg() -> Sintaxis incorrecta bg pos\n");
        return EXIT_FAILURE;
    }
    pos = atoi(args[1]);
    if (pos == 0 || pos > n_job)
    {
        printf("internal_bg() -> Posicion de trabajo incorrecta\n");
        return EXIT_FAILURE;
    }
    if (jobs_list[pos].estado == 'E')
    {
        printf("internal_bg() -> Error, trabajo en segundo plano\n");
        return EXIT_FAILURE;
    }
    jobs_list[pos].estado = 'E';
    strcat(jobs_list[pos].cmd, " &\0");

    kill(jobs_list[pos].pid, SIGCONT);
#if DEBUGN6
    fprintf(stderr, "internal_bg() -> Enviando señal SIGCONT a %d (%s)", jobs_list[pos].pid, jobs_list[pos].cmd);
#endif

    return EXIT_SUCCESS;
}

/**
 * @brief mira si el comando introducido por el shell
 * es un comando creado por nosotros, si lo es ejecuta
 * su respectiva funcion internal, sino se ejecuta normal
 * 
 * @param args el nombre del comando args[0]
 * @return int estado de la funcion
 */
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

/**
 * @brief funcion asociada a SIGCHLD
 * 
 * @param signum 
 */
void reaper(int signum)
{
    int ended;
    int status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
#if DEBUGN4
            sprintf(mensaje, "[reaper()→ Proceso hijo %d (%s) finalizado con exit code %d]\n", ended, jobs_list[0].cmd, WEXITSTATUS(status));
            write(2, mensaje, strlen(mensaje));
#endif
        }
        if (WIFSIGNALED(status))
        {
#if DEBUGN4
            sprintf(mensaje, "[reaper()→ Proceso hijo %d (%s) finalizado por señal %d]\n", ended, jobs_list[0].cmd, WTERMSIG(status));
            write(2, mensaje, strlen(mensaje));
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

/**
 * @brief funcion asociada a ctrlc que finaliza un job
 * 
 * @param signum 
 */
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

/**
 * @brief funcion asociada a ctrlz que para un job poniendolo
 * en estado 'D'
 * 
 */
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

/**
 * @brief funcion principal del programa, se dedica a ir ejecutando
 * y parseando cada una de las entradas del programa
 * 
 * @param line la linea introducida por el usuario
 * @return int estado de la funcion
 */
int execute_line(char *line)
{
    char sup_line[COMMAND_LINE_SIZE];
    memset(sup_line, '\0', sizeof(line));
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

        // int redirect = is_output_redirection(args);
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

/**
 * @brief añade un trabajo a job_list
 * 
 * @param pid el pid
 * @param estado su estado actual
 * @param cmd su cmd
 * @return int el estado de la funcion
 */
int jobs_list_add(pid_t pid, char estado, char *cmd)
{
    n_job++;
    if (n_job != N_JOBS)
    {
        jobs_list[n_job].pid = pid;
        jobs_list[n_job].estado = estado;
        strcpy(jobs_list[n_job].cmd, cmd);
        printf("[%d] %d \t %c \t %s\n",
               n_job, pid, estado, cmd);
    }
    else
    {
        printf("jobs_list_add() -> Demasiados trabajos, no se pudo añadir\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief dado un pid devuelve la posicion de ese
 * job en nuestra lista job_list
 * 
 * @param pid el pid del job a buscar
 * @return int posicion del job
 */
int jobs_list_find(pid_t pid)
{
    int pos = 0;
    while (jobs_list[pos].pid != pid)
    {
        pos++;
    }
    return pos;
}

/**
 * @brief dado una posicion de jobs_list elimina
 * ese trabajo del array
 * 
 * @param pos posicion a eliminar
 * @return int estado de la funcion
 */
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

int is_output_redirection(char **args)
{
}

/**
 * @brief inicializacion del programa, asocia señales
 * a las funciones creadas y ejecuta un bucle infinito del que se sale
 * solo con el comando EXIT o ctrl+D
 * 
 * @param argc se esperan dos argumentos
 * @param argv ./shell nombre_shell
 * @return int 
 */
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