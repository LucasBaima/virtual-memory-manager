#include "pagetable.h"


int pagetable_lookup(const Memory *m, int page) {
    return (page >= 0 && page < VIRTUAL_PAGES) ? m->page_to_frame[page] : -1;
}
