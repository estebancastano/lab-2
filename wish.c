#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Mensaje de error único exigido por la guía
char error_message[30] = "An error has occurred\n";

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int interactive = 1;
    
    // Paso 3: Gestión de Paths inicial (solo /bin)
    // Usamos memoria dinámica para que el comando 'path' pueda modificarlo luego
    char **paths = malloc(2 * sizeof(char *));
    paths[0] = strdup("/bin");
    paths[1] = NULL;
    int path_count = 1;

    // Paso 1 & 4: Validación de argumentos y Modo Batch
    if (argc > 2) {
        print_error();
        exit(1);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            print_error();
            exit(1);
        }
        interactive = 0; // En modo batch no hay prompt
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // Paso 1: Loop principal
    while (1) {
        if (interactive) {
            printf("wish> ");
            fflush(stdout); // CRÍTICO: Fuerza la aparición del prompt inmediatamente
        }

        nread = getline(&line, &len, input);

        // Detectar EOF (Ctrl+D o final de archivo batch)
        if (nread == -1) {
            free(line);
            for (int i = 0; i < path_count; i++) free(paths[i]);
            free(paths);
            exit(0);
        }

        // Limpiar el salto de línea (\n)
        if (line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        // Paso 2: Parsing (Trocear la línea en comandos y argumentos)
        char *args[64];
        int arg_count = 0;
        char *temp_line = line;
        char *token;

        while ((token = strsep(&temp_line, " \t")) != NULL) {
            if (strlen(token) > 0) {
                args[arg_count++] = token;
            }
        }
        args[arg_count] = NULL;

        if (arg_count == 0) continue; // Si la línea está vacía, volver al inicio

        // Paso 3: Comandos Built-in
        if (strcmp(args[0], "exit") == 0) {
            if (arg_count > 1) {
                print_error();
            } else {
                free(line);
                for (int i = 0; i < path_count; i++) free(paths[i]);
                free(paths);
                exit(0);
            }
        } 
        else if (strcmp(args[0], "cd") == 0) {
            if (arg_count != 2) {
                print_error();
            } else {
                if (chdir(args[1]) != 0) {
                    print_error();
                }
            }
        } 
        else if (strcmp(args[0], "path") == 0) {
            // Liberar paths anteriores
            for (int i = 0; i < path_count; i++) free(paths[i]);
            free(paths);

            // Asignar nuevos paths desde los argumentos
            paths = malloc((arg_count) * sizeof(char *));
            for (int i = 1; i < arg_count; i++) {
                paths[i-1] = strdup(args[i]);
            }
            path_count = arg_count - 1;
            paths[path_count] = NULL;
        } 
        else {
            // Paso 2: Ejecución de comandos externos
            char found_path[256];
            int executable_found = 0;

            // Buscar el ejecutable en cada directorio del path actual
            for (int i = 0; i < path_count; i++) {
                snprintf(found_path, sizeof(found_path), "%s/%s", paths[i], args[0]);
                if (access(found_path, X_OK) == 0) {
                    executable_found = 1;
                    break;
                }
            }

            if (executable_found) {
                pid_t pid = fork();
                if (pid < 0) {
                    print_error();
                } else if (pid == 0) {
                    // PROCESO HIJO
                    execv(found_path, args);
                    // Si execv retorna, hubo un error
                    print_error();
                    exit(1);
                } else {
                    // PROCESO PADRE
                    wait(NULL);
                }
            } else {
                // No se encontró en ningún path
                print_error();
            }
        }
    }

    return 0;
}