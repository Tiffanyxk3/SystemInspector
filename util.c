#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>


#include "util.h"
#include "logger.h"

int open_path(char *proc_dir, char *path) {
	if (proc_dir == NULL || path == NULL) {
		errno = EINVAL;
		return -1;
	}

	size_t path_len = strlen(proc_dir) + strlen(path) + 2;
	char *full_path = malloc(path_len * sizeof(char));
	if (full_path == NULL) {
		return -1;
	}

	snprintf(full_path, path_len, "%s/%s", proc_dir, path);
	LOG("Opening path: %s + %s\n", proc_dir, path);

	int fd = open(full_path, O_RDONLY);
	free(full_path);
	
	return fd;
}

ssize_t lineread(int fd, char *buf, size_t sz) {
	size_t counter = 0;
	
	while (counter < sz) {
		char c;
		ssize_t read_sz = read(fd, &c, 1);

		if (read_sz == 0) {
			return 0;
		}
		else if (read_sz == -1) {
			return -1;
		}
		else {
			buf[counter] = c;
			counter++;
			if (c == '\n') {
				return counter;
			}
		}
	}
	return counter;
}

ssize_t one_lineread(int fd, char *buf, size_t sz, char *delim) {
	ssize_t read_sz = lineread(fd, buf, sz);
    if (read_sz == -1) {
        return -1;
    }

    size_t token_loc = strcspn(buf, delim);
    buf[token_loc] = '\0';

    return read_sz;
}

void draw_percbar(char *buf, double frac) {
	frac *= 100;
	if (frac <= 0 || isnan(frac)) {
		frac = 0;
	}
	if (frac >= 100) {
		frac = 100;
	}
	*buf = '[';
	for (int i = 1; i < 21; i++) {
		if (i <= round(frac) / 5) {
			*(buf + i) = '#';
		}
		else {
			*(buf + i) = '-';
		}
	}
	*(buf + 21) = ']';
	*(buf + 22) = ' ';
	for (int i = 23; *(buf + i) != '\0'; i++) {
		*(buf + i) = '\0';
	}
	char fstr[16];
	snprintf(fstr, 16, "%.1f", frac);
	strcat(buf, fstr);
	strcat(buf, "%");
}

void uid_to_uname(char *name_buf, uid_t uid) {
	if (uid == 0) {
		strcpy(name_buf, "root");
	}
	else if (uid == 1) {
		strcpy(name_buf, "bin");
	}
	else if (uid == 2) {
		strcpy(name_buf, "daemon");
	}
	else if (uid == 8) {
		strcpy(name_buf, "mail");
	}
	else if (uid == 14) {
		strcpy(name_buf, "ftp");
	}
	else if (uid == 33) {
		strcpy(name_buf, "http");
	}
	else if (uid == 65534) {
		strcpy(name_buf, "nobody");
	}
	else if (uid == 81) {
		strcpy(name_buf, "dbus");
	}
	else if (uid == 982) {
		strcpy(name_buf, "systemd-journal");
	}
	else if (uid == 981) {
		strcpy(name_buf, "systemd-network");
	}
	else if (uid == 980) {
		strcpy(name_buf, "systemd-resolve");
	}
	else if (uid == 979) {
		strcpy(name_buf, "systemd-timesyn");
	}
	else if (uid == 978) {
		strcpy(name_buf, "systemd-coredum");
	}
	else if (uid == 68) {
		strcpy(name_buf, "uuidd");
	}
	else if (uid == 977) {
		strcpy(name_buf, "dhcpcd");
	}
	else if (uid == 976) {
		strcpy(name_buf, "git");
	}
	else if (uid == 1000) {
		strcpy(name_buf, "nzhang18");
	}
	else if (uid == 975) {
		strcpy(name_buf, "avahi");
	}
	else if (uid == 974) {
		strcpy(name_buf, "colord");
	}
	else if (uid == 102) {
		strcpy(name_buf, "polkitd");
	}
	else {
		char uid_str[16];
		snprintf(uid_str, 16, "%d", uid);
		strcpy(name_buf, uid_str);
	}
}
