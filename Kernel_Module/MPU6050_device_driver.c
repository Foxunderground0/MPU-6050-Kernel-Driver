#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define LOG(x) printk("%s", x) // Helper logger macro
#define ELOG(x) {printk("%s", x); return 1;} // Helper error logger macro
#define I2C_BUS_ADAPTER_NUMBER 1              // I2C Bus number on the Raspberry Pi
#define SLAVE_DEVICE_NAME "MPU6050"       // Device and Driver Name
#define MPU6050_SLAVE_ADDR 0x68             // MPU6050 Slave Address


static struct cdev mpu_cdev;
static dev_t mpu_dev;
static struct class* mpu_class;
static struct device* mpu_device;

static struct i2c_adapter* i2c_adapter = NULL;  // I2C Adapter Structure
static struct i2c_client* i2c_client_mpu = NULL;  // I2C Cient Structure (In our case it is MPU)

//I2C_BOARD_INFO is a macro, we use it to set all the necessary values of the struct
static struct i2c_board_info mpu_i2c_board_info = {
		I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MPU6050_SLAVE_ADDR)
};

static int mpu_write(uint8_t* buf, uint8_t len) {
	int ret = i2c_master_send(i2c_client_mpu, buf, 2);
	return ret;
}

static int mpu_read(uint8_t* out_buf, uint8_t len) {
	int ret = i2c_master_recv(i2c_client_mpu, out_buf, len);
	return ret;
}

// Function to read the values from registers 0x3F and 0x40 of MPU6050
int mpu_read_registers(void) {
	uint8_t buf[2];
	int data = 0;

	// Read register 0x3F
	buf[0] = 0x3F;
	if (i2c_master_send(i2c_client_mpu, buf, 1) < 0) {
		printk(KERN_ERR "Failed to send register address 0x3F\n");
		return -EIO;
	}
	if (i2c_master_recv(i2c_client_mpu, buf, 1) < 0) {
		printk(KERN_ERR "Failed to read from register 0x3F\n");
		return -EIO;
	}
	uint8_t register3F_value = buf[0];

	// Read register 0x40
	buf[0] = 0x40;
	if (i2c_master_send(i2c_client_mpu, buf, 1) < 0) {
		printk(KERN_ERR "Failed to send register address 0x40\n");
		return -EIO;
	}
	if (i2c_master_recv(i2c_client_mpu, buf, 1) < 0) {
		printk(KERN_ERR "Failed to read from register 0x40\n");
		return -EIO;
	}
	uint8_t register40_value = buf[0];

	// Combine the values into a single 16-bit integer
	data = ((register3F_value << 8) | register40_value);
	if (data > 32767) {
		data -= 65536;
	}

	return data;
}

//This function is ran when the device is first detected / probbed
static int mpu_probe(struct i2c_client* client, const struct i2c_device_id* id) {
	LOG("MPU Probed");

	//Set Poewer registers to enable readings
	uint8_t buf[2] = { 0x6b, 0 };
	mpu_write(buf, 2);
	return 0;
}

static void mpu_remove(struct i2c_client* client) {
	LOG("MPU Removed");
	//return 0;
}

static int mpu_open_file(struct inode* inode, struct file* filp) {
	// Add your open operation implementation here
	return 0;
}

static int mpu_release_file(struct inode* inode, struct file* filp) {
	// Add your release operation implementation here
	return 0;
}

static ssize_t mpu_read_file(struct file* filp, char __user* buf, size_t count, loff_t* f_pos) {
	int data = mpu_read_registers();

	// Copy the 16-bit value to the user-space buffer 'buf'
	if (copy_to_user(buf, &data, sizeof(int)) != 0) {
		// Failed to copy data to user space
		return -EFAULT; // Return the error code for "Bad address"
	}
	//LOG("Read");
	// Update the file position to indicate the next position to read (if needed)
	// For example, if you want to read a different value on the next read, you can update *f_pos here

	return sizeof(int); // Return the number of bytes read (should be 2 bytes for a 16-bit value)
}

static ssize_t mpu_write_file(struct file* filp, const char __user* buf, size_t count, loff_t* f_pos) {
	// Add your write operation implementation here
	return count; // Return the number of bytes written
}

static struct file_operations mpu_fops = {
	.owner = THIS_MODULE,
	.open = mpu_open_file,
	.release = mpu_release_file,
	.read = mpu_read_file,
	.write = mpu_write_file,
};

// Structure that has slave device id
static const struct i2c_device_id mpu_id[] = {
	{ SLAVE_DEVICE_NAME, 0 },
	{ }
};

// This is a macro to add the mpuid to the devices that the i2c subsystem probes for
MODULE_DEVICE_TABLE(i2c, mpu_id);

// The structure that holds the functions that the driver calls
static struct i2c_driver mpu_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = mpu_probe,
	.remove = mpu_remove,
	.id_table = mpu_id,
};

static int create_cdev_file(void) {
	// Allocate a range of character device numbers (major and minor numbers)
	if (alloc_chrdev_region(&mpu_dev, 0, 1, "mpu6050") < 0) {
		ELOG("Failed to allocate character device numbers");
		return -1;
	}

	// Initialize the character device structure
	cdev_init(&mpu_cdev, &mpu_fops);
	mpu_cdev.owner = THIS_MODULE;

	// Add the character device to the kernel
	if (cdev_add(&mpu_cdev, mpu_dev, 1) < 0) {
		ELOG("Failed to add character device");
		unregister_chrdev_region(mpu_dev, 1);
		return -1;
	}

	// Create a class for the device file in /sys/class
	mpu_class = class_create(THIS_MODULE, "mpu6050");
	if (IS_ERR(mpu_class)) {
		ELOG("Failed to create device class");
		cdev_del(&mpu_cdev);
		unregister_chrdev_region(mpu_dev, 1);
		return PTR_ERR(mpu_class);
	}

	// Create a device file for the character device
	mpu_device = device_create(mpu_class, NULL, mpu_dev, NULL, "mpu6050");
	if (IS_ERR(mpu_device)) {
		ELOG("Failed to create device file");
		class_destroy(mpu_class);
		cdev_del(&mpu_cdev);
		unregister_chrdev_region(mpu_dev, 1);
		return PTR_ERR(mpu_device);
	}

	return 0;
}

// This gets called when the module is loaded
static int __init mpu_driver_init(void) {
	LOG("MPU6050 driver loaded\n");
	i2c_adapter = i2c_get_adapter(I2C_BUS_ADAPTER_NUMBER);
	if (i2c_adapter != NULL) {
		i2c_client_mpu = i2c_new_client_device(i2c_adapter, &mpu_i2c_board_info);

		if (i2c_client_mpu != NULL) {
			if (i2c_add_driver(&mpu_driver) < 0) {
				ELOG("Error adding the driver");
			} else {
				i2c_put_adapter(i2c_adapter);
				LOG("Success\n");
				create_cdev_file();
			}
		} else {
			ELOG("Couldnt create mew device")
		}
	} else {
		ELOG("Couldnt obtain i2c adapter")
	}

	return 0;
}

// This gets called when the module is unloaded
static void __exit mpu_driver_exit(void) {
	LOG("MPU6050 driver unloaded\n");

	// Destroy the device file
	device_destroy(mpu_class, mpu_dev);

	// Destroy the device class
	class_destroy(mpu_class);

	// Remove the character device
	cdev_del(&mpu_cdev);

	// Unregister the character device numbers
	unregister_chrdev_region(mpu_dev, 1);

	// Remove the i2c driver from the i2c subsystem
	i2c_del_driver(&mpu_driver);

	// Remove the i2c client from the kernel
	if (i2c_client_mpu != NULL) {
		i2c_unregister_device(i2c_client_mpu);
	}

	return;
}

module_init(mpu_driver_init);
module_exit(mpu_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Foxunderground0");
MODULE_DESCRIPTION("MPU6050 I2C Driver");
MODULE_VERSION("1.0");
