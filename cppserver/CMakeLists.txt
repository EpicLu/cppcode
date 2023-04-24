# cpp
project(server)

# 配置构建类型
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# 配置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -lmysqlclient")
set(CMAKE_CXX_STANDARD 17)

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
set(TARGET webserver)
# 设置库名
set(PROJECT_LINK_LIBS http pool timer)

# 设置头文件目录
include_directories(include)

# 设置源文件目录 使用GLOB可使用wilecard来获取源文件
#file(GLOB THREADPOOL_SOURCES "src/*.cpp")
#set(THREADPOOL_SOURCES "src/threadpool.cpp" "src/task.cpp")
file(GLOB HTTP_SOURCES "src/http/*.cc")
file(GLOB BUFFER_SOURCES "src/buffer/*.cc")
file(GLOB LOG_SOURCES "src/log/*.cc")
file(GLOB POOL_SOURCES "src/pool/*.cc")
file(GLOB SERVER_SOURCES "src/server/*.cc")
file(GLOB TIMER_SOURCES "src/timer/*.cc")
set(MAIN_SOURCES "src/main.cc")



# 制作静态库 如果动态库已经使用过的名字静态库不能在add_library中使用 可用设置版本指令修改名字
#add_library(event_static STATIC ${EVENT_SOURCES})
#SET_TARGET_PROPERTIES(event_static PROPERTIES OUTPUT_NAME event) # 静态库改名 输出libevent.a

# cmake 在构建一个新的target 时，会尝试清理掉其他使用这个名字的库，如果没有清理还是会只会构建一个动态库，不会构建出静态库
#SET_TARGET_PROPERTIES(event_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 制作动态库
add_library(pool SHARED ${POOL_SOURCES})
SET_TARGET_PROPERTIES(pool PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本
add_library(server SHARED ${SERVER_SOURCES})
SET_TARGET_PROPERTIES(server PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本
add_library(timer SHARED ${TIMER_SOURCES})
SET_TARGET_PROPERTIES(timer PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本
add_library(log SHARED ${LOG_SOURCES})
SET_TARGET_PROPERTIES(log PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本
add_library(buffer SHARED ${BUFFER_SOURCES})
SET_TARGET_PROPERTIES(buffer PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本
add_library(http SHARED ${HTTP_SOURCES})
SET_TARGET_PROPERTIES(http PROPERTIES VERSION 1.0 SOVERSION 1) # 设置动态库版本

message(${PROJECT_LINK_LIBS})

# add_executable用于生成执行文件的地方
add_executable(${TARGET} ${MAIN_SOURCES})	
 
# 生成可执行文件main，需要把连接库给带上，添加库文件名不需要指定文件路径，cmake会自己找
#target_link_libraries(reactor INTERFACE threadpool)
#target_link_libraries(webserver PUBLIC threadpool)
target_link_libraries(log PUBLIC buffer)
target_link_libraries(pool PUBLIC log)
target_link_libraries(pool PUBLIC mysqlclient)
target_link_libraries(server PUBLIC ${PROJECT_LINK_LIBS})
target_link_libraries(${TARGET} server) 

# set the C++ standard to the appropriate standard for using auto
target_compile_features(${PROJECT_NAME} PUBLIC cxx_auto_type)

# Print the list of known compile features for this version of CMake
message("List of compile features: ${CMAKE_CXX_COMPILE_FEATURES}")

#set the location for library installation --i.e.
#/usr/lib in this case
#not really necessary in this example, use "sudo make install" to apply
#install(TARGETS testStudent DESTINATION /usr/lib)