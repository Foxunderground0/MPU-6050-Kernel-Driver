make
sudo insmod MPU6050_device_driver.ko
sudo rmmod MPU6050_device_driver
dmesg | tail -n 10