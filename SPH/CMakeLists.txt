CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

# Name of the project
PROJECT(SPH)


FILE(GLOB_RECURSE SOURCES "src/*.cpp")
FILE(GLOB_RECURSE HEADERS "src/*.h")
FILE(GLOB_RECURSE GLSL "resources/*.glsl")


# Set the executable.
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
SET(GLM_INCLUDE_DIR "include/glm-0.9.9.8")
IF(NOT GLM_INCLUDE_DIR)
	# The environment variable was not set
	SET(ERR_MSG "Please point the environment variable GLM_INCLUDE_DIR to the root directory of your GLM installation.")
	IF(WIN32)
		# On Windows, try the default location
		MESSAGE(STATUS "Looking for GLM in ${DEF_DIR_GLM}")
		IF(IS_DIRECTORY ${DEF_DIR_GLM})
			MESSAGE(STATUS "Found!")
			SET(GLM_INCLUDE_DIR ${DEF_DIR_GLM})
		ELSE()
			MESSAGE(FATAL_ERROR ${ERR_MSG})
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR ${ERR_MSG})
	ENDIF()
ENDIF()
INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIR})


SET(ENTT_DIR "include/entt-3.10.3")
IF(NOT ENTT_DIR)
	# The environment variable was not set
	SET(ERR_MSG "Please point the environment variable ENTT to the root directory of your ENTT.")
	MESSAGE(FATAL_ERROR ${ERR_MSG})
ENDIF()
INCLUDE_DIRECTORIES(${ENTT_DIR}/single_include)
SET(QUEUE_DIR "include")
INCLUDE_DIRECTORIES(${QUEUE_DIR})


# SET(IMGUI_DIR "include/imgui-1.88")

# target_sources( ${CMAKE_PROJECT_NAME}
#                 PRIVATE
#                     ${IMGUI_DIR}/imgui_demo.cpp
#                     ${IMGUI_DIR}/imgui_draw.cpp
#                     ${IMGUI_DIR}/imgui_tables.cpp
#                     ${IMGUI_DIR}/imgui_widgets.cpp
#                     ${IMGUI_DIR}/imgui.cpp

#                 PRIVATE
#                     ${IMGUI_DIR}/imgui_impl_opengl3.cpp
#                     ${IMGUI_DIR}/imgui_impl_glfw.cpp
#                 )

# target_include_directories( ${CMAKE_PROJECT_NAME}
#                             PUBLIC ${IMGUI_DIR}
#                             )

# Get the GLFW environment variable. There should be a CMakeLists.txt in the 
# specified directory.
SET(GLFW_DIR "include/glfw-3.3.6")
IF(NOT GLFW_DIR)
	# The environment variable was not set
	SET(ERR_MSG "Please point the environment variable GLFW_DIR to the root directory of your GLFW installation.")
	IF(WIN32)
		# On Windows, try the default location
		MESSAGE(STATUS "Looking for GLFW in ${DEF_DIR_GLFW}")
		IF(IS_DIRECTORY ${DEF_DIR_GLFW})
			MESSAGE(STATUS "Found!")
			SET(GLFW_DIR ${DEF_DIR_GLFW})
		ELSE()
			MESSAGE(FATAL_ERROR ${ERR_MSG})
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR ${ERR_MSG})
	ENDIF()
ENDIF()
OPTION(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
OPTION(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
OPTION(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
IF(CMAKE_BUILD_TYPE MATCHES Release)
	ADD_SUBDIRECTORY(${GLFW_DIR} ${GLFW_DIR}/release)
ELSE()
	ADD_SUBDIRECTORY(${GLFW_DIR} ${GLFW_DIR}/debug)
ENDIF()
INCLUDE_DIRECTORIES(${GLFW_DIR}/include)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

# Get the GLEW environment variable.
IF(WIN32)
	SET(GLEW_DIR "include/glew-2.2.0")
ELSE()
	SET(GLEW_DIR $ENV{GLEW_DIR})
ENDIF()
IF(NOT GLEW_DIR)
	# The environment variable was not set
	SET(ERR_MSG "Please point the environment variable GLEW_DIR to the root directory of your GLEW installation.")
	IF(WIN32)
		# On Windows, try the default location
		MESSAGE(STATUS "Looking for GLEW in ${DEF_DIR_GLEW}")
		IF(IS_DIRECTORY ${DEF_DIR_GLEW})
			MESSAGE(STATUS "Found!")
			SET(GLEW_DIR ${DEF_DIR_GLEW})
		ELSE()
			MESSAGE(FATAL_ERROR ${ERR_MSG})
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR ${ERR_MSG})
	ENDIF()
ENDIF()
INCLUDE_DIRECTORIES(${GLEW_DIR}/include)





IF(WIN32)
	# With prebuilt binaries
	# Check for 32 vs 64 bit generator
	IF(NOT CMAKE_CL_64)
		MESSAGE(STATUS "Using 32Bit")
		TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
	ELSE()
		MESSAGE(STATUS "Using 64Bit")
		FIND_LIBRARY(GLEW_LIB glew32s.lib ${GLEW_DIR}/lib/Release/x64)
		TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLEW_LIB})
	ENDIF()
ELSE()
	TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLEW_DIR}/lib/libGLEW.a)
ENDIF()

# Use c++17
SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD 17)


# OS specific options and libraries
IF(WIN32)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
	TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} opengl32.lib)
	SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})
ELSE()
	# Enable all pedantic warnings.
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
	IF(APPLE)
		# Add required frameworks for GLFW.
		TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
	ELSE()
		#Link the Linux OpenGL library
		TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} "GL")
	ENDIF()
ENDIF()
