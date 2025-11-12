/*
 * insecure_demo.c
 *
 * Purpose: Educational demo showing many insecure patterns that static tools
 * (e.g., flawfinder) will flag. Do NOT compile/run this on production systems.
 *
 * Each function demonstrates a common weakness; comments explain the issue
 * and recommended fix.
 *
 * Build (for lab testing only):
 *   gcc -Wall -Wextra -o insecure_demo insecure_demo.c
 *
 * NOTE: This file is intended to teach how to find and fix vulnerabilities.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/* --- Helper: intentionally insecure input fetch --- */
char *read_line_insecure(void) {
    /* Using gets() is dangerous (buffer overflow). gets() removed in C11.
       Flawfinder will flag this. */
    static char buf[256];
    printf("Enter a line (insecure demo): ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) { /* using fgets is slightly better */
        return NULL;
    }
    /* Remove newline */
    buf[strcspn(buf, "\n")] = '\0';
    return buf;
}

/* --- 1) Buffer overflow via unsafe copy --- */
void vuln_unbounded_copy(const char *src) {
    char local[64];
    /* strcpy has no bounds checking and is a classic vuln. Flawfinder flags strcpy. */
    strcpy(local, src); /* POTENTIAL BUFFER OVERFLOW */
    printf("You said: %s\n", local);
}

/* --- 2) Format string vulnerability --- */
void vuln_format_string(char *user) {
    /* Passing user-controlled string directly as format is dangerous. */
    printf(user); /* FORMAT STRING VULNERABILITY */
    printf("\n");
}

/* --- 3) Use of system() with unsanitized input --- */
void vuln_system_exec(char *cmd_part) {
    char cmd[512];
    /* Building a shell command with user data allows command injection. */
    snprintf(cmd, sizeof(cmd), "echo 'running' && %s", cmd_part); /* UNSAFE: user content in command */
    system(cmd); /* COMMAND INJECTION RISK */
}

/* --- 4) Use of tmpnam / insecure temporary file handling --- */
void vuln_tmpfile_demo(void) {
    char *tmp = tmpnam(NULL); /* tmpnam is insecure and flagged */
    if (tmp) {
        printf("Temp filename (insecure): %s\n", tmp);
        /* Opening temp file without O_EXCL might allow TOCTOU / symlink attack */
        int fd = open(tmp, O_RDWR | O_CREAT, 0666); /* weak permissions */
        if (fd >= 0) {
            write(fd, "data\n", 5);
            close(fd);
        }
    }
}

/* --- 5) Weak RNG for security-related use --- */
void vuln_weak_random(void) {
    /* Using rand() for anything security-related is insecure. srand(time(NULL)) seeds with low entropy. */
    srand(time(NULL));
    int r = rand(); /* WEAK RNG */
    printf("Weak random number (do not use for crypto): %d\n", r);
}

/* --- 6) Hardcoded credentials --- */
void vuln_hardcoded_pwd(void) {
    /* Hardcoding secrets in source is a risk. */
    const char *password = "P@ssw0rd123"; /* HARD-CODED PASSWORD */
    printf("Using hardcoded password: %s\n", password);
}

/* --- 7) Use of gets-style / unbounded scanf --- */
void vuln_unbounded_scan(void) {
    char buf[32];
    /* %s with scanf has no width; can overflow. Flawfinder flags scanf usage without width. */
    printf("Enter a short word: ");
    scanf("%s", buf); /* UNSAFE: no width specified */
    printf("You entered: %s\n", buf);
}

/* --- 8) Insecure file permission / race condition demonstration --- */
void vuln_insecure_open(const char *filename) {
    /* Opening files with world-writable permissions is bad. O_CREAT without O_EXCL -> TOCTOU. */
    int fd = open(filename, O_CREAT | O_WRONLY, 0666); /* INSECURE PERMISSIONS */
    if (fd >= 0) {
        write(fd, "log\n", 4);
        close(fd);
    } else {
        perror("open");
    }
}

/* --- 9) Pointer leak / use-after-free style (demonstrative) --- */
void vuln_use_after_free(void) {
    char *p = malloc(32);
    if (!p) return;
    strcpy(p, "hello"); /* small buffer ok here but demonstrates ownership issue */
    free(p);
    /* Use after free */
    printf("After free: %s\n", p); /* USE-AFTER-FREE (undefined behavior) */
}

/* --- 10) Weak parsing leading to integer overflow --- */
void vuln_integer_overflow(const char *numstr) {
    long val = atol(numstr); /* no error handling */
    /* If attacker supplies a huge number, operations may overflow */
    long size = val * 1024 * 1024;
    char *buf = malloc(size); /* could fail or overflow */
    if (buf) {
        memset(buf, 0, 1); /* touch to avoid optimized-out */
        free(buf);
    }
}

/* --- main: call the vulnerable demos --- */
int main(int argc, char **argv) {
    char *line = read_line_insecure();
    if (line) {
        /* intentionally call vulnerable routines for demonstration */
        vuln_unbounded_copy(line);
        vuln_format_string(line);
    }

    printf("\n--- system exec demo ---\n");
    if (argc > 1) {
        vuln_system_exec(argv[1]);
    } else {
        vuln_system_exec("ls -la /tmp"); /* example command part */
    }

    printf("\n--- tmpfile demo ---\n");
    vuln_tmpfile_demo();

    printf("\n--- weak RNG demo ---\n");
    vuln_weak_random();

    printf("\n--- hardcoded password demo ---\n");
    vuln_hardcoded_pwd();

    printf("\n--- unbounded scanf demo ---\n");
    vuln_unbounded_scan();

    printf("\n--- insecure open demo ---\n");
    vuln_insecure_open("insecure_log.txt");

    printf("\n--- use-after-free demo ---\n");
    vuln_use_after_free();

    printf("\n--- integer overflow demo ---\n");
    vuln_integer_overflow("4000000"); /* large-ish number for demo */

    return 0;
}