#include <stdio.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "platform.h"

void showcase_of_working_proces_in_background(void) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    unsigned long long uptime;
    unsigned long long total_mb = 0;
    unsigned long long available_mb = 0;
    char os_name[128];
    char kernel[128];
    char arch[64];
    char cpu[128];

    platform_os_name(os_name, sizeof(os_name));
    platform_kernel_version(kernel, sizeof(kernel));
    platform_architecture(arch, sizeof(arch));
    platform_cpu_model(cpu, sizeof(cpu));

    setColor();
    printf("\n");
    printf(" ██████████████████████████████████████████████████████████████████████████████████████████\n");
    printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf(" █▒                                                                                     ▒█▒\n");
    printf(" █▒          ██████╗     ██████║ ║██████████║     ╔═╝██████╚═╗   ████████╗              ▒█▒\n");
    printf(" █▒         ██╔════╝    ██╔═╗██║ ╚═══╗██╔═══╝     ║██      ██║   ██╚═════╗              ▒█▒\n");
    printf(" █▒   █████╗██║        ██╔╝ ║██║     ║██║         ║██      ██║   ████████║              ▒█▒\n");
    printf(" █▒   ╚════╝██║       █████████║     ║██║         ║██      ██║         ██║              ▒█▒\n");
    printf(" █▒         ╚██████╗ ██╔════╗██║     ║██║         ╚═╗██████╔═╝  ║████████║              ▒█▒\n");
    printf(" █▒          ╚═════╝ ╚═╝    ╚══╝     ╚══╝           ╚══════╝    ╚════════╝              ▒█▒\n");
    printf(" █▒                                                                                     ▒█▒\n");
    printf(" █████████████████████████████████████████████████████████████████████████████████████████▒\n");
    printf(" █▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
    printf(" █▒                                                                                     ▒█▒\n");
    printf(" █▒    ██╗          ██████║  ██╗   ██║  ██████║  ████║ ║██║ ║██████╚╗ ██████║ ██████═╗  ▒█▒\n");
    printf(" █▒    ██║         ██╔═╗██║  ██║  ██║  ██╔═╗██║  ██║██║║██║ ║██   ██║ ██║║    ██║ ║██║  ▒█▒\n");
    printf(" █▒    ██║        ██╔╝ ║██║  ██║ ██║  ██╔╝ ║██║  ██║║██║██║ ║██   ██║ ██████║ ██████═╗  ▒█▒\n");
    printf(" █▒    ██║       █████████║  ██║██║  █████████║  ██║ ║████║ ║██   ██║ ██║║    ██║ ║██╚╗ ▒█▒\n");
    printf(" █▒    ███████╗ ██╔════╗██║  ████║  ██╔════╗██║  ██║   ║██║ ║██████╔╝ ██████║ ██║  ║██║ ▒█▒\n");
    printf(" █▒    ╚══════╝ ╚═╝    ╚═╝   ╚═══╝  ╚═╝    ╚══╝  ╚═╝   ╚══╝ ╚══════╝ ╚══════╝ ╚═╝  ╚══╝ ▒█▒\n");
    printf(" █▒                                                                                     ▒█▒\n");
    printf(" █▒    ██╗  ██╗ ███████╗ ██████╗  ███╗   ██╗ ███████╗ ██╗                               ▒█▒\n");
    printf(" █▒    ██╔╝ ██║ ██╔════╝ ██╔══██║ ████╗  ██║ ██╔════╝ ██║                               ▒█▒\n");
    printf(" █▒    █████╔═╝ █████╗   █████╔═╝ ██╔██╗ ██║ █████╗   ██║                               ▒█▒\n");
    printf(" █▒    ██║ ██╗  ██╔══╝   ██╔╗██╗  ██║╚██╗██║ ██╔══╝   ██║                               ▒█▒\n");
    printf(" █▒    ██║  ██╗ ███████║ ██║ ║██║ ██║ ╚████║ ███████║ ███████║                          ▒█▒\n");
    printf(" █▒    ╚══════╝ ╚══════╝ ╚═╝ ╚══╝ ╚═╝  ╚═══╝ ╚══════╝ ╚══════╝                          ▒█▒\n");
    printf(" █▒                                                                                     ▒█▒\n");
    printf(" █████████████████████████████████████████████████████████████████████████████████████████▒\n");
    printf("  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");

    printf("  RUNNING AS A SANDBOX ");
    printf("  OS:          %s\n", os_name);
    printf("  Kernel:      %s\n", kernel);
    printf("  Shell:       LAVANDER SHELL\n");
    printf("  Architecture: %s\n", arch);

    uptime = platform_uptime_seconds();
    printf("  Uptime:      %02llu:%02llu:%02llu\n", uptime / 3600ULL,
           (uptime / 60ULL) % 60ULL, uptime % 60ULL);

    if (tm_info != NULL)
        printf("  Date:        %02d-%02d-%d\n", tm_info->tm_mday,
               tm_info->tm_mon + 1, tm_info->tm_year + 1900);

    if (platform_memory_info(&total_mb, &available_mb) == 0 && total_mb > 0)
        printf("  Memory:      %llu MB / %llu MB\n", total_mb - available_mb,
               total_mb);
    else
        printf("  Memory:      Unknown\n");

    printf("  CPU:         %s\n", cpu);
    printf("  Status:      Running\n\n");
    resetColor();
}

void free_memory_in_well_memory_ofc(void) {
    unsigned long long total_mb = 0;
    unsigned long long available_mb = 0;

    setColor();

    if (platform_memory_info(&total_mb, &available_mb) == 0 && total_mb > 0) {
        printf("██████████████████████████████████████████████\n");
        printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
        printf("█▒                                           █▒\n");
        printf("█▒ Memory: %llu MB total, %llu MB available █▒\n", total_mb,
               available_mb);
        printf("█▒                                           █▒\n");
        printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
        printf("██████████████████████████████████████████████▒\n");
        printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
    } else {
        printf("Memory information is unavailable.\n");
    }

    resetColor();
}
