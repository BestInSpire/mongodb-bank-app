cmake_minimum_required(VERSION 3.10)
project(BankApp)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
include(ExternalProject)
ExternalProject_Add(mongo-cxx-driver
        GIT_REPOSITORY https://github.com/mongodb/mongo-cxx-driver.git
        GIT_TAG releases/stable
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
        -DBUILD_SHARED_LIBS=ON
        -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/install
        BUILD_COMMAND cmake --build . --target EP_mongo-cxx-driver
        INSTALL_COMMAND cmake --build . --target install
)

ExternalProject_Get_Property(mongo-cxx-driver install_dir)
set(MONGO_CXX_DRIVER_INCLUDE_DIR ${install_dir}/include)
set(MONGO_CXX_DRIVER_LIB_DIR ${install_dir}/lib)

include_directories(${MONGO_CXX_DRIVER_INCLUDE_DIR})
link_directories(${MONGO_CXX_DRIVER_LIB_DIR})

add_executable(BankApp main.cpp)

add_dependencies(BankApp mongo-cxx-driver)

target_link_libraries(BankApp PRIVATE mongocxx bsoncxx)
