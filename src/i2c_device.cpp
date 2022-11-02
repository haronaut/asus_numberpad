#include "i2c_device.h"
#include <iostream>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <unistd.h>
#include <vector>


I2CDevice::I2CDevice(uint8_t bus, unsigned char device_address):
    m_bus{bus},
    m_device_address(device_address){}

void I2CDevice::connect(){
    
    const std::string i2c_file = "/dev/i2c-" + std::to_string(m_bus);
    if ((m_fd_device = open(i2c_file.c_str(),O_RDWR)) == -1){
        throw std::runtime_error("unable to open i2c device file.");
    }
    else{
        
        int addr = 0x15; 
        int rc = 0; 
        //Force to use the device i2c address as it is already used by a kernel module
        if ((rc = ioctl(m_fd_device, I2C_SLAVE_FORCE, addr)) < 0) {
            throw std::runtime_error("failed to enable forced ioctl on i2c device.");
        }
    }

}


void I2CDevice::transfer(const std::vector<uint8_t>& data){

                        
    int rc;
    if ((rc = write(m_fd_device, data.data(), data.size())) != 13) {
        throw std::runtime_error("unable to write to i2c bus.");
    }
}