#include <stdint.h>
#include "tss.h"
#include "klib/string.h"

uint8_t double_fault_stack[4096];

void init_tss() {
    memset(&tss, 0, sizeof(tss));
    tss.ist[0] = (uint64_t)&double_fault_stack[sizeof(double_fault_stack)]; 
}

