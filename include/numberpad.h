#ifndef NUMBERPAD_H
#define NUMBERPAD_H

#include <string>
#include <i2c_device.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>
#include "layout.h"


enum Brightness: uint8_t{

    OFF = 0x00,
    LOW = 0x01,
    MID = 0x18,
    HIGH = 0x10
};


class Numberpad{

    

public:
    Numberpad(uint8_t bus, uint8_t device_address, uint8_t event_id, const Layout& layout);
    ~Numberpad();
    void connect();
    void close();
    void run();

private:
    void triggerKey(int x, int y);
    void waitForInput(libevdev* dev);
    void processInput(const struct input_event& ev);
    void enable();
    void disable();
    void createUinputDevice();
    void deactivateNumlock();
    void activateNumlock();

    I2CDevice m_i2c_device;
    uint8_t m_event_id;
    struct libevdev *m_dev_touchpad;
    struct libevdev *m_dev_uniput_dummy;
    struct libevdev_uinput *m_dev_uinput;
    int m_x_min;
    int m_x_max;
    int m_y_min;
    int m_y_max;
    bool m_pad_is_active{false};
    const Layout m_layout;
};


#endif