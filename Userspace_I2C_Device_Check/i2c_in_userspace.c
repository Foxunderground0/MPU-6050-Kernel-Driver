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

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);

	file = open(filename, O_RDWR);

	if (file < 0) {
		printf("[Error] Failed to open the I2C bus\n");
		return 1;
	}

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		printf("[Error] Unable to set the i2c address\n");
		return 1;
	}


	printf("%d\n", readMPU(0x23));
	writeMPU(0x23, 0x00);
	printf("%d\n", readMPU(0x23));

	close(file); // Close the opened I2C device
	return 0;
}



unsigned char writeMPU(unsigned char reg, unsigned char val) {
	unsigned char res;

	/* Using SMBus command */
	res = i2c_smbus_write_byte_data(file, reg, val);
	if (res < 0) {
		printf("Writing: %d at register %d failed\n", val, reg);
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
		printf("Reading From: %d failed\n", reg);
	} else {
		//printf("%d", res);
		return res;
	}

}
