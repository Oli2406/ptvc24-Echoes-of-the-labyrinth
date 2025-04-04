cmake_minimum_required(VERSION 3.22)

if(APPLE)
    # Fix runtime path for installed executables
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
endif()

project(executable1_echoes_of_the_labyrinth)

if(MSVC)
    # https://docs.microsoft.com/en-us/previous-versions/958x11bc(v=vs.140)
    add_compile_options("$<$<CONFIG:Debug>:/MTd>$<$<CONFIG:Release>:/MT>")
    add_compile_options("$<$<CONFIG:Debug>:/ZI>$<$<CONFIG:Release>:/Zi>")
    add_compile_options(/FS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Qpar")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
set(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/physx")

if(UNIX AND NOT APPLE)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux" ".")
elseif(UNIX AND APPLE)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/macos" ".")
elseif(WIN32)
    set(LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows" ".")
endif()

find_package(OpenGL REQUIRED)

# ImGui source files
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")
file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/imgui_impl_opengl3.cpp"
)

# Include ImGui headers
set(IMGUI_INCLUDE_DIRS
    "${IMGUI_DIR}"
)

set(LINK_LIBRARIES
    debug glfw3d optimized glfw3
    debug glew32sd optimized glew32s
    debug GCG_GL_Lib_Debug optimized GCG_GL_Lib_Release
    debug assimp-vc143-mtd optimized assimp-vc143-mtd
    debug freetype optimized freetype
    debug BulletCollision_Debug optimized BulletCollision_RelWithDebugInfo
    debug BulletDynamics_Debug optimized BulletDynamics_RelWithDebugInfo
    debug LinearMath_Debug optimized LinearMath_RelWithDebugInfo
    debug PhysXCommon_64_debug optimized PhysXCommon_64
    debug PhysX_64_debug optimized PhysX_64
    debug PhysXFoundation_64_debug optimized PhysXFoundation_64
    debug PhysXExtensions_static_64_debug optimized PhysXExtensions_static_64
    debug PhysXPvdSDK_static_64_debug optimized PhysXPvdSDK_static_64
    debug PhysXCooking_64_debug optimized PhysXCooking_64
    debug PhysXCharacterKinematic_static_64_debug optimized PhysXCharacterKinematic_static_64
    OpenGL::GL
)

message(LINK_LIBRARIES=${LINK_LIBRARIES})

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} "src/ImGuiHUD.h" "src/puzzle.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBRARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

# IDE specific settings
if(CMAKE_GENERATOR MATCHES "Visual Studio")
   set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
   set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
elseif(CMAKE_GENERATOR MATCHES "Xcode")
   set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_GENERATE_SCHEME TRUE CMAKE_XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
   set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY XCODE_STARTUP_PROJECT ${PROJECT_NAME})
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
endif()

install(
    TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug DESTINATION "debug"
)
install(
    TARGETS ${PROJECT_NAME} CONFIGURATIONS Release DESTINATION "release"
)
