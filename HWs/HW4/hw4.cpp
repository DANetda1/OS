#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 4096

void copy_file(const char *src, const char *dst) {
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        perror("open source file");
        exit(EXIT_FAILURE);
    }
    struct stat st;
    if (fstat(src_fd, &st) < 0) {
        perror("fstat");
        close(src_fd);
        exit(EXIT_FAILURE);
    }
    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (dst_fd < 0) {
        perror("open destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }
    char buffer[BUF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, BUF_SIZE)) > 0) {
        char *out_ptr = buffer;
        ssize_t bytes_written;
        while (bytes_read > 0) {
            bytes_written = write(dst_fd, out_ptr, bytes_read);
            if (bytes_written < 0) {
                perror("write");
                close(src_fd);
                close(dst_fd);
                exit(EXIT_FAILURE);
            }
            bytes_read -= bytes_written;
            out_ptr += bytes_written;
        }
    }
    if (bytes_read < 0) {
        perror("read");
    }
    close(src_fd);
    close(dst_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    copy_file(argv[1], argv[2]);
    return 0;
}
