#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <ctype.h>

#include "logger.h"
#include "procfs.h"
#include "util.h"

/**
 * Looks for the hostname.
 *
 * @param proc_dir directory
 * @param hostname_buf character pointer that stores the hostname
 * @param buf_sz size of the hostname
 *
 * @return 0 if the hostname is found and stored, -1 if not
 */
int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz) {
    int fd = open_path(proc_dir, "sys/kernel/hostname");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

    ssize_t read_sz = one_lineread(fd, hostname_buf, buf_sz, "\n");
    if (read_sz == -1) {
        return -1;
    }
    close(fd);

    return 0;
}

/**
 * Looks for the kernel version.
 *
 * @param proc_dir directory
 * @param version_buf character pointer that stores the kernel version
 * @param buf_sz size of the kernel version
 *
 * @return 0 if the hostname is found and stored, -1 if not
 */
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz) {
    int fd = open_path(proc_dir, "sys/kernel/osrelease");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

    ssize_t read_sz = one_lineread(fd, version_buf, buf_sz, "-");
    if (read_sz == -1) {
        return -1;
    }
    close(fd);

    return 0;
}

/**
 * Looks for the CPU model
 *
 * @param proc_dir directory
 * @param model_buf character pointer that stores the CPU model
 * @param buf_sz size of the CPU model
 *
 * @return 0 if the hostname is found and stored, -1 if not
 */
int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz) {
    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

    // model_buf is going to be 128 characters in display.c, so we will use that plus some headroom here:
    char line[256] = { 0 };
    ssize_t read_sz;
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "model name", 10) == 0) {
            size_t model_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[model_loc], "\n");
            strncpy(model_buf, &line[model_loc], newline_loc);
        }
    }
    close(fd);

    return 0;
}

/**
 * Looks for the CPU units
 *
 * @param proc_dir directory
 *
 * @return the CPU units found
 */
int pfs_cpu_units(char *proc_dir) {
    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

    char processor_buff[128] = { 0 }, line[256] = { 0 };
    ssize_t read_sz;
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "processor", 9) == 0) {
            size_t processor_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[processor_loc], "\n");
            strncpy(processor_buff, &line[processor_loc], newline_loc);
        }
    }
    close(fd);

    return atoi(processor_buff) + 1;
}

/**
 * Looks for the pfs uption
 *
 * @param proc_dir directory
 *
 * @return the uptime found
 */
double pfs_uptime(char *proc_dir) {
    double uptime = 0.0;
    int fd = open_path(proc_dir, "uptime");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

    char uptime_buf[256] = { 0 };
    ssize_t read_sz = one_lineread(fd, uptime_buf, 256, " ");
    if (read_sz == -1) {
        return -1;
    }
    close(fd);
    char *end;
    uptime = strtod(uptime_buf, &end);

    return uptime;
}

/**
 * Generates the formatted uptime
 *
 * @param time value of the uptime
 * @param uptime_buf character pointer that stores the uptime
 *
 * @return 0 if the formatted uptime is generated, -1 if not
 */
int pfs_format_uptime(double time, char *uptime_buf) {
    int uptime = (int) time;
    int seconds = uptime % 60;
    int minutes_full = uptime / 60;
    int minutes = minutes_full % 60;
    int hours_full = minutes_full / 60;
    int hours = hours_full % 24;
    int days_full = hours_full / 24;
    int days = days_full % 365;
    int years = days / 365;

    int i = 0;
    if (years != 0) {
        char years_buf[128];
        sprintf(years_buf, "%d years, ", years);
        sprintf(uptime_buf + i, years_buf);
        i += (strlen(years_buf) + 1);
    }
    if (days != 0) {
        char days_buf[128];
        sprintf(days_buf, "%d days, ", days);
        sprintf(uptime_buf + i, days_buf);
        i += (strlen(days_buf));
    }
    if (hours != 0) {
        char hours_buf[128];
        sprintf(hours_buf, "%d hours, ", hours);
        sprintf(uptime_buf + i, hours_buf);
        i += (strlen(hours_buf));
    }
    if (minutes != 0) {
        char minutes_buf[128];
        sprintf(minutes_buf, "%d minutes, ", minutes);
        sprintf(uptime_buf + i, minutes_buf);
        i += (strlen(minutes_buf));
    }
    if (seconds != 0) {
        char seconds_buf[128];
        sprintf(seconds_buf, "%d seconds", seconds);
        sprintf(uptime_buf + i, seconds_buf);
    }

    return 0;
}

/**
 * Generates the load average.
 *
 * @param proc_dir directory
 *
 * @return the struct of load average
 */
struct load_avg pfs_load_avg(char *proc_dir) {
    struct load_avg lavg = { 0 };
    int fd = open_path(proc_dir, "loadavg");
    if (fd == -1) {
        perror("open_path");
        return lavg;
    }

    char loadavg[256];
    ssize_t read_sz = lineread(fd, loadavg, 256);
    if (read_sz == -1) {
        return lavg;
    }
    close(fd);

    char *loadavg_buf = loadavg;
    char *s1 = strsep(&loadavg_buf," ");
    char *end1;
    double one = strtod(s1, &end1);
    char *s2 = strsep(&loadavg_buf," ");
    char *end2;
    double five = strtod(s2, &end2);
    char *s3 = strsep(&loadavg_buf," ");
    char *end3;
    double fifteen = strtod(s3, &end3);

    lavg.one = one;
    lavg.five = five;
    lavg.fifteen = fifteen;

    return lavg;
}

/**
 * Generates the CPU usage
 *
 * @param proc_dir directory
 * @param prev previous CPU status
 * @param curr current CPU status
 *
 * @return the CPU usage
 */
double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr) {
    int fd = open_path(proc_dir, "stat");
    if (fd == -1) {
        perror("open_path");
        return 0;
    }

    char cpu[256] = { 0 };
    ssize_t read_sz = lineread(fd, cpu, 256);
    if (read_sz == -1) {
        return 0;
    }
    close(fd);

    char *cpu_buff = cpu;
    char *token;
    int i = 0;
    curr->total = 0;
    while((token = strsep(&cpu_buff, " ")) != NULL) {
        i++;
        if (i == 1) {
            continue;
        }
        if (i == 6) {
            curr->idle = atoi(token);
        }
        curr->total += atoi(token);
    }

    double usage = 1 - ((double)(curr->idle - prev->idle) / (double)(curr->total - prev->total));
    if(((curr->idle - prev->idle < 0) || (curr->total - prev->total < 0)) || isnan(usage)) {
        return 0.0;
    }

    return (double)usage;
}

/**
 * Generates the memory usage.
 *
 * @param proc_dir directory
 *
 * @return struct of memory usage
 */
struct mem_stats pfs_mem_usage(char *proc_dir) {
    struct mem_stats mstats = { 0 };
    int fd = open_path(proc_dir, "meminfo");
    if (fd == -1) {
        perror("open_path");
        return mstats;
    }

    char meminfo_buf[256] = { 0 };
    ssize_t read_sz = one_lineread(fd, meminfo_buf, 256, " ");
    if (read_sz == -1) {
        return mstats;
    }

    char total_buff[256] = { 0 }, available_buff[256] = { 0 }, line[256] = { 0 };
    ssize_t read_sz1;
    while ((read_sz1 = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "MemTotal", 8) == 0) {
            size_t total_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[total_loc], "\n");
            strncpy(total_buff, &line[total_loc], newline_loc);
        }
        if (strncmp(line, "MemAvailable", 12) == 0) {
            size_t available_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[available_loc], "\n");
            strncpy(available_buff, &line[available_loc], newline_loc);
        }
    }
    close(fd);

    char *end1, *end2;
    double mem_total = strtod(total_buff, &end1) / 1024 / 1024;
    double mem_available = strtod(available_buff, &end2) / 1024 / 1024;
    double mem_used = mem_total - mem_available;
    mstats.total = mem_total;
    mstats.used = mem_used;

    return mstats;
}

/**
 * Creates task_stats structs.
 *
 * @return the task_stats structs created
 */
struct task_stats *pfs_create_tstats() {
    struct task_stats *ts = calloc(1, sizeof(struct task_stats));
    if (ts != NULL) {
        ts->active_tasks = calloc(5000, sizeof(struct task_info));
    }
    return ts;
}

/**
 * Destroys task_stats structs.
 *
 * @param tstats the task_stats struct to be destroyed
 */
void pfs_destroy_tstats(struct task_stats *tstats) {
    free(tstats->active_tasks);
    free(tstats);
}

/**
 * Checks whether a string literal is a number
 * 
 * @param str the string literal to be checked
 * @return the boolean whether indicating whether the string literal is a number
 */
bool isNumeric(char *str) {
    while (*str) {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }
    return true;
}

/**
 * Looks for the tasks
 *
 * @param proc_dir directory
 * @param tstats the task struct
 *
 * @return 
 */
int pfs_tasks(char *proc_dir, struct task_stats *tstats) {
    DIR *directory;
    if ((directory = opendir(proc_dir)) == NULL) {
        perror("opendir");
        return 1;
    }
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(directory)) != NULL) {
        if (isNumeric(entry->d_name)) {
            size_t path_len = strlen(proc_dir) + 1 + strlen(entry->d_name) + strlen("/status") + 1;
            char *full_path = malloc(path_len * sizeof(char));
            if (full_path == NULL) {
                free(full_path);
                return -1;
            }

            strcpy(full_path, proc_dir);
            strcat(full_path, "/");
            strcat(full_path, entry->d_name);
            strcat(full_path, "/status");
            int fd = open(full_path, O_RDONLY);
            free(full_path);
            if (fd == -1) {
                continue;
            }

            ssize_t read_sz;
            char line[256] = { 0 }, name_buff[256] = { 0 }, state_buff[256] = { 0 }, uid_buff[256] = { 0 };
            while ((read_sz = lineread(fd, line, 256)) > 0) {
                if (strncmp(line, "Name", 4) == 0) {
                    size_t name_loc = strcspn(line, ":") + 2;
                    size_t newline_loc1 = strcspn(&line[name_loc], "\n");
                    if (newline_loc1 > 25) {
                        newline_loc1 = 25;
                    }
                    strncpy(name_buff, &line[name_loc], newline_loc1);
                }
                if (strncmp(line, "State", 5) == 0) {
                    size_t state_loc = strcspn(line, ":") + 2;
                    size_t newline_loc2 = strcspn(&line[state_loc], "\n");
                    strncpy(state_buff, &line[state_loc], newline_loc2);
                }
                if (strncmp(line, "Uid", 3) == 0) {
                    size_t uid_loc = strcspn(line, ":") + 2;
                    size_t newline_loc4 = strcspn(&line[uid_loc], "\n");
                    strncpy(uid_buff, &line[uid_loc], newline_loc4);
                }
            }
            close(fd);

            tstats->total++;
            if (state_buff[0] == 'R') {
                tstats->running++;
                strcpy(tstats->active_tasks[i].state, "running");
            }
            else if (state_buff[0] == 'D') {
                tstats->waiting++;
                strcpy(tstats->active_tasks[i].state, "disk sleep");
            }
            else if (state_buff[0] == 'S' || state_buff[0] == 'I') {
                tstats->sleeping++;
                strcpy(tstats->active_tasks[i].state, "sleeping");
            }
            else if (state_buff[0] == 'T') {
                tstats->stopped++;
                strcpy(tstats->active_tasks[i].state, "stopped");
            }
            else if (state_buff[0] == 't') {
                tstats->stopped++;
                strcpy(tstats->active_tasks[i].state, "tracing stop");
            }
            else if (state_buff[0] == 'Z') {
                tstats->zombie++;
                strcpy(tstats->active_tasks[i].state, "zombie");
            }

            pid_t pid = atoi(entry->d_name);
            uid_t uid = atoi(uid_buff);

            if (strcmp(tstats->active_tasks[i].state, "sleeping") != 0) {
                tstats->active_tasks[i].pid = pid;
                tstats->active_tasks[i].uid = uid;
                strcpy(tstats->active_tasks[i].name, name_buff);
                i++;
            }
        }
    }
    closedir(directory);

    return 0;
}
