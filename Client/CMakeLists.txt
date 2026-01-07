cmake_minimum_required(VERSION 3.25)
project(hotslug C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /home/marker_/slugknob/)
set(SOURCES
        src/slug.h
        src/main.c
        src/bigdigit.c
        src/lever.c
        src/display.c
        src/hotslug.c
        src/vibe.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        pigpio     # -lpigpio
        rt         # -lrt
        pthread    # -lpthread
        curl       # -lcurl
        cjson      # -lcjson
)
target_include_directories(${PROJECT_NAME} PRIVATE src)