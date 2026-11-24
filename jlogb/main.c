
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define PAGE_SIZE 256
#define MAX_ADDRESS 65535



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr,
                "Uso: %s <arquivo> <fifo|lru> <fifo|lru>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    if ((strcmp(argv[2], "fifo") != 0 &&
         strcmp(argv[2], "lru") != 0) ||
        (strcmp(argv[3], "fifo") != 0 &&
         strcmp(argv[3], "lru") != 0)) {
        fprintf(stderr, "Erro: algoritmo invalido.\n");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }

    char line[128];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *end;
        errno = 0;

        long address = strtol(line, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (end == line || *end != '\0' ||
            errno == ERANGE ||
            address < 0 || address > MAX_ADDRESS) {
            fprintf(stderr, "Endereco invalido na linha %d.\n",
                    count + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        int page = (int)(address / PAGE_SIZE);
        int offset = (int)(address % PAGE_SIZE);

        if (count < 5) {
            printf("Endereco: %ld | Pagina: %d | Offset: %d\n",
                   address, page, offset);
        }

        count++;
    }

    if (ferror(file)) {
        perror("Erro durante leitura");
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);

    printf("\nTotal de enderecos processados: %d\n", count);

    return EXIT_SUCCESS;
}
