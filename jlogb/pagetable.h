#ifndef PAGETABLE_H
#define PAGETABLE_H
#include "memory.h"
int pagetable_lookup(const Memory *m, int page);
#endif
