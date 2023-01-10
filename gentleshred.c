/* Copyright © 2018-2023 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <unistd.h>

#define PROGRAM_NAME "gentleshred"

static size_t block_size = 0;
static const size_t block_size_limit = SIZE_MAX / 2;

static void show_usage(FILE *fp)
{
    fprintf(fp, "Usage: %s [-b SIZE] FILE [FILE...]\n", PROGRAM_NAME);
    if (fp != stdout)
        return;
    fprintf(fp,
        "\n"
        "Options:\n"
        "  -b SIZE     set block size to SIZE\n"
        "  -h, --help  show this help message and exit\n"
    );
}

static void posix_error(const char *context)
{
    int orig_errno = errno;
    fprintf(stderr, "%s: ", PROGRAM_NAME);
    errno = orig_errno;
    perror(context);
    exit(EXIT_FAILURE);
}

static ssize_t xread(int fd, char *buffer, size_t size)
{
    size_t asize = 0;
    while (asize < size) {
        ssize_t n = read(fd, buffer + asize, size - asize);
        if (n < 0)
            return n;
        if (n == 0)
            break;
        asize += n;
    }
    return asize;
}

static ssize_t xwrite(int fd, const char *buffer, size_t size)
{
    size_t asize = 0;
    while (asize < size) {
        ssize_t n = write(fd, buffer + asize, size - asize);
        if (n < 0)
            return n;
        if (n == 0) {
            errno = EIO;
            return -1;
        }
        asize += n;
    }
    assert(asize == size);
    return asize;
}

static void shred_file(int fd, const char *path)
{
    struct statvfs st;
    size_t bufsize = block_size;
    if (block_size == 0) {
        int rc = fstatvfs(fd, &st);
        if (rc < 0)
            posix_error(path);
        bufsize = st.f_bsize;
    }
    char *buffer = malloc(bufsize);
    if (buffer == NULL)
        posix_error("malloc()");
    char *zbuffer = calloc(bufsize, 1);
    if (zbuffer == NULL)
        posix_error("calloc()");
    while (1) {
        ssize_t n = xread(fd, buffer, bufsize);
        if (n < 0)
            posix_error(path);
        if (n > 0) {
            bool shred_needed = false;
            for (ssize_t i = 0; i < n; i++)
                if (buffer[i]) {
                    shred_needed = true;
                    break;
                }
            if (shred_needed) {
                off_t off = lseek(fd, -n, SEEK_CUR);
                if (off < 0)
                    posix_error(path);
                ssize_t m = xwrite(fd, zbuffer, n);
                if (m < 0)
                    posix_error(path);
            }
        }
        if ((size_t)n < bufsize)
            break;
    }
    free(buffer);
    free(zbuffer);
}

int main(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "b:h-:")) != -1)
        switch (opt) {
        case 'b':
        {
            char *endptr;
            long value;
            errno = 0;
            value = strtol(optarg, &endptr, 10);
            if (errno != 0)
                ;
            else if (endptr == optarg || *endptr != '\0')
                errno = EINVAL;
            else if (value <= 0 || (unsigned long) value >= block_size_limit)
                errno = ERANGE;
            if (errno != 0)
                posix_error("-b");
            block_size = (size_t) value;
            break;
        }
        case 'h':
            show_usage(stdout);
            exit(EXIT_SUCCESS);
        case '-':
            if (strcmp(optarg, "help") == 0) {
                show_usage(stdout);
                exit(EXIT_SUCCESS);
            }
            /* fall through */
        default:
            show_usage(stderr);
            exit(EXIT_FAILURE);
        }
    if (optind >= argc) {
        show_usage(stderr);
        exit(EXIT_FAILURE);
    }
    argc -= optind;
    argv += optind;
    for (; *argv; argv++) {
        const char *path = *argv;
        int fd = open(path, O_RDWR);
        if (fd < 0)
            posix_error(path);
        shred_file(fd, path);
        close(fd);
    }
    return 0;
}

/* vim:set ts=4 sts=4 sw=4 et:*/
