#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

int main() {
	int file;
	int adapter_nr = 1; /* probably dynamically determined */
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	file = open(filename, O_RDWR);
	if (file < 0) {
		perror("Failed to open the I2C bus");
		return 1;
	}

	// Further code for I2C communication and transactions would follow here

	int addr = 0x68; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */	
		return 1;
	}

	printf("Made it this far 1 \n");

	__u8 reg = 0x75; /* Device register to access */
	__u8 res;
	char buf[10];

	/* Using SMBus commands */
	res = i2c_smbus_read_word_data(file, reg);
	if (res < 0) {
		printf("SMB Failed \n");
		/* ERROR HANDLING: I2C transaction failed */
	} else {
		printf("Made It this far 2 \n");
		/* res contains the read word */
		printf("%d", res);
	}

	close(file); // Close the opened I2C device
	return 0;
}
