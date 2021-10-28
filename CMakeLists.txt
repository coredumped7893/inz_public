cmake_minimum_required(VERSION 3.17)
project(FlightSystem)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(OpenCV_DIR "/usr/local/lib/cmake/opencv4/")
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories(inc)
include_directories(src)
include_directories(src/Control)
include_directories(src/DataInput)
include_directories(src/FlightManager)
include_directories(src/ImageProcessing)
include_directories(src/Util)
include_directories(/usr/local/include/opencv2/)


add_executable(FlightSystem
        inc/bitarray.h
        inc/dllist.h
        inc/fileutils.h
        inc/graph.h
        inc/libinetsocket.h
        inc/libunixsocket.h
        inc/llist.h
        inc/minunit.h
        inc/queue.h
        inc/stack.h
        inc/stringlib.h
        inc/timing.h
        src/Control/control.c
        src/Control/control.h
        src/Control/hardwareOut.c
        src/Control/hardwareOut.h
        src/Control/simulatedOut.c
        src/Control/simulatedOut.h
        src/DataInput/dataInput.c
        src/DataInput/dataInput.h
        src/DataInput/hardwareInput.c
        src/DataInput/hardwareInput.h
        src/DataInput/xPlane.c
        src/DataInput/xPlane.h
        src/FlightManager/app.c
        src/FlightManager/app.h
        src/FlightManager/route.c
        src/FlightManager/route.h
        src/ImageProcessing/imageProcessing.c
        src/ImageProcessing/imageProcessing.h
        src/Maps/maps.c
        src/Maps/maps.h
        src/Util/buff_tools.c
        src/Util/buff_tools.h
        src/Util/message.c
        src/Util/message.h
        src/config.h
        src/FlightManager/main.c
        src/FlightManager/main.h src/FlightManager/PID.c src/FlightManager/PID.h src/Aircraft/prk2.h src/Aircraft/sf50.h src/ImageProcessing/cvParseFrameStrategy.cpp src/ImageProcessing/cvParseFrameStrategy.h src/ImageProcessing/cvParseFrameStrategy.hpp src/Util/other.c src/Util/other.h src/ImageProcessing/imageProcessingStrategy.h src/Tests/testsr1.c src/Tests/testsr2.cpp src/Tests/testUtilsr1.c src/Tests/testAppr1.c src/Aircraft/aircraft.c src/Aircraft/aircraft.h src/Control/controlStrategy.h src/Tests/testDataInputr1.c src/FlightManager/collisionModule.c src/FlightManager/collisionModule.h src/FlightManager/collisionAvoidanceStrategy.h src/Maps/terrain.c src/Maps/terrain.h src/Util/route_utils.c src/Util/route_utils.h src/Util/time_diff.c src/Util/time_diff.h src/Maps/kd_tree.c src/Maps/kd_tree.h src/Cameras/camDataStrategy.h src/Cameras/cameras.cpp src/Cameras/cameras.h src/Cameras/virtualSim.cpp src/Cameras/virtualSim.h src/Cameras/hardwareCameraDrivers.cpp src/Cameras/hardwareCameraDrivers.h src/Util/proc.c src/Util/proc.h src/Util/profiler.c src/Util/profiler.h)

