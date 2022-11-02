#include "touchpad_detect.h"
#include <fstream>
#include <regex>
#include "easylogging++.h"

std::tuple<uint8_t, uint8_t>  detect_touchpad(){

    std::ifstream devices{"/proc/bus/input/devices"};
    const std::regex i2c_touchpad_regex{"^S:.*i2c-(\\d+).*ASUE.*"};
    const std::regex eventid_regex{"^H:.*event(\\d+).*"};
    const std::regex str_expr{"^N: Name=\"ASUE.*Touchpad\"$"};
    std::smatch matches;
    bool touchpad_section_found{false};
    uint8_t i2c_bus{0};
    uint8_t event_id{0};

    for( std::string line; getline( devices, line ); )
    {

        if (std::regex_match (line,str_expr)){
            touchpad_section_found = true;
        }

        if (touchpad_section_found && std::regex_match(line, matches, i2c_touchpad_regex)) {
            
            if(matches.size()!=2){
                LOG(FATAL) << "Error while parsing touchpad information, despite an Asus touchpad was found.";
                exit(1);
            }
            i2c_bus = std::stoi(matches[1].str());
            LOG(INFO) << "Found Asus i2c bus id: " << unsigned(i2c_bus);
            continue;
        }   
        if(touchpad_section_found && std::regex_match(line,matches, eventid_regex)){
            event_id = std::stoi(matches[1].str());
            LOG(INFO) << "Found Asus event handler id: " << unsigned(event_id);
            break;
            
        }
    }
    return{i2c_bus,event_id};
}

