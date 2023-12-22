#include "shell.h"

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
    printf("LLAMASTE A CD\n");
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
    printf("LLAMASTE A EXPORT\n");
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
    return EXIT_SUCCESS;
}

int execute_line(char *line)
{
    char *args[MAX_ARGS];
    if (parse_args(args, line) == -1)
    {
        perror("Error al parsear argumentos");
        return -1;
    }
    if (sizeof(args) < 0)
    {
        perror("No hay argumentos");
        return -1;
    }
    check_internal(args);
    return 1;
}

int main()
{
    while (1)
    {
        char *line = malloc(COMMAND_LINE_SIZE);
        if (fgets(line, COMMAND_LINE_SIZE, stdin) == NULL)
        {
            if (feof(stdin))
            {
                printf("\r\nNos fuimos, hasta la proxima\n");
                exit(0);
            }
            else
            {
                perror("Error al leer la línea");
                exit(EXIT_FAILURE);
            }
        }
        if (line != NULL)
        {
            line[strlen(line) - 1] = '\0';
            execute_line(line);
        }
        free(line);
    }
}