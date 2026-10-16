#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "config.h"

void showcase_of_working_proces_in_background(void) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    FILE *f;
    char line[256];
    char os_name[128] = "Linux";
    char kernel[128] = "Unknown";
    char arch[64] = "Unknown";
    char cpu[128] = "Unknown";
    unsigned long long total = 0;
    unsigned long long available = 0;

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

    f = fopen("/etc/os-release", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *p = line + 12;
                if (*p == '"') p++;
                strncpy(os_name, p, sizeof(os_name) - 1);
                os_name[sizeof(os_name) - 1] = '\0';
                os_name[strcspn(os_name, "\n")] = 0;
                if (os_name[strlen(os_name) - (strlen(os_name) > 0 ? 1 : 0)] == '"' && strlen(os_name) > 0)
                    os_name[strlen(os_name) - 1] = '\0';
                break;
            }
        }
        fclose(f);
    }

    printf("  OS:          %s\n", os_name);

    f = fopen("/proc/version", "r");
    if (f) {
        if (fgets(line, sizeof(line), f))
            sscanf(line, "Linux version %127s", kernel);
        fclose(f);
    }
    printf("  Kernel:      %s\n", kernel);
    printf("  Shell:       LAVANDER SHELL\n");

    f = popen("uname -m", "r");
    if (f) {
        if (fgets(arch, sizeof(arch), f))
            arch[strcspn(arch, "\n")] = 0;
        pclose(f);
    }
    printf("  Architecture: %s\n", arch);

    if (tm_info != NULL) {
        printf("  Uptime:      %02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
        printf("  Date:        %02d-%02d-%d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
    }

    f = fopen("/proc/meminfo", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, "MemTotal: %llu kB", &total) == 1) continue;
            if (sscanf(line, "MemAvailable: %llu kB", &available) == 1) continue;
        }
        fclose(f);
    }

    if (total > 0)
        printf("  Memory:      %llu MB / %llu MB\n", (total - available) / 1024, total / 1024);
    else
        printf("  Memory:      Unknown\n");

    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *p = strchr(line, ':');
                if (p) {
                    p++;
                    while (*p == ' ') p++;
                    strncpy(cpu, p, sizeof(cpu) - 1);
                    cpu[sizeof(cpu) - 1] = '\0';
                    cpu[strcspn(cpu, "\n")] = 0;
                    break;
                }
            }
        }
        fclose(f);
    }

    printf("  CPU:         %s\n", cpu);
    printf("  Status:      Running\n\n");
    resetColor();
}

void free_memory_in_well_memory_ofc(void) {
    setColor();
#if defined(__linux__)
    FILE *memory_file = fopen("/proc/meminfo", "r");
    char line[128];
    unsigned long long total = 0;
    unsigned long long available = 0;

    if (memory_file != NULL) {
        while (fgets(line, sizeof(line), memory_file) != NULL) {
            if (sscanf(line, "MemTotal: %llu kB", &total) == 1) continue;
            if (sscanf(line, "MemAvailable: %llu kB", &available) == 1) continue;
        }
        fclose(memory_file);
    }

    if (total > 0) {
        printf("██████████████████████████████████████████████\n");
        printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
        printf("█▒                                           █▒\n");
        printf("█▒ Memory: %llu MB total, %llu MB available █▒\n", total / 1024, available / 1024);
        printf("█▒                                           █▒\n");
        printf("█▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒\n");
        printf("██████████████████████████████████████████████▒\n");
        printf(" ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n");
    } else {
        printf("Memory information is unavailable.\n");
    }
#else
    printf("Memory information is unavailable on this platform.\n");
#endif
    resetColor();
}
