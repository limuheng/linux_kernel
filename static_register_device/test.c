#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    FILE *file = NULL;
    char buf[128];
    int i = 0, sum = 0;

    file = fopen("/dev/hank0", "w+");

    if (file == NULL) {
        printf("[Test][%d] Failed to open /dev/hank0\n", getpid());
        return -1;
    }

    fread(buf, sizeof(buf), 1, file);
    printf("[Test][%d] Read /dev/hank0: %s\n", getpid(), buf);

    strncpy(buf, "Hello, Hank!\n", 12);
    printf("[Test][%d] Write /dev/hank0: %s\n", getpid(), buf);
    fwrite(buf, sizeof(buf), 1, file);

    fread(buf, sizeof(buf), 1, file);
    printf("[Test][%d] Read /dev/hank0: %s\n", getpid(), buf);

    fclose(file);

    return 0;
}

