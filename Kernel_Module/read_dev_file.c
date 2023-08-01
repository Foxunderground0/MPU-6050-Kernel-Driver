#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define DEVICE_FILE "/dev/mpu6050"

int main() {
    int fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
        perror("Error opening device file");
        return 1;
    }

    uint16_t data;
    ssize_t bytes_read;

    // Read multiple 16-bit values from the device file
    while ((bytes_read = read(fd, &data, sizeof(uint16_t))) == sizeof(uint16_t)) {
        // Process the 16-bit value received from the device
        // For example, you can print it in hexadecimal format:
        printf("Received value: 0x%04x\n", data);
    }

    close(fd);
    return 0;
}
