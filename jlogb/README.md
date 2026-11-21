# Virtual Memory Manager

Projeto desenvolvido em C para a disciplina de Infraestrutura de Software.

O objetivo é simular o gerenciamento de memória virtual, realizando a tradução de endereços virtuais para físicos por meio de uma tabela de páginas e de uma TLB.

A implementação utilizará os algoritmos FIFO e LRU para substituição de páginas e entradas da TLB, além de threads para simular a busca paralela na TLB.

## Compilação

```bash
make
```

## Execução

```bash
./vm addresses.txt lru fifo
```



## Limpeza

```bash
make clean
```

**Status:** estrutura inicial criada. 