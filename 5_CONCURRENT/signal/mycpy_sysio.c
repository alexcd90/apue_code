#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define BUFSIZE 1024

int main(int argc, char **argv) {
    int sfd, dfd;
    char buf[BUFSIZ];
    int len, pos, ret = 0;
    if (argc < 3) {
        fprintf(stderr, "Usage:.....\n");
        exit(1);
    }

    do {
        sfd = open(argv[1], O_RDONLY);
        if (sfd < 0) {
            if (errno != EINTR) {
                perror("open():sfd");
                exit(1);
            }
        }
    } while (sfd < 0);

    do {
        dfd = open(argv[2], O_WRONLY | O_CREAT, O_TRUNC, 0600);
        if (dfd < 0){
            if (errno != EINTR){
                perror("open():dfd");
                exit(1);
            }
        }
    } while (dfd < 0);

    while(1) {
        len = read(sfd, buf, BUFSIZ);
        if (len < 0){
            if (errno == EINTR){
                continue;
            }
            perror("read()");
            exit(1);
        }

        if (len == 0) break;

        pos = 0;
        while(len > 0){
            ret = write(dfd, buf+pos, len);
            if (ret < 0){
                if (errno == EINTR){
                    continue;
                }
                perror("write()");
                exit(1);
            }

            pos += ret;
            len -= ret;

        }
    }

    close(dfd);
    close(sfd);

    exit(0);
}

