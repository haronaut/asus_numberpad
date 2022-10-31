#ifndef LAYOUT_H
#define LAYOUT_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include "json.hpp"
#include <fstream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <ostream>

using json = nlohmann::json;

struct Layout{

    Layout() = default;

    size_t rows;
    size_t cols;

    int top_left_x;
    int top_left_y; 
    int botton_right_x;
    int botton_right_y;
    
    int width;
    int height;

    int numlock_top_left_x;
    int numlock_top_left_y;
    int numlock_bottom_right_x;
    int numlock_bottom_right_y;

    int key_sector_width;
    int key_sector_height;

    std::vector<int> key_bindings;

    friend std::ostream& operator<<(std::ostream& os, const Layout& dt);

    bool withinKeyboard(int touchpad_x, int touchpad_y) const;
    bool numlockPressed(int touchpad_x, int touchpad_y) const;
    int getKey(int touchpad_x, int touchpad_y) const;
};



class LayoutReader{

    public:
        LayoutReader(const std::string& path);
        std::vector<std::string> getSupportedModels() const;
        std::string getSupportedModelsPretty() const;
        Layout getModelLayout(const std::string& modelname) const;
        bool isModelSupported(const std::string& modelname) const;
    private:
        json m_config;
};



#endif //LAYOUT_H