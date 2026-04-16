#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>  // Jorge: necesario para open()

char error_message[30] = "An error has occurred\n";

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// JORGE: Detecta '>' en args, valida que haya exactamente un archivo destino, y corta args antes del '>'.
// Retorna 1 si hay redirección, 0 si no, -1 si error.

int handle_redirection(char **args, int arg_count, char **outfile) {
    *outfile = NULL;
    int redir_index = -1;

    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (redir_index != -1) { print_error(); return -1; } // dos '>' = error
            redir_index = i;
        }
    }

    if (redir_index == -1) return 0; // sin redirección

    // Error: nada después de '>', o más de un archivo destino
    if (redir_index == arg_count - 1 || redir_index < arg_count - 2) {
        print_error();
        return -1;
    }

    *outfile = args[redir_index + 1]; // archivo de salida
    args[redir_index] = NULL;         // cortar args antes del '>'
    return 1;
}

// JORGE: Separa la línea por '&' y guarda cada subcadena en cmds[]. Retorna la cantidad de comandos encontrados.

int split_parallel(char *line, char **cmds, int max) {
    int count = 0;
    char *token;
    while ((token = strsep(&line, "&")) != NULL) {
        while (*token == ' ' || *token == '\t') token++;
        char *end = token + strlen(token) - 1;
        while (end >= token && (*end == ' ' || *end == '\t')) { *end = '\0'; end--; }
        if (strlen(token) > 0 && count < max)
            cmds[count++] = token;
    }
    return count;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int interactive = 1;

    // Path inicial: solo /bin
    char **paths = malloc(2 * sizeof(char *));
    paths[0] = strdup("/bin");
    paths[1] = NULL;
    int path_count = 1;

    // Validar argumentos y abrir batch file si aplica
    if (argc > 2) { print_error(); exit(1); }
    else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) { print_error(); exit(1); }
        interactive = 0;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        if (interactive) { printf("wish> "); fflush(stdout); }

        nread = getline(&line, &len, input);
        if (nread == -1) {
            free(line);
            for (int i = 0; i < path_count; i++) free(paths[i]);
            free(paths);
            exit(0);
        }

        if (line[nread - 1] == '\n') line[nread - 1] = '\0';


        // JORGE: separar la línea en comandos paralelos por '&'

        char *line_copy = strdup(line);
        char *cmds[64];
        int cmd_count = split_parallel(line_copy, cmds, 64);

        pid_t pids[64];
        int pid_count = 0;

        for (int c = 0; c < cmd_count; c++) {

            // Parsear cada comando en tokens
            char *args[64];
            int arg_count = 0;
            char *temp = cmds[c];
            char *token;

            while ((token = strsep(&temp, " \t")) != NULL) {
                if (strlen(token) > 0)
                    args[arg_count++] = token;
            }
            args[arg_count] = NULL;

            if (arg_count == 0) continue;

            // Built-ins
            if (strcmp(args[0], "exit") == 0) {
                if (arg_count > 1) print_error();
                else {
                    free(line); free(line_copy);
                    for (int i = 0; i < path_count; i++) free(paths[i]);
                    free(paths);
                    exit(0);
                }
                continue;
            }
            else if (strcmp(args[0], "cd") == 0) {
                if (arg_count != 2) print_error();
                else if (chdir(args[1]) != 0) print_error();
                continue;
            }
            else if (strcmp(args[0], "path") == 0) {
                for (int i = 0; i < path_count; i++) free(paths[i]);
                free(paths);
                paths = malloc((arg_count) * sizeof(char *));
                for (int i = 1; i < arg_count; i++) paths[i-1] = strdup(args[i]);
                path_count = arg_count - 1;
                paths[path_count] = NULL;
                continue;
            }

            // JORGE: detectar redirección antes de ejecutar

            char *outfile = NULL;
            int redir = handle_redirection(args, arg_count, &outfile);
            if (redir == -1) continue;

            // Buscar ejecutable en el path
            char found_path[256];
            int found = 0;
            for (int i = 0; i < path_count; i++) {
                snprintf(found_path, sizeof(found_path), "%s/%s", paths[i], args[0]);
                if (access(found_path, X_OK) == 0) { found = 1; break; }
            }

            if (!found) { print_error(); continue; }

            pid_t pid = fork();
            if (pid < 0) { print_error(); continue; }

            if (pid == 0) {

                // JORGE: aplicar redirección stdout+stderr al archivo

                if (outfile != NULL) {
                    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) { print_error(); exit(1); }
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
                execv(found_path, args);
                print_error();
                exit(1);
            } else {

                // JORGE: guardar PID para esperar al final del ciclo
                
                pids[pid_count++] = pid;
            }
        }

        // JORGE: esperar a todos los procesos paralelos lanzados
        for (int i = 0; i < pid_count; i++)
            waitpid(pids[i], NULL, 0);

        free(line_copy);
    }

    return 0;
}
