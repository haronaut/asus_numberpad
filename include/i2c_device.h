
#include <string>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <vector>
#include "easylogging++.h"


class I2CDevice{

public:
    I2CDevice(uint8_t bus, unsigned char device_address);
    ~I2CDevice() = default;
    void connect();
    void transfer(const std::vector<uint8_t>& data);

private:
    uint8_t m_bus;
    unsigned char m_device_address;
    int m_fd_device;

};