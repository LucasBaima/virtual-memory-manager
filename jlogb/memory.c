#include "memory.h"
#include <string.h>

int memory_init(Memory *m, const char *path, Policy policy) {
    memset(m, 0, sizeof(*m));
    for (int i = 0; i < VIRTUAL_PAGES; i++) m->page_to_frame[i] = -1;
    for (int i = 0; i < PHYSICAL_FRAMES; i++) m->frame_to_page[i] = -1;
    m->replacement = policy;
    m->backing = fopen(path, "rb");
    if (!m->backing) return -1;
    if (fseek(m->backing, 0, SEEK_END) != 0 || ftell(m->backing) != VIRTUAL_PAGES * PAGE_SIZE ||
        fseek(m->backing, 0, SEEK_SET) != 0) {
        fclose(m->backing);
        m->backing = NULL;
        return -1;
    }
    return 0;
}

void memory_close(Memory *m) {
    if (m->backing) fclose(m->backing);
    m->backing = NULL;
}

void memory_touch(Memory *m, int frame) { m->used_at[frame] = ++m->clock; }

int memory_resolve(Memory *m, int page, int *evicted_page) {
    *evicted_page = -1;
    if (m->page_to_frame[page] >= 0) {
        int frame = m->page_to_frame[page];
        memory_touch(m, frame);
        return frame;
    }
    int frame;
    if (m->next_free < PHYSICAL_FRAMES) {
        frame = m->next_free++;
    } else {
        frame = 0;
        for (int i = 1; i < PHYSICAL_FRAMES; i++) {
            unsigned long long age_i = m->replacement == FIFO ? m->loaded_at[i] : m->used_at[i];
            unsigned long long age_best = m->replacement == FIFO ? m->loaded_at[frame] : m->used_at[frame];
            if (age_i < age_best) frame = i;
        }
        *evicted_page = m->frame_to_page[frame];
    }
    unsigned char buf[PAGE_SIZE];
    if (fseek(m->backing, page * PAGE_SIZE, SEEK_SET) != 0 ||
        fread(buf, 1, PAGE_SIZE, m->backing) != PAGE_SIZE) return -1;
    if (*evicted_page >= 0) m->page_to_frame[*evicted_page] = -1;
    for (int i = 0; i < PAGE_SIZE; i++) m->data[frame][i] = (signed char)buf[i];
    m->frame_to_page[frame] = page;
    m->page_to_frame[page] = frame;
    m->loaded_at[frame] = ++m->clock;
    m->used_at[frame] = m->clock;
    m->faults++;
    return frame;
}

int memory_value(const Memory *m, int frame, int offset) {
    return m->data[frame][offset];
}
