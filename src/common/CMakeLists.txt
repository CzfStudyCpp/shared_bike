CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#获得所有在指定路径下的源文件的文件名，将输出结果存在指定的变量中
aux_source_directory(. SOURCE_COMMON_FILES)

#构建库供他人模块使用
ADD_LIBRARY(common ${SOURCE_COMMON_FILES})
#用来显式地定义变量
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

#指定头文件
INCLUDE_DIRECTORIES(../../third/include)

#需要连接的库文件
LINK_DIRECTORIES(../../third/lib/iniparser)
LINK_DIRECTORIES(../../third/lib/log4cpp)

TARGET_LINK_LIBRARIES(common iniparser)
TARGET_LINK_LIBRARIES(common log4cpp)
TARGET_LINK_LIBRARIES(common pthread)
TARGET_LINK_LIBRARIES(common dl)