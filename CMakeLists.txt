cmake_minimum_required(VERSION 3.15)


project(numberpad VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_NO_DEFAULT_LOG_FILE -DELPP_DISABLE_DEBUG_LOGS")



find_package(PkgConfig)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

include_directories(${LIBEVDEV_INCLUDE_DIRS} 
    include
)



add_executable(numberpad 
    src/main.cpp
    src/i2c_device.cpp
    src/numberpad.cpp
    src/touchpad_detect.cpp
    src/layout.cpp
    src/easylogging++.cc
)


target_link_libraries(numberpad ${LIBEVDEV_LIBRARIES})



install(TARGETS numberpad
    DESTINATION "/usr/bin"
)

install(FILES "systemd/numberpad.service"
        DESTINATION "/lib/systemd/system"
)


install(FILES "config/layouts.json"
        DESTINATION "/etc/numberpad"
)


set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/control/postinst;${CMAKE_SOURCE_DIR}/control/prerm;")


set(CPACK_PACKAGE_NAME ${PROJECT_NAME})

SET(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Asus Numberpad userspace driver")
set(CPACK_PACKAGE_VENDOR "haronaut")

set(CPACK_VERBATIM_VARIABLES YES)

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/dist")

set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/bin")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_CONTACT "haronaut@gmx.at")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "haronaut")

#for auto resolving deps set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libevdev2,libi2c0")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

set(CPACK_DEB_COMPONENT_INSTALL YES)

include(CPack)
