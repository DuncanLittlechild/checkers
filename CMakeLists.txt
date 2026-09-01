cmake_minimum_required(VERSION 3.24)
project(GPU_TEST)
set(CMAKE_CXX_STANDARD 23)
# Toggle to set whether this is a prod build (ON) or debug (OFF)
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

#####################################
##### STATIC LINK RUNTIME LIBS ######
#####################################

# Statically links the c and c++ runtime libraries. This is needed for game development, if the binaries
# are to be distributed on windows, as dynamically linking the c and c++ runtime libraries is sketchy and
# very version dependent on windows.
if(MSVC)
       set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
       set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
       add_compile_options(/arch:AVX2) # Ensures SIMD optimizations. Remove if triggers errors on apple device
else()
       add_link_options(-static-libstdc++ -static-libgcc)
       add_compile_options(-mavx2 -mfma) # Ensures SIMD optimizations. Remove if triggers errors on apple device
endif()


############################
##### LIBRARY OPTIONS ######
############################

# -- Set Library options --
# Forces SDL3_ttf to use the vendored versions of its libraries found in its./external folder instead of system ones
# Note that this only works if you remembered to clone the external dependencies, in this case the vendored libraries
# This may be unnecessary, especially if you have freetype installed on your system somewhere it can be easily located,
# eg if your system is linux and not windows. 
set(SDLTTF_VENDORED ON CACHE BOOL "Forces SDL3_ttf to use the vendored versions of its libraries" FORCE)
# Disables the Harfbuzz external dependency. This is by far the largest dependency, and only needed for non-latin fonts
# with unusual glyph formation (eg, arabic, where letters look different depending on where they are in the word) or
# right to left font rendering.
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "Disables the plutosvg external dependency" FORCE)
set(SDLTTF_PLUTOVG OFF CACHE BOOL "Disables the plutovg external dependency" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "Disables the Harfbuzz external dependency" FORCE)

######################
##### LIBRARIES ######
######################

# -- Add Libraries --
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3-3.4.12)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3_ttf-3.2.2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb_image)

# AS imgui has no cmake, it must be compiled and statically linked manually
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui-1.92.8")
add_library(imgui_lib STATIC
       ${IMGUI_DIR}/imgui.cpp
       ${IMGUI_DIR}/imgui_draw.cpp
       ${IMGUI_DIR}/imgui_demo.cpp
       ${IMGUI_DIR}/imgui_tables.cpp
       ${IMGUI_DIR}/imgui_widgets.cpp
       ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
       ${IMGUI_DIR}/backends/imgui_impl_sdlgpu3.cpp
        # Add/remove backends to match what you actually use
)
target_include_directories(imgui_lib PUBLIC
       ${IMGUI_DIR}
       ${IMGUI_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC SDL3::SDL3)

# Adds stb as a header-only library
add_library(stb_header INTERFACE)
target_include_directories(stb_header INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/")

########################################
##### EXECUTABLE INCLUDES AND SRC ######
########################################

# --- Your executable ---
#Bundles togetehr all cpp files in src recursively, so they can be added as an executable all as one
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME}
    "${MY_SOURCES}"
)

# -- Include headers --
#Path for include headers
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")


####################################
##### PREPROCESSOR DIRECTIVES ######
####################################

# -- Preprocessor directives creation --
if (PRODUCTION_BUILD)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC RESOURCES_PATH="./resources/")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC PRODUCTION_BUILD=1)
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC PRODUCTION_BUILD=0)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SHADER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    set(SHADERS_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
endif()
# Creates the RESOURCES_PATH preprocessor macro which directs all file loading operations to the resources file


############################
##### DISABLE CONSOLE ######
############################

if(MSVC) # If using the VS compiler...
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		#YOU CAN REMOVE THE CONSOLE WITH THIS LINE!
		set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	endif()

elseif(MINGW) # Windows / MinGW GCC/Clang
	if(PRODUCTION_BUILD)
		# Remove console on Windows under MinGW
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
			-mwindows
		)
	endif()

elseif(APPLE OR UNIX) # macOS / Linux
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -s)
	endif()
endif()
###############################
##### Shader compilation ######
###############################
macro(compile_shaders)
       file(GLOB_RECURSE src_shader_files
              RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/shaders
              CONFIGURE_DEPENDS
              "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl"
       )
       find_program(SC_EXECUTABLE shadercross REQUIRED)
       foreach(fn ${src_shader_files})
              set(in_f "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${fn}")
              set(out_f "${SHADERS_OUTPUT_DIR}/${fn}")
              string(REPLACE ".hlsl" ".spv" fn_spv ${out_f})
              string(REPLACE ".hlsl" ".msl" fn_msl ${out_f})
              string(REPLACE ".hlsl" ".dxil" fn_dxil ${out_f})
              add_custom_command(
                     OUTPUT ${fn_spv} ${fn_msl} ${fn_dxil}
                     COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_spv}
                     COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_msl}
                     COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_dxil}
                     MAIN_DEPENDENCY ${in_f}
                     VERBATIM
              )
              list(APPEND out_shader_files ${fn_spv})
              list(APPEND out_shader_files ${fn_msl})
              list(APPEND out_shader_files ${fn_dxil})
       endforeach()
       add_custom_target(${PROJECT_NAME}_shaders DEPENDS ${out_shader_files})
       add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)

endmacro()

compile_shaders()

####################
##### LINKING ######
####################

# -- Final linking step --
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        imgui_lib
        stb_header
        stb_image
)
