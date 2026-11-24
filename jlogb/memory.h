#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#define PAGE_SIZE 256
#define VIRTUAL_PAGES 256
#ifndef PHYSICAL_FRAMES
#define PHYSICAL_FRAMES 128
#endif

typedef enum { FIFO, LRU } Policy;
typedef struct {
    signed char data[PHYSICAL_FRAMES][PAGE_SIZE];
    int page_to_frame[VIRTUAL_PAGES];
    int frame_to_page[PHYSICAL_FRAMES];
    unsigned long long loaded_at[PHYSICAL_FRAMES];
    unsigned long long used_at[PHYSICAL_FRAMES];
    int next_free;
    unsigned long long clock;
    unsigned long faults;
    FILE *backing;
    Policy replacement;
} Memory;

int memory_init(Memory *m, const char *backing_path, Policy replacement);
void memory_close(Memory *m);
int memory_resolve(Memory *m, int page, int *evicted_page);
void memory_touch(Memory *m, int frame);
int memory_value(const Memory *m, int frame, int offset);
#endif
