#cpp
project(cppstudy)

# 配置构建类型
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# 配置编译选项
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 11)

# 设置二进制文件目录 默认为当前目录的build -> ./build
# SET(PROJECT_BINARY_DIR ../)
# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 设置动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 设置所有库文件目录
#SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 设置可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# 设置可执行文件名
set(TARGET main)
# 设置库名
#set(PROJECT_LINK_LIBS threadpool http)

# 设置头文件目录
include_directories(include)

# 设置源文件目录 使用GLOB可使用wilecard来获取源文件
#file(GLOB _SOURCES "src/*.cpp")
set(MAIN_SOURCES "src/main.cpp")

# 制作静态库 如果动态库已经使用过的名字静态库不能在add_library中使用 可用设置版本指令修改名字
#add_library(event_static STATIC ${EVENT_SOURCES})
#SET_TARGET_PROPERTIES(event_static PROPERTIES OUTPUT_NAME event) # 静态库改名 输出libevent.a

# cmake 在构建一个新的target 时，会尝试清理掉其他使用这个名字的库，如果没有清理还是会只会构建一个动态库，不会构建出静态库
#SET_TARGET_PROPERTIES(event_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 制作动态库
#add_library(event SHARED ${EVENT_SOURCES})
#SET_TARGET_PROPERTIES(event PROPERTIES VERSION 1.2 SOVERSION 1) # 设置动态库版本

message(${PROJECT_LINK_LIBS})

# add_executable用于生成执行文件的地方
add_executable(${TARGET} ${MAIN_SOURCES})	
 
# 生成可执行文件main，需要把连接库给带上，添加库文件名不需要指定文件路径，cmake会自己找
#target_link_libraries(slave PUBLIC event)
#target_link_libraries(${TARGET} ${PROJECT_LINK_LIBS}) 

#set the location for library installation --i.e.
#/usr/lib in this case
#not really necessary in this example, use "sudo make install" to apply
#install(TARGETS testStudent DESTINATION /usr/lib)