project(Qt3DOpenVDBRenderer)
cmake_minimum_required(VERSION 3.02)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#OpenVDB does not like qt's "foreach"
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake
  )

find_package(OpenVDB REQUIRED)
find_package(TBB REQUIRED)
find_package(Half REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt53DInput REQUIRED)
find_package(Qt53DQuick REQUIRED)
find_package(Qt53DRender REQUIRED)
find_package(Qt53DQuickRender REQUIRED)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qopenvdbgrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qopenvdbgridpointsurfacegeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/openvdbpointsvertexdatagenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/openvdbgenmethod_voxelcenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/openvdbgenmethod_marchingcubescenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/openvdbgenmethod_surrounding.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/qopenvdbreader.cpp
  )
set(HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qopenvdbgrid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qopenvdbgridpointsurfacegeometry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/qopenvdbreader.h
  )

set(BUILD_EXAMPLE ON CACHE BOOL "Build the example project")

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif(BUILD_EXAMPLE)

add_library(Qt3DOpenVDBRenderer ${SOURCE} ${HEADER})

target_include_directories(Qt3DOpenVDBRenderer PUBLIC  ${PROJECT_SOURCE_DIR}/include/
                                               PRIVATE ${PROJECT_SOURCE_DIR}/src/ )
target_link_libraries(Qt3DOpenVDBRenderer ${OPENVDB_LIBRARIES} ${TBB_LIBRARIES} ${HALF_LIBRARIES} )

qt5_use_modules(Qt3DOpenVDBRenderer Qml Quick 3DCore 3DQuick 3DRender 3DQuickRender)
