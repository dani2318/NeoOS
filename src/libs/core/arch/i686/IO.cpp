#include "IO.hpp"

#define UNUSED_PORT 0x80

void iowait(){
    Outb(UNUSED_PORT, 0);
}