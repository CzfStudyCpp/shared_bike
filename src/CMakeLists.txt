CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(shared_bike)

INCLUDE_DIRECTORIES(../third/include)
INCLUDE_DIRECTORIES(./common)

LINK_DIRECTORIES(../third/lib/iniparser)
LINK_DIRECTORIES(../third/lib/log4cpp)
LINK_DIRECTORIES(./common)

#内置变量，定义了顶级CMakeLists.txt所在的文件夹，定义了包含最近的project（）命令的CMakeLists,txt所在的文件夹
aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

ADD_EXECUTABLE(shared_bike ${SOURCE_FILES})

#用来显式地定义变量
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

TARGET_LINK_LIBRARIES(shared_bike iniparser)
#TARGET_LINK_LIBRARIES(shared_bike log4cpp)
TARGET_LINK_LIBRARIES(shared_bike pthread)
TARGET_LINK_LIBRARIES(shared_bike common)
TARGET_LINK_LIBRARIES(shared_bike liblog4cpp.a)
#子目录
ADD_SUBDIRECTORY(common)

SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
INSTALL (TARGETS shared_bike DESTINATION bin)
