#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "chardemo.h"

int main(int argc, char *argv[])
{
    char cmd[50];
    int fd, ret = 0;
    unsigned long buffer_size;
    char *buffer;
    const char *str = "hello";

    if (argc > 2) {
        fprintf(stderr, "Too much arguments. There's only one optional argument buffer_size.\n");
        return 1;
    } else if (argc == 2) {
        buffer_size = atol(argv[1]);
        sprintf(cmd, "sudo modprobe chardemo buffer_size=%s", argv[1]);
    } else {
        buffer_size = 4096;
        sprintf(cmd, "sudo modprobe chardemo");
    }

    buffer = malloc(buffer_size);
    system(cmd);
    
    fd = open("/dev/chardemo", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "failure on opening /dev/chardemo\n");
        ret = 1;
        goto out;
    }

    write(fd, str, strlen(str) + 1);
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, strlen(str) + 1);
    printf("read/write at offset 0: %s\n", !strcmp(str, buffer) ? "OK" : "FAILED");

    lseek(fd, -10, SEEK_END);
    write(fd, str, strlen(str) + 1);
    lseek(fd, -(strlen(str) + 1), SEEK_CUR);
    read(fd, buffer, strlen(str) + 1);
    printf("read/write at offset %lu: %s\n", buffer_size - 10, !strcmp(str, buffer) ? "OK" : "FAILED");

    ioctl(fd, CHARDEMO_MEM_CLEAR);
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, buffer_size);
    int i, cleared = 1;
    for(i = 0; i != buffer_size; ++i)
        if (buffer[i])
            cleared = 0;
    printf("ioctl mem_clear: %s\n", cleared ? "OK" : "FAILED");

    close(fd);
out:
    system("sudo modprobe -r chardemo");
    free(buffer);

    return ret;
}
