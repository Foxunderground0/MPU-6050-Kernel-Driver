#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

int addr = 0x68; /* The I2C address of the MPU */
int file;

unsigned char readMPU(unsigned char reg);  // Function prototype
unsigned char writeMPU(unsigned char reg, unsigned char val);  // Function prototype

int main() {
	int adapter_nr = 1;
	char filename[20];
	sprintf(filename, "/dev/i2c-1");
	file = open(filename, O_RDWR);

	writeMPU(0x6b, 0x00);
	writeMPU(0x1c, 0x00);
	writeMPU(0x1a, 0x00);
	printf("%X, %X, %X\n", readMPU(0x6b), readMPU(0x1c), readMPU(0x1a));

	if (file < 0) {
		printf("[Error] Failed to open the I2C bus\n");
		return 1;
	}

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		printf("[Error] Unable to set the I2C address\n");
		return 1;
	}

	while (1) {
		signed int data = ((readMPU(0x3f) << 8) | readMPU(0x40));
		if (data > 32767){
			data -= 65536;
		}
		//data = data / 16384.0;
		printf("Val: %f\n", (float)data/16384.0);
	}

	for (unsigned int i = 0x00; i < 0x76; i++){
		//writeMPU(i, 0xff);
		printf("Val: %X form reg %X\n", readMPU(i), i);
	}

	close(file); // Close the opened I2C device
	return 0;
}


unsigned char writeMPU(unsigned char reg, unsigned char val) {
	unsigned char res;

	/* Using SMBus command */
	res = i2c_smbus_write_byte_data(file, reg, val);
	if (res < 0) {
		printf("Writing: %X at register %X failed\n", val, reg);
	} else {
		//printf("%d", res);
		return res;
	}

}


unsigned char readMPU(unsigned char reg) {
	unsigned char res;

	/* Using SMBus command */
	res = i2c_smbus_read_byte_data(file, reg);
	if (res < 0) {
		printf("Reading From: %X failed\n", reg);
	} else {
		//printf("%d", res);
		return res;
	}

}
