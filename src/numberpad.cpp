#include <numberpad.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <math.h>
#include <exception>
#include "easylogging++.h"

Numberpad::Numberpad(uint8_t bus, uint8_t device_address, uint8_t event_id, const Layout& layout) : 
m_i2c_device{bus, device_address}, m_event_id{event_id}, m_layout{layout} {}

Numberpad::~Numberpad()
{

    libevdev_free(m_dev_touchpad);
    libevdev_free(m_dev_uniput_dummy);
    libevdev_uinput_destroy(m_dev_uinput);
}

void Numberpad::createUinputDevice()
{

    int err;

    LOG(INFO)  << "creating uinput device\n";
    m_dev_uniput_dummy = libevdev_new();

    libevdev_set_name(m_dev_uniput_dummy, "numberpad");

    libevdev_enable_event_type(m_dev_uniput_dummy, EV_KEY);
    libevdev_enable_event_type(m_dev_uniput_dummy, EV_SYN);
    libevdev_enable_event_code(m_dev_uniput_dummy, EV_KEY, KEY_NUMLOCK, NULL);

    for(const int key : m_layout.key_bindings){
        libevdev_enable_event_code(m_dev_uniput_dummy, EV_KEY, key, NULL);
    }
    
    
    err = libevdev_uinput_create_from_device(m_dev_uniput_dummy,
                                             LIBEVDEV_UINPUT_OPEN_MANAGED,
                                             &m_dev_uinput);
    usleep(100000);
    if (err != 0)
    {    
        throw std::runtime_error("Unable to create uinput device.");
    }
    LOG(INFO) << "Successfully created uinput device.";
}

void Numberpad::deactivateNumlock()
{

    libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_NUMLOCK, 0);
    libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
}
void Numberpad::activateNumlock()
{

    libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_NUMLOCK, 1);
    libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
}

void Numberpad::connect()
{

    m_i2c_device.connect();

    int rc, fd_touchpad;
    std::string file_evdev_event = "/dev/input/event" + std::to_string(m_event_id);

    fd_touchpad = open(file_evdev_event.c_str(), O_RDONLY);
    if (fd_touchpad < 0) {
        throw std::runtime_error("unable to open touchpad event file.");
    }

    rc = libevdev_new_from_fd(fd_touchpad, &m_dev_touchpad);
    if (rc != 0){
        throw std::runtime_error("Unable to create libevdev object from file descriptor.");
    }

    createUinputDevice();

    const struct input_absinfo *absinfo_x = libevdev_get_abs_info(m_dev_touchpad, ABS_X);
    const struct input_absinfo *absinfo_y = libevdev_get_abs_info(m_dev_touchpad, ABS_Y);

    m_x_min = absinfo_x->minimum;
    m_x_max = absinfo_x->maximum;
    m_y_min = absinfo_y->minimum;
    m_y_max = absinfo_y->maximum;
    LOG(INFO) <<"touchpad x_min: "<<m_x_min << " x_max: "<< m_x_max <<" y_min: "<<m_y_min << " y_max: "<< m_y_max;
}

void Numberpad::enable()
{
    LOG(INFO) << "Enabling numberpad LED";
    libevdev_grab(m_dev_touchpad, LIBEVDEV_GRAB);
    m_i2c_device.transfer({0x05, 0x00, 0x3d, 0x03, 0x06, 0x00, 0x07, 0x00, 0x0d, 0x14, 0x03, Brightness::HIGH, 0xad});
    activateNumlock();
    m_pad_is_active = true;
}
void Numberpad::disable()
{
    LOG(INFO) << "Disabling numberpad LED";
    libevdev_grab(m_dev_touchpad, LIBEVDEV_UNGRAB);
    m_i2c_device.transfer({0x05, 0x00, 0x3d, 0x03, 0x06, 0x00, 0x07, 0x00, 0x0d, 0x14, 0x03, 0x00, 0xad});
    deactivateNumlock();
    m_pad_is_active = false;
}

void Numberpad::close()
{
    m_i2c_device.transfer({0x05, 0x00, 0x3d, 0x03, 0x06, 0x00, 0x07, 0x00, 0x0d, 0x14, 0x03, 0x00, 0xad});
}

void Numberpad::triggerKey(int x, int y)
{

    if (!m_layout.withinKeyboard(x,y)){
        LOG(INFO) << "Touch event not within keyboard layout";
        return;
    }


    LOG(DEBUG) << " key: " << libevdev_event_code_get_name(EV_KEY,  m_layout.getKey(x, y));

    if ( m_layout.getKey(x, y) == KEY_5)
    {
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_LEFTSHIFT, 1);
        libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
        usleep(10000);
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_5, 1);
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_5, 0);
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY, KEY_LEFTSHIFT, 0);
        libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
    }
    else
    {
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY, m_layout.getKey(x, y), 1);
        libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
        libevdev_uinput_write_event(m_dev_uinput, EV_KEY,  m_layout.getKey(x, y), 0);
        libevdev_uinput_write_event(m_dev_uinput, EV_SYN, SYN_REPORT, 0);
    }
}

void Numberpad::processInput(const struct input_event &ev)
{

    static int x = 0, y = 0, x_down = 0, y_down, x_up = 0, y_up = 0;

    if (ev.type == EV_ABS)
    {
        if (ev.code == ABS_MT_POSITION_X)
        {
            x = ev.value;
        }
        if (ev.code == ABS_MT_POSITION_Y)
        {
            y = ev.value;
        }
    }
    else if (ev.type == EV_KEY && ev.code == BTN_TOOL_FINGER)
    {
 
        if (ev.value == 1)
        {
            x_down = x;
            y_down = y;
        }
        
        else if (ev.value == 0)
        {
            if (m_layout.numlockPressed(x,y))
            {
                if (m_pad_is_active) {
                    disable();
                }
                else {
                    enable();
                }
            }
            else
            {
                x_up = x;
                y_up = y;
                // touchpad keys here
                if (m_pad_is_active)
                {

                    if (sqrt(pow(x_up - x_down, 2.0) + pow(y_up - y_down, 2.0)) < 40.0)
                    {
                        triggerKey(x, y);
                    }
                }
            }
        }
    }
}

void Numberpad::waitForInput(libevdev *dev)
{

    LOG(INFO) << "Waiting for touchpad input...";
    int rc = -1;

    for (;;)
    {
        struct input_event ev;

        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC)
        {

            do
            {
                struct input_event ev;
                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            } while (rc == LIBEVDEV_READ_STATUS_SYNC);
        }
        else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
        {

            LOG(DEBUG) << "Event type: " << libevdev_event_type_get_name(ev.type)
                      << " code: " << libevdev_event_code_get_name(ev.type, ev.code)
                      << " value: " << ev.value;
            processInput(ev);
        }
        else if (rc == -EAGAIN)
        {
        }
        else
        {
            throw std::runtime_error("Error while reading evdev events. Error code: " + std::string(strerror(-rc)));
        }
    };
}

void Numberpad::run()
{

    while (true)
    {
        waitForInput(m_dev_touchpad);
    }
}