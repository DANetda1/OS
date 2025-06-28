#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_NAME_LEN 256
#define WORK_DIR "symlink_depth_test"

int main() {
    char target[MAX_NAME_LEN] = "a";
    char newlink[MAX_NAME_LEN];
    int depth = 0;

    if (mkdir(WORK_DIR, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }

    if (chdir(WORK_DIR) == -1) {
        perror("chdir");
        return 1;
    }

    int fd = open("a", O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("open base file");
        return 1;
    }
    write(fd, "test", 4);
    close(fd);

    while (1) {
        snprintf(newlink, MAX_NAME_LEN, "link_%d", depth);
        if (symlink(target, newlink) == -1) break;

        int fd = open(newlink, O_RDONLY);
        if (fd == -1) break;
        close(fd);

        strncpy(target, newlink, MAX_NAME_LEN);
        depth++;
    }

    printf("%d\n", depth);

    for (int i = 0; i < depth; ++i) {
        snprintf(newlink, MAX_NAME_LEN, "link_%d", i);
        unlink(newlink);
    }
    unlink("a");
    chdir("..");
    rmdir(WORK_DIR);

    return 0;
}
