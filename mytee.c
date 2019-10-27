#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX 10000

void write_to(int place, char *buf, int count);

mode_t command_flag_parser(const char **argv, int argc, int *filePosition, bool *Flag);

bool is_stdin_empty(void);

int command_processor(int argc, const char *argv[]);

int write_in_file(const char **argv, mode_t mode, char *buf, int count, int argc, int filePosition);

char *read_line(int *read_count);

char *read_from_stdin(int *count);

void sighandler(int signal);

int main(int argc, const char *argv[]) {
    return command_processor(argc, argv);
}

int command_processor(int argc, const char **argv) {
    int count = 0;
    int ptrSize = 1;
    char c[1];
    c[0] = (char) 0;
    bool Flag = false;
    char *ptr = malloc(sizeof(char));
    int filePosition = 1;
    mode_t mode = command_flag_parser(argv, argc, &filePosition, &Flag);
    if (Flag) signal(SIGINT, sighandler);
    if (!is_stdin_empty()) {
        write_in_file(argv, mode, &c[0], count, argc, filePosition);
        mode = O_WRONLY | O_APPEND;
        do {
            char *buf = read_from_stdin(&count);
            write_in_file(argv, mode, buf, count, argc, filePosition);
            free(buf);
        } while (count != 0);
    } else {
        while (1) {
            if (ptrSize == 0) mode = O_WRONLY | O_APPEND;
            char *tmp = read_line(&count);
            ptrSize = count;
            ptr = realloc(ptr, sizeof(char) * ptrSize);
            memcpy(ptr, tmp, ptrSize);
            write_in_file(argv, mode, ptr, ptrSize, argc, filePosition);
            ptrSize = 0;
        }
    }
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


int write_in_file(const char **argv, mode_t mode, char *buf, int count, int argc, int filePosition) {
    for (int i = filePosition; i < argc; ++i) {
        int fileDescriptor;
        if (strcmp(argv[i], ">") == 0) {
            //large file done in cmake file
            mode = O_WRONLY | O_TRUNC;
            i++;
        } else if (strcmp(argv[i], ">>") == 0) {
            mode = O_WRONLY | O_APPEND;
            i++;
        }
        fileDescriptor = open(argv[i], mode | O_CREAT, 0644);
        if (fileDescriptor < 0) {
            return 1;
        }
        write_to(fileDescriptor, buf, count);
        if (close(fileDescriptor) < 0) return 1;
    }
    return 0;
}


mode_t command_flag_parser(const char **argv, int argc, int *filePosition, bool *Flag) {
    *filePosition = 1;
    //large file done in cmake file
    mode_t mode = O_WRONLY | O_TRUNC;
    if (argc >= 3) {
        if (strcmp(argv[1], "-a") == 0 || strcmp(argv[2], "-a") == 0) {
            mode = O_WRONLY | O_APPEND;
            ++*filePosition;
        }
        if (strcmp(argv[1], "-i") == 0 || strcmp(argv[2], "-i") == 0) {
            ++*filePosition;
            *Flag = true;
        }
    } else if (argc >= 2) {
        if (strcmp(argv[1], "-a") == 0) {
            mode = O_WRONLY | O_APPEND;
            ++*filePosition;
        }
        if (strcmp(argv[1], "-i") == 0) {
            *Flag = true;
            ++*filePosition;
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
    //allow a program to monitor multiple file
    //       descriptors, waiting until one or more of the file descriptors become
    //       "ready" for some class of I/O operation
//    This blocks the program until input or output is ready on a specified set of file descriptors, or until a timer expires, whichever comes first.
    ret = select(1, &rd, NULL, NULL, &tv);
    return (ret == 0);
}

void write_to(int place, char *buf, int count) {
    ssize_t bytes_read = 0;
    ssize_t n = 0;
    do {
        n = write(place, buf + bytes_read, count - bytes_read);
        if (n == -1) exit(EIO);
        bytes_read += n;
    } while (bytes_read < count);
}