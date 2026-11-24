#include "memory.h"
#include "pagetable.h"
#include "tlb.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int parse_policy(const char *s, Policy *p) {
    if (strcmp(s, "fifo") == 0) { *p = FIFO; return 0; }
    if (strcmp(s, "lru") == 0) { *p = LRU; return 0; }
    return -1;
}

static int parse_address(const char *line, int *address) {
    const unsigned char *p = (const unsigned char *)line;
    while (*p && isspace(*p)) p++;
    if (!isdigit(*p)) return -1;
    errno = 0;
    char *end;
    long n = strtol((const char *)p, &end, 10);
    if (errno == ERANGE || n < 0 || n > 65535) return -1;
    while (*end && isspace((unsigned char)*end)) end++;
    if (*end) return -1;
    *address = (int)n;
    return 0;
}

int main(int argc, char **argv) {
    Policy page_policy, tlb_policy;
    if (argc != 4 || parse_policy(argc > 2 ? argv[2] : "", &page_policy) ||
        parse_policy(argc > 3 ? argv[3] : "", &tlb_policy)) {
        fprintf(stderr, "Uso: %s <enderecos.txt> <fifo|lru> <fifo|lru>\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *input = fopen(argv[1], "r");
    if (!input) { perror("Erro ao abrir arquivo de enderecos"); return EXIT_FAILURE; }
    Memory memory;
    if (memory_init(&memory, "BACKING_STORE.bin", page_policy)) {
        fprintf(stderr, "Erro ao abrir BACKING_STORE.bin (esperados 65536 bytes)\n");
        fclose(input);
        return EXIT_FAILURE;
    }
    TLB tlb;
    tlb_init(&tlb, tlb_policy);
    /* Gera a saída solicitada pelo enunciado. */
    FILE *output = fopen("correct.txt", "w"); // Saída do programa
    if (!output) {
        perror("Erro ao criar correct.txt");
        memory_close(&memory); fclose(input);
        return EXIT_FAILURE;
    }
    char line[128];
    unsigned long total = 0;
    int failed = 0;
    while (fgets(line, sizeof(line), input)) {
        if (!strchr(line, '\n') && !feof(input)) {
            int c;
            while ((c = fgetc(input)) != '\n' && c != EOF) {}
            fprintf(stderr, "Linha %lu excessivamente longa\n", total + 1);
            failed = 1; break;
        }
        int address;
        if (parse_address(line, &address)) {
            fprintf(stderr, "Endereco invalido na linha %lu\n", total + 1);
            failed = 1; break;
        }
        int page = address / PAGE_SIZE, offset = address % PAGE_SIZE; // assume PAGE_SIZE = 256
        int frame;
        int tlb_slot = tlb_lookup(&tlb, page, &frame);
        if (tlb_slot == -2) {
            fprintf(stderr, "Erro ao criar threads para busca da TLB\n");
            failed = 1; break;
        }
        if (tlb_slot >= 0) {
            memory_touch(&memory, frame);
        } else {
            
            int evicted = -1;
            frame = memory_resolve(&memory, page, &evicted);
            if (frame < 0) { fprintf(stderr, "Erro ao carregar pagina %d\n", page); failed = 1; break; }
            if (evicted >= 0) tlb_invalidate(&tlb, evicted);
            tlb_slot = tlb_insert(&tlb, page, frame);
        }
        fprintf(output, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                address, tlb_slot, frame * PAGE_SIZE + offset,
                memory_value(&memory, frame, offset));
        total++;
    }


    if (ferror(input)) { perror("Erro de leitura"); failed = 1; } // ferror = verifica se ocorreu algum erro de leitura no arquivo / perror = imprime a mensagem de erro correspondente ao valor atual da variável errno
    if (!failed && total == 0) { fprintf(stderr, "Arquivo de enderecos vazio\n"); failed = 1; }
    if (!failed) {
        fprintf(output, "Number of Translated Addresses = %lu\n", total);
        fprintf(output, "Page Faults = %lu\n", memory.faults);
        fprintf(output, "Page Fault Rate = %.3f\n", (double)memory.faults / total);
        fprintf(output, "TLB Hits = %lu\n", tlb.hits);
        fprintf(output, "TLB Hit Rate = %.3f\n", (double)tlb.hits / total);
    }


    if (fclose(output) != 0) failed = 1;
    memory_close(&memory);
    fclose(input);
    if (failed) { remove("correct.txt"); return EXIT_FAILURE; }
    printf("Processados %lu enderecos; page faults: %lu; TLB hits: %lu. Saida: correct.txt\n",
           total, memory.faults, tlb.hits);
    return EXIT_SUCCESS;
}
