#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#define MAX 8192

void print_error();

void write_to(int file_desc, char *buf, long count);

void close_file(int file_desc);

char *read_from_stdin(int *read_count);

mode_t command_flag_parser(char **argv, int argc, int *filePosition);

int command_processor(int argc, char **argv);

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
        }
//        } else if ((strcmp(argv[i], "--") == 0)) i++;
        fileDescriptor = open(argv[i], mode | O_CREAT, 0644);
        if (fileDescriptor < 0)
            print_error();
        files_desc[j++] = fileDescriptor;
    }
    *files_count = j;
    return files_desc;
}

int command_processor(int argc, char **argv) {
    int count = 0;
    char c[1];
    c[0] = (char) 0;
    int filePosition = 1;
    mode_t mode = command_flag_parser(argv, argc, &filePosition);
    int files_count = 0;
    int *files = open_file(argv, mode, argc, filePosition, &files_count);
    do {
        char *buf = read_from_stdin(&count);
        for (int i = 0; i < files_count; ++i) {
            write_to(files[i], buf, count);
        }
        free(buf);
    } while (count != 0);
    for (int i = 0; i < files_count; ++i) {
        close_file(files[i]);
    }
    return 0;
}

char *read_from_stdin(int *read_count) {
    *read_count = 0;
    char *lines = malloc(sizeof(char) * MAX);
    *read_count = read(STDIN_FILENO, lines, MAX);
    write_to(STDOUT_FILENO, lines, *read_count);
    return lines;
}


mode_t command_flag_parser(char **argv, int argc, int *filePosition) {
    *filePosition = 1;
    mode_t mode = O_WRONLY | O_TRUNC;
    int val;
//    opterr = 0;
    while ((val = getopt(argc, argv, "ai")) != -1) {
        switch (val) {
            case 'a':
                ++*filePosition;
                mode = O_WRONLY | O_APPEND;
                break;
            case 'i':
                (void) signal(SIGINT, SIG_IGN);
                ++*filePosition;
                break;
            default:
                write_to(STDERR_FILENO, "usage: tee [-ai] [file ...]\n", 29);
                exit(0);
        }
    }

//    const char *short_options = "ai";
//
//    const struct option long_options[] = {
//            {"help", no_argument, NULL, 'h'}
//    };
//    while ((val = getopt_long(argc, argv, short_options,
//                              long_options, NULL)) != -1) {
//        switch (val) {
//            case 'a': {
//                ++*filePosition;
//                mode = O_WRONLY | O_APPEND;
//                break;
//            }
//
//            case 'i': {
//                (void) signal(SIGINT, SIG_IGN);
//                ++*filePosition;
//                break;
//            }
//            case '?':
//            default:
//            case 'h': {
//                write_to(STDERR_FILENO, "usage: tee [-ai] [file ...]\n", 29);
//                exit(0);
//            }
//        }
//    }
    return mode;
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
