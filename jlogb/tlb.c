#include "tlb.h"
#include <string.h>
#include <pthread.h>
#ifndef USE_TLB_THREADS
#define USE_TLB_THREADS 1
#endif

#if USE_TLB_THREADS
typedef struct {
    const TLBEntry *entry;
    int page;
    int matches;
} LookupTask;

static void *check_entry(void *arg) {
    LookupTask *task = arg;
    task->matches = task->entry->valid && task->entry->page == task->page;
    return NULL;
}
#endif

void tlb_init(TLB *tlb, Policy policy) {
    memset(tlb, 0, sizeof(*tlb));
    tlb->replacement = policy;
}

int tlb_lookup(TLB *tlb, int page, int *frame) {
    int slot = -1;
#if USE_TLB_THREADS
    pthread_t threads[TLB_SIZE];
    LookupTask tasks[TLB_SIZE];
    int started = 0;
    for (int i = 0; i < TLB_SIZE; i++) {
        tasks[i] = (LookupTask){ .entry = &tlb->entries[i], .page = page, .matches = 0 };
        if (pthread_create(&threads[i], NULL, check_entry, &tasks[i]) != 0) break;
        started++;
    }
    for (int i = 0; i < started; i++) pthread_join(threads[i], NULL);
    if (started != TLB_SIZE) return -2;
    for (int i = 0; i < TLB_SIZE; i++)
        if (tasks[i].matches) { slot = i; break; }
#else
    for (int i = 0; i < TLB_SIZE; i++)
        if (tlb->entries[i].valid && tlb->entries[i].page == page) { slot = i; break; }
#endif
    if (slot >= 0) {
        *frame = tlb->entries[slot].frame;
        tlb->entries[slot].last_used = ++tlb->clock;
        tlb->hits++;
    }
    return slot;
}

int tlb_insert(TLB *tlb, int page, int frame) {
    int slot = -1;
    if (tlb->replacement == FIFO) {
        slot = tlb->next_fifo;
        tlb->next_fifo = (tlb->next_fifo + 1) % TLB_SIZE;
    } else {
        for (int i = 0; i < TLB_SIZE; i++) {
            if (!tlb->entries[i].valid) { slot = i; break; }
        }
        if (slot < 0) {
            slot = 0;
            for (int i = 1; i < TLB_SIZE; i++)
                if (tlb->entries[i].last_used < tlb->entries[slot].last_used) slot = i;
        }
    }
    tlb->entries[slot] = (TLBEntry){.valid=1, .page=page, .frame=frame, .last_used=++tlb->clock};
    return slot;
}

void tlb_invalidate(TLB *tlb, int page) {
    for (int i = 0; i < TLB_SIZE; i++)
        if (tlb->entries[i].valid && tlb->entries[i].page == page) tlb->entries[i].valid = 0;
}
