#ifndef TLB_H
#define TLB_H
#include "memory.h"
#define TLB_SIZE 16

typedef struct {
    int valid;
    int page;
    int frame;
    unsigned long long last_used;
} TLBEntry;
typedef struct {
    TLBEntry entries[TLB_SIZE];
    unsigned long long clock;
    unsigned long hits;
    Policy replacement;
    int next_fifo;
    int next_empty;
} TLB;
void tlb_init(TLB *tlb, Policy policy);
int tlb_lookup(TLB *tlb, int page, int *frame);
int tlb_insert(TLB *tlb, int page, int frame);
void tlb_invalidate(TLB *tlb, int page);
#endif
