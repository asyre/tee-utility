#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <getopt.h>

#define MAX 8192

void print_error();

void write_to(int file_desc, char *buf, long count);

void close_file(int file_desc);

char *read_from_stdin(int *read_count);

mode_t command_flag_parser(char **argv, int argc, int *filePosition);

bool is_stdin_empty(void);

int command_processor(int argc, char **argv);


void sighandler(int signal);

int main(int argc, char **argv) {
    return command_processor(argc, argv);
}

int *open_file(char **argv, mode_t mode, int argc, int file_position, int *files_count) {
    int *files_desc = malloc(sizeof(int) * argc);
    int j = 0;
    for (int i = file_position; i < argc; ++i) {
        int fileDescriptor;
        if (strcmp(argv[i], ">") == 0) {
            mode = O_WRONLY | O_TRUNC;
            i++;
        } else if (strcmp(argv[i], ">>") == 0) {
            mode = O_WRONLY | O_APPEND;
            i++;
        } else if ((strcmp(argv[i], "--") == 0)) i++;
        fileDescriptor = open(argv[i], mode | O_CREAT, 0644);
        if (fileDescriptor < 0)
            print_error();
        files_desc[j++] = fileDescriptor;
    }
    *files_count = j;
    return files_desc;
}

int command_processor(int argc, char **argv) {
//    int count = 0;
//    int ptrSize = 1;
    char c[1];
    c[0] = (char) 0;
//    char *ptr = malloc(sizeof(char));
    int filePosition = 1;
    mode_t mode = command_flag_parser(argv, argc, &filePosition);
    int files_count = 0;
    int *files = open_file(argv, mode, argc, filePosition, &files_count);
    return 0;
}


char *read_line(int *read_count) {
    *read_count = 0;
    int sum_read = 0;
    int count;
    char *lines = malloc(sizeof(char));
    int i = 1;
    do {
        count = read(STDIN_FILENO, lines + i - 1, sizeof(char));
        if (count > 0) {
            sum_read += count;
            lines = realloc(lines, sizeof(char) * ++i + 1);
        }
    } while (lines[i - 2] != '\n');
    (*read_count) = sum_read;
    write_to(STDOUT_FILENO, lines, *read_count);
    return lines;
}

char *read_from_stdin(int *read_count) {
    *read_count = 0;
    char *lines = malloc(sizeof(char) * MAX);
    *read_count = read(STDIN_FILENO, lines, MAX);
    write_to(STDOUT_FILENO, lines, *read_count);
    return lines;
}


//int write_in_file(char *buf, int count) {
//    write_to(fileDescriptor, buf, count);
//    close_file(fileDescriptor);
//    return 0;
//}

mode_t command_flag_parser(char **argv, int argc, int *filePosition) {
    *filePosition = 1;
    mode_t mode = O_WRONLY | O_TRUNC;

    const char *short_options = "ai";

    const struct option long_options[] = {
            {"help", no_argument, NULL, 'h'}
    };
    int val;
    opterr = 0;
    while ((val = getopt_long(argc, argv, short_options,
                              long_options, NULL)) != -1) {
        switch (val) {
            case 'h': {
                if ((strcmp(argv[1], "--help") == 0)) {
                    write_to(STDERR_FILENO, "usage: tee [-ai] [file ...]\n", 29);
                    exit(0);
                }
                break;
            }
            case 'a': {
                ++*filePosition;
                mode = O_WRONLY | O_APPEND;
                break;
            }

            case 'i': {
                (void) signal(SIGINT, SIG_IGN);
                ++*filePosition;
                break;
            }
            case '?':
            default:
                break;
        }
    }
    return mode;
}

void sighandler() {
}

bool is_stdin_empty(void) {
    fd_set rd;
    struct timeval tv = {0};
    int ret;
    FD_ZERO(&rd);
    FD_SET(STDIN_FILENO, &rd);
    ret = select(1, &rd, NULL, NULL, &tv);
    return (ret == 0);
}

void write_to(int file_desc, char *buf, long count) {
    ssize_t bytes_read = 0;
    ssize_t n = 0;
    do {
        n = write(file_desc, buf + bytes_read, count - bytes_read);
        if (n == -1) print_error();
        bytes_read += n;
    } while (bytes_read < count);
}


void close_file(int file_desc) {
    if (close(file_desc) < 0) print_error();
}

void print_error() {
    char *error = strerror(errno);
    write_to(STDERR_FILENO, error, strlen(error));
}
