#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

long long getCurrentTimeMicroseconds() {
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
    return (long long)currentTime.tv_sec * 1000000LL + currentTime.tv_nsec / 1000LL;
}


#define DEVICE_FILE "/dev/mpu6050"

int main() {
    long long startTime, endTime, elapsedTimeMicroseconds;

    // Record the start time
    int fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
        perror("Error opening device file");
        return 1;
    }

    int data;
    long long count = 0;
    ssize_t bytes_read;

    // Read multiple 16-bit values from the device file
    startTime = getCurrentTimeMicroseconds();

    while ((bytes_read = read(fd, &data, sizeof(int))) == sizeof(int) && (getCurrentTimeMicroseconds() - startTime) <= 1000000) {
        //printf("Val: %f\n", (float)data / 16384.0);
        count++;
    }

    // Record the end time
    endTime = getCurrentTimeMicroseconds();
    close(fd);

    // Calculate the elapsed time in microseconds
    elapsedTimeMicroseconds = endTime - startTime;

    // Print the result
    printf("Elapsed Time: %lld microseconds | Number of samples: %lld samples\n", elapsedTimeMicroseconds, count);

    return 0;
}
