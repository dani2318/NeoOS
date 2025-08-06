#pragma once
#include <stdbool.h>

typedef struct{
    const char* Name;
    bool (*Probe)();
    void (*Initialize)(uint8_t offsetPic1, uint8_t offsetPic2, bool autoEOI);
    void (*Disable)();
    void (*SendEOI)(int irq);
    void (*Mask)(int irq);
    void (*Unmask)(int irq);
} PICDriver;
