# Laboratorio # 2: API de Procesos

## Nombres, correos y números de documento

* Esteban Andres Castaño Gallo, **esteban.castano1@udea.edu.co**, 1001967876
* Jorge Luis Rodriguez, **jorge.rodriguezj@udea.edu.co**, 1027941053

---

## Documentación del código

### print_error()
Imprime el mensaje de error estándar "An error has occurred\n" a stderr. Es el único mensaje de error usado en todo el programa, tal como lo exige el enunciado.

### init_path() / free_path()
init_path inicializa el search path del shell con /bin como único directorio al arrancar. free_path libera la memoria dinámica del path actual antes de sobrescribirlo.

### find_executable(cmd)
Recorre los directorios del search path actual y usa access() para verificar si el ejecutable existe y tiene permisos de ejecución. Retorna la ruta completa si lo encuentra, o NULL si no.

### builtin_exit(args)
Implementa el comando integrado exit. Verifica que no se pasen argumentos y llama a exit(0).

### builtin_chd(args)
Implementa el comando integrado cd. Requiere exactamente un argumento y usa chdir() para cambiar el directorio de trabajo.

### builtin_path(args)
Sobrescribe el search path actual con los directorios pasados como argumentos. Si no se pasan argumentos, el path queda vacío y el shell no puede ejecutar comandos externos.

### handle_redirection(args, arg_count, outfile)
Detecta el operador > dentro de los argumentos del comando. Valida que haya exactamente un archivo destino, corta el arreglo de argumentos antes del > y retorna el nombre del archivo de salida. Si hay más de un > o el formato es incorrecto, imprime el error estándar.

### split_parallel(line, cmds, max)
Separa la línea de entrada por el operador &, limpia espacios al inicio y final de cada subcadena, y almacena cada comando en el arreglo cmds[]. Retorna la cantidad de comandos encontrados.

### execute_command(args, redirect)
Verifica si el comando es un built-in y lo ejecuta directamente. Si es un comando externo, hace fork(), aplica la redirección con dup2() si corresponde, y ejecuta el programa con execv(). El proceso padre guarda el PID del hijo para esperar después.

### parse_and_run(line)
Función principal de procesamiento. Recibe la línea leída, la pasa por split_parallel para detectar comandos paralelos, parsea cada comando con strsep, detecta redirección, lanza todos los procesos con fork y al final espera a todos con waitpid.

### main(argc, argv)
Punto de entrada. Valida los argumentos, determina si el shell corre en modo interactivo o batch, inicializa el path e inicia el loop principal de lectura con getline.

---

## Problemas presentados durante el desarrollo de la práctica y sus soluciones

**Problema 1 — El prompt 'wish>' no aparecía al iniciar (Esteban)**
Al ejecutar el shell en modo interactivo, el prompt 'wish>' no se mostraba hasta que el usuario presionaba Enter. El problema era que printf almacena la salida en un buffer y no la envía a pantalla de inmediato. La solución fue agregar fflush(stdout) justo después del printf("wish> "), lo que fuerza el vaciado del buffer y hace que el prompt aparezca de inmediato.

**Problema 2 — Dudas en la implementación de redirección y paralelos (Jorge)**
Durante el desarrollo de handle_redirection y split_parallel surgieron dudas sobre cómo cortar correctamente el arreglo de argumentos al encontrar > y cómo manejar los espacios sobrantes al separar por &. Se consultaron referencias y se ajustó la lógica de punteros para garantizar que los argumentos quedaran bien delimitados antes de pasarlos a execv.

---

## Pruebas realizadas a los programas que verificaron su funcionalidad

1. **Modo interactivo básico:** se ejecutó './wish' y se corrieron 'ls' y 'ls -la', verificando que el shell mostraba el prompt y ejecutaba comandos correctamente.
   
   <img width="488" height="246" alt="image" src="https://github.com/user-attachments/assets/4ba51603-e412-4ae2-9c9f-1ef4a2cdfc9a" />

2. **Comando inexistente:** se ingresó 'comandofalso' y el shell imprimió el error estándar sin cerrarse.
   
   <img width="412" height="66" alt="image" src="https://github.com/user-attachments/assets/ef041f22-69a6-40bb-891e-5d516a3f307e" />

3. **Built-in cd:** se cambió al directorio '/tmp' y se verificó con 'ls' que el directorio activo cambió.

   <img width="1024" height="104" alt="image" src="https://github.com/user-attachments/assets/bcca850f-aff0-4cdf-b6dd-2bb52aa8c7d6" />

4. **Built-in path:** se actualizó el path con '/bin /usr/bin' y se comprobó que los comandos seguían funcionando.

   <img width="1017" height="103" alt="image" src="https://github.com/user-attachments/assets/722946aa-df63-4020-852b-b4171830f24a" />

5. **Redirección >:** se ejecutó 'ls > salida.txt' y se verificó con 'cat salida.txt' que la salida se guardó en el archivo.

   <img width="1029" height="263" alt="image" src="https://github.com/user-attachments/assets/9bf2e98c-eb1c-4033-8331-cf0b7214448e" />

6. **Comandos paralelos &:** se ejecutó 'ls & pwd & echo hola' y los tres comandos corrieron en paralelo, retornando al prompt solo cuando todos terminaron.

    <img width="483" height="103" alt="image" src="https://github.com/user-attachments/assets/1417a14b-8ed6-4b4f-9ace-ab8dabd0f1cc" />

7. **Modo batch:** se ejecutó './wish batch.txt' y el shell procesó los comandos del archivo sin mostrar el prompt.

    <img width="527" height="63" alt="image" src="https://github.com/user-attachments/assets/62732da0-07d8-47be-baa6-221c4e171794" />

8. **Más de 2 argumentos:** se ejecutó './wish arg1 arg2' y el shell imprimió el error estándar y terminó con código 1.

    <img width="522" height="62" alt="image" src="https://github.com/user-attachments/assets/b3a5aa69-4e19-4a79-960e-8701a6ef8c93" />


---

## Enlace al vídeo

[Por agregar]

---

## Manifiesto de transparencia

Durante el desarrollo de esta práctica se hizo uso de IA generativa de forma puntual y responsable en los siguientes casos:

- **Esteban:** consultó IA para resolver el problema del prompt 'wish>' que no aparecía en pantalla sin presionar Enter. La IA orientó hacia el uso de fflush(stdout) como solución. El resto del desarrollo fue realizado de forma autónoma.
- **Jorge:** consultó IA para aclarar dudas puntuales sobre la implementación de la redirección con dup2() y el manejo de punteros al separar comandos paralelos con strsep. La lógica, estructura y escritura del código fueron realizadas de forma propia.

En ningún caso se usó IA para generar bloques completos de código sin comprenderlos.
