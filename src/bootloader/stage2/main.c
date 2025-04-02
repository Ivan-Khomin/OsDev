#include "stdint.h"
#include "stdio.h"

void _cdecl start(uint16_t bootDevice)
{
    puts("Hello world from C!");

    for (;;);
}