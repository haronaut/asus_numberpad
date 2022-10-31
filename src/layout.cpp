#include "layout.h"
#include "easylogging++.h"
#include <math.h>


bool Layout::numlockPressed(int touchpad_x, int touchpad_y) const{

    return touchpad_x > numlock_top_left_x && touchpad_x < numlock_bottom_right_x && \
            touchpad_y > numlock_top_left_y && touchpad_y < numlock_bottom_right_y;
}

bool Layout::withinKeyboard(int touchpad_x, int touchpad_y) const{

    return touchpad_x > top_left_x && touchpad_x < botton_right_x && \
        touchpad_y > top_left_y && touchpad_y < botton_right_y;
}

int Layout::getKey(int touchpad_x, int touchpad_y) const{

    int sector_x_idx = floor((touchpad_x - top_left_x) / key_sector_width);
    int sector_y_idx = floor((touchpad_y - top_left_y) / key_sector_height);

    //LOG(INFO) << "getKeyName: " << libevdev_event_code_get_name(EV_KEY, key_bindings[sector_y_idx * cols + sector_x_idx]);
    
    return key_bindings[sector_y_idx * cols + sector_x_idx];
}

LayoutReader::LayoutReader(const std::string& config_path){
    std::ifstream layout_file{config_path};
    m_config = json::parse(layout_file);
}

std::vector<std::string> LayoutReader::getSupportedModels() const{

    std::vector<std::string> supported_models;
    for (const auto& model : m_config.items())
    {
        supported_models.push_back(model.key());
    }

    return supported_models;
}

std::string LayoutReader::getSupportedModelsPretty() const{

    std::string supported_models_pretty{""};

    for(const std::string& model : getSupportedModels()){
        supported_models_pretty += model + '\n';
    }
    return supported_models_pretty;
}


bool LayoutReader::isModelSupported(const std::string& modelname) const{

    auto models = getSupportedModels();
    return std::find(models.begin(), models.end(), modelname) != models.end();
}

std::ostream& operator<<(std::ostream &output, const Layout& layout){

    output << layout.cols<< " " <<layout.rows;
    std::cout<<layout.rows;
    return output;
}


Layout LayoutReader::getModelLayout(const std::string& modelname) const{


    Layout layout;
    auto model_data = m_config[modelname];
    layout.width = model_data["width"];
    layout.height = model_data["height"];
    layout.cols = model_data["key_bindings"][0].size();
    layout.rows = model_data["key_bindings"].size();
    layout.botton_right_x = model_data["bottom_right"]["x"];
    layout.botton_right_y = model_data["bottom_right"]["y"];
    layout.top_left_x = model_data["top_left"]["x"];
    layout.top_left_y = model_data["top_left"]["y"];
    layout.numlock_top_left_x = model_data["numlock"]["top_left"]["x"];
    layout.numlock_top_left_y = model_data["numlock"]["top_left"]["y"];
    layout.numlock_bottom_right_x = model_data["numlock"]["bottom_right"]["x"];
    layout.numlock_bottom_right_y = model_data["numlock"]["bottom_right"]["y"];

    layout.key_sector_width = floor((layout.botton_right_x - layout.top_left_x) / layout.cols);
    layout.key_sector_height = floor((layout.botton_right_y - layout.top_left_y) / layout.rows);

    layout.key_bindings.resize(layout.rows * layout.cols);

    for(size_t i = 0; i < layout.rows; ++i){
        for(size_t j = 0; j < layout.cols; ++j){
            std::string keyname = model_data["key_bindings"][i][j];
            layout.key_bindings[i * layout.cols + j] = libevdev_event_code_from_name(EV_KEY, keyname.c_str());
        }
    }
    return layout;
}


