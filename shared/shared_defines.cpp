#include "shared_defines.h"

#include <memory>

void MemorySet(void * dest, u8 value, u64 size) {
    memset(dest, value, size);
}
