#include "cpu.h"

int check_apic(void)
{
    unsigned int eax, unused, edx;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}

void print_cpu_info() {
    unsigned int eax, ebx, ecx, edx;
    printf("===== CPU INFO ====\n\r");
    // Get max supported CPUID leaf
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);
    unsigned int max_basic = eax;

    // Vendor string
    char vendor[13];
    *(uint32_t *)(vendor)     = ebx;
    *(uint32_t *)(vendor + 4) = edx;
    *(uint32_t *)(vendor + 8) = ecx;
    vendor[12] = '\0';
    printf("CPU Vendor: %s\n", vendor);

    if (max_basic < 1) {
        printf("CPUID level 1 not supported\n");
        return;
    }

    // Basic processor info
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);

    unsigned int stepping = eax & 0xF;
    unsigned int model = (eax >> 4) & 0xF;
    unsigned int family = (eax >> 8) & 0xF;
    unsigned int processor_type = (eax >> 12) & 0x3;
    unsigned int extended_model = (eax >> 16) & 0xF;
    unsigned int extended_family = (eax >> 20) & 0xFF;

    // Calculate real family and model
    if (family == 0xF)
        family += extended_family;
    if (family == 0x6 || family == 0xF)
        model += (extended_model << 4);

    printf("CPU Stepping: %u\n", stepping);
    printf("CPU Model: %u\n", model);
    printf("CPU Family: %u\n", family);
    printf("Processor Type: %u\n", processor_type);

    // Feature flags in ECX and EDX
    printf("Features (EDX):\n");
    if (edx & (1 << 0)) printf("  FPU\n");
    if (edx & (1 << 23)) printf("  MMX\n");
    if (edx & (1 << 25)) printf("  SSE\n");
    if (edx & (1 << 26)) printf("  SSE2\n");

    printf("Features (ECX):\n");
    if (ecx & (1 << 0)) printf("  SSE3\n");
    if (ecx & (1 << 9)) printf("  SSSE3\n");
    if (ecx & (1 << 19)) printf("  SSE4.1\n");
    if (ecx & (1 << 20)) printf("  SSE4.2\n");
    if (ecx & (1 << 28)) printf("  AVX\n");

    // Extended functions for brand string
    __get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    unsigned int max_extended = eax;

    if (max_extended >= 0x80000004) {
        char brand[49];
        __get_cpuid(0x80000002, (unsigned int *)(brand + 0), (unsigned int *)(brand + 4), (unsigned int *)(brand + 8), (unsigned int *)(brand + 12));
        __get_cpuid(0x80000003, (unsigned int *)(brand + 16), (unsigned int *)(brand + 20), (unsigned int *)(brand + 24), (unsigned int *)(brand + 28));
        __get_cpuid(0x80000004, (unsigned int *)(brand + 32), (unsigned int *)(brand + 36), (unsigned int *)(brand + 40), (unsigned int *)(brand + 44));
        brand[48] = '\0';
        printf("CPU Brand: %s\n", brand);
    }

    if (check_apic()) {
        printf("APIC supported!\n");
    } else {
        printf("No APIC support.\n");
    }
    printf("===== CPU INFO ====\n\r");
}
