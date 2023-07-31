#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/module.h>

#define LOG(x) printk("%s", x) // Helper logger macro
#define ELOG(x) {printk("%s", x); return 1;} // Helper error logger macro
#define I2C_BUS_ADAPTER_NUMBER 1              // I2C Bus number on the Raspberry Pi
#define SLAVE_DEVICE_NAME "MPU6050"       // Device and Driver Name
#define MPU6050_SLAVE_ADDR 0x68             // MPU6050 Slave Address

static struct i2c_adapter* i2c_adapter = NULL;  // I2C Adapter Structure
static struct i2c_client* i2c_client_mpu = NULL;  // I2C Cient Structure (In our case it is MPU)

//I2C_BOARD_INFO is a macro, we use it to set all the necessary values of the struct
static struct i2c_board_info mpu_i2c_board_info = {
		I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MPU6050_SLAVE_ADDR)
};


//This function is ran when the device is first detected / probbed
static int mpu_probe(struct i2c_client* client, const struct i2c_device_id* id) {
	LOG("MPU Probbed");
	return 0;
}

/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static void mpu_remove(struct i2c_client* client) {
	LOG("MPU Removed");
	//return 0;
}

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

// This gets called when the module is loaded
static int __init mpu_driver_init(void) {
	LOG("MPU6050 driver loaded\n");
	i2c_adapter = i2c_get_adapter(I2C_BUS_ADAPTER_NUMBER);

	if (i2c_adapter != NULL) {
		i2c_client_mpu = i2c_new_client_device(i2c_adapter, &mpu_i2c_board_info);

		if (i2c_client_mpu != NULL) {
			i2c_add_driver(&mpu_driver);
		} else {
			ELOG("Couldnt create mew device")
		}

		i2c_put_adapter(i2c_adapter);
		LOG("Success\n");
	} else {
		ELOG("Couldnt obtain i2c adapter")
	}

	return 0;
}

// This gets called when the module is unloaded
static void __exit mpu_driver_exit(void) {
	LOG("MPU6050 driver unloaded\n");
	return;
}

module_init(mpu_driver_init);
module_exit(mpu_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Foxunderground0");
MODULE_DESCRIPTION("MPU6050 I2C Driver");
MODULE_VERSION("1.0");
