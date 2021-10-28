#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

struct account{
    int id;
    char username[100];
    char password[100];
    int active;
};

void main(){
    struct account admin = {1, "admin", "123", 1};
    struct account user = {1, "user", "123", 1};
    struct account agent = {1, "agent", "123", 1};

    int fd = open("adminDB", O_CREAT|O_EXCL|O_WRONLY, 0666);
    write(fd, &admin, sizeof(admin));
    close(fd);

    fd = open("agentDB", O_CREAT|O_EXCL|O_WRONLY, 0666);
    write(fd, &agent, sizeof(agent));
    close(fd);

    fd = open("userDB", O_CREAT|O_EXCL|O_WRONLY, 0666);
    write(fd, &user, sizeof(user));
    close(fd);

    fd = open("trainsDB", O_CREAT|O_EXCL, 0666);
    close(fd);

    fd = open("bookingsDB", O_CREAT|O_EXCL, 0666);
    close(fd);
}