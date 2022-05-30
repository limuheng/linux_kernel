#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    FILE *file = NULL;
    FILE *file2 = NULL;
    char buf[128];
    
    // test for minor 0
    file = fopen("/dev/hank0", "w+");

    if (file == NULL) {
        printf("[Test][%d] Failed to open /dev/hank0\n", getpid());
        return -1;
    }

    fread(buf, sizeof(buf), 1, file);
    printf("[Test][%d] Read /dev/hank0: %s\n", getpid(), buf);

    // clear buffer
    memset(buf, 0x0, sizeof(buf));

    strncpy(buf, "Hello, Hank!\n", 12);
    printf("[Test][%d] Write /dev/hank0: %s\n", getpid(), buf);
    fwrite(buf, sizeof(buf), 1, file);

    // clear buffer
    memset(buf, 0x0, sizeof(buf));

    fread(buf, sizeof(buf), 1, file);
    printf("[Test][%d] Read /dev/hank0: %s\n", getpid(), buf);

    fclose(file);

    // test for minor 1
    file2 = fopen("/dev/hank1", "w+");

    if (file2 == NULL) {
        printf("[Test][%d] Failed to open /dev/hank1\n", getpid());
        return -1;
    }

    fread(buf, sizeof(buf), 1, file2);
    printf("[Test][%d] Read /dev/hank1: %s\n", getpid(), buf);

    strncpy(buf, "Hello, Hank2!\n", 13);
    printf("[Test][%d] Write /dev/hank1: %s\n", getpid(), buf);
    fwrite(buf, sizeof(buf), 1, file2);

    fread(buf, sizeof(buf), 1, file2);
    printf("[Test][%d] Read /dev/hank1: %s\n", getpid(), buf);

    fclose(file2);

    return 0;
}

