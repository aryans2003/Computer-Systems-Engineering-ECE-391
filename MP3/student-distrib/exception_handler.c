#include "exception_handler.h"

void exc_handler(uint32_t id, struct registers regs, uint32_t error_code) {
    if (id != SYSTEM_CALLS) {
        if(id < NUM_EXCEPTIONS) {
            printf("Exception raised %s\n", os_exceptions[id]);
        }
    }
    halt(255);
}
