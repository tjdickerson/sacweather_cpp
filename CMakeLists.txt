cmake_minimum_required(VERSION 3.19)
project(sacweather)

set(CMAKE_CXX_STANDARD 14)

set(sacw-src-dir sacwlib/)
set(imgui_dir ${sacw-src-dir}imgui/)
set(resources_dir resources/)

file(COPY ${resources_dir} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

include_directories(${sacw-src-dir})
include_directories(windows/)
include_directories(${imgui_dir})


add_executable(sacweather

        WIN32

        windows/win32_main.cpp
        windows/win32_menu.cpp

        ${sacw-src-dir}sacw_api.h
        ${sacw-src-dir}sacw_main.cpp

        ${sacw-src-dir}tjd_shapefile.cpp
        ${sacw-src-dir}tjd_shapefile.h

        ${sacw-src-dir}tjd_share.h

        ${sacw-src-dir}tjd_gl_render.cpp
        ${sacw-src-dir}tjd_gl.h

        ${sacw-src-dir}render_extern.h
        ${sacw-src-dir}sacw_extern.h

        ${sacw-src-dir}tjd_conversions.h
        ${sacw-src-dir}tjd_conversions.cpp

        ${sacw-src-dir}tjd_radar.cpp
        ${sacw-src-dir}tjd_radar.h

        ${sacw-src-dir}nws_info.h
        ${sacw-src-dir}nws_info.cpp

        sacwlib/tjd_level2.cpp sacwlib/tjd_level2.h

        ${sacw-src-dir}tjd_ui.cpp
        ${sacw-src-dir}tjd_ui.h
        ${sacw-src-dir}tjd_font.cpp

#        ${sacw-src-dir}tjd_ftp.cpp
#        ${sacw-src-dir}tjd_ftp.h


        ${sacw-src-dir}glad/glad.h
        ${sacw-src-dir}glad.c
        ${sacw-src-dir}KHR/khrplatform.h


        ${imgui_dir}imgui.cpp
        ${imgui_dir}imgui_demo.cpp
        ${imgui_dir}imgui_draw.cpp
        ${imgui_dir}imgui_impl_opengl3.cpp
        ${imgui_dir}imgui_impl_win32.cpp
        ${imgui_dir}imgui_tables.cpp
        ${imgui_dir}imgui_widgets.cpp

        ${imgui_dir}imconfig.h
        ${imgui_dir}imgui.h
        ${imgui_dir}imgui_internal.h
        ${imgui_dir}imstb_rectpack.h
        ${imgui_dir}imstb_textedit.h
        ${imgui_dir}imstb_truetype.h
        ${imgui_dir}imgui_impl_opengl3.h
        ${imgui_dir}imgui_impl_win32.h

        sacwlib/tjd_level3.cpp
        sacwlib/tjd_level3.h
        windows/tjd_win32_download.cpp
        windows/tjd_win32_download.h
        sacwlib/tjd_product_desc.h)


# Build BZip2
file(GLOB
        bzip2_src
        "/Users/tjdic/cpplibs/source/bzip2-1.0.8/*.c"
        "/Users/tjdic/cpplibs/source/bzip2-1.0.8/*.cpp"
        "/Users/tjdic/cpplibs/source/bzip2-1.0.8/*.h")

add_library(lib-bz2 STATIC ${bzip2_src})


find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        lib-bz2

        # this is needed by windows api stuff
        user32.lib
        gdi32.lib
        UxTheme.lib

        # for windows downloading stuff
        Wininet.lib
        urlmon.lib

        # this is needed by ftp stuff
        #urlmon.lib
        #ws2_32.lib

        # these are needed by ImGui
        imm32.lib
        dwmapi.lib
        )