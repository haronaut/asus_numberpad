
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string>
#include "touchpad_detect.h"
#include "numberpad.h"
#include "easylogging++.h"
#include "layout.h"
#include "cxxopts.hpp"


INITIALIZE_EASYLOGGINGPP


namespace fs = std::filesystem;

int main(int argc, char** argv){


    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "%datetime %level: %msg");

    //el::Loggers::reconfigureLogger("default", defaultConf);


    cxxopts::Options options("Numberpad", "A ASUS Numberpad driver");
    std::string modelname;
    fs::path configfile_path;

    options.add_options()
        ("f,file", "touchpad config file path", cxxopts::value<fs::path>())
//        ("v,verbose", "Enable verbose output", cxxopts::value<bool>()->default_value("false"))
        ("m,model", "The notebook model name", cxxopts::value<std::string>())
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);


    if (result.count("help") || argc <= 1)
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }


    if (result.count("f")){

        configfile_path = result["f"].as<fs::path>();
 
        if(!fs::exists(configfile_path)){
            LOG(ERROR) << "touchpad config file: " << configfile_path << " not found. Exiting.";
            exit(1);
        } 
    }
    else{
        LOG(ERROR) << "no touchpad config file provided. Exiting.";
        exit(1);
    }

    //LayoutReader reader{"../config/layouts.json"};
    LayoutReader reader{configfile_path};


    if (result.count("m")){

        modelname = result["m"].as<std::string>();
 
        if(!reader.isModelSupported( modelname )){
        LOG(ERROR) << "Provided modelname not supported.\nSupported models are:\n" << reader.getSupportedModelsPretty();
        exit(1);
    }
    }
    else{
        LOG(ERROR) << "no notebook model provided. Exiting.";
        exit(1);
    }  


    auto [i2c_address, event_id] = detect_touchpad();
    Numberpad numberpad{i2c_address, 0x15, event_id, reader.getModelLayout(modelname)};
    numberpad.connect();
    numberpad.run();
    
    return 0;
}


