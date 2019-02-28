set(PROJECT_NAME "WebM_Practice")
set(EXECUTABLE_NAME "WebM_Player")
# Пути к исходникам
set(SRC_FOLDERS
    src
)
# Конфиги используемых библиотек
set(LIBS_FOLDER 
    libs/
)
set(LIBS_FOLDERS
    libyuv
    glm
    glfw
    libnestegg
)
set(LIBS_LINK_NAMES
    libnestegg
    libyuv
    glfw
)
set(LIBS_DEPENCY_NAMES
    libnestegg
    libyuv
    glfw
    glm
)
set(LIBS_INCLUDE_FOLDERS
    libnestegg/include
    libyuv/include
    libvpx
    glm/glm
    glfw/include
    glfw/deps
)


####################################################
# Настройка CMake сборки
####################################################
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
if (NOT CMAKE_VERSION VERSION_LESS "3.1")
	cmake_policy(SET CMP0054 NEW)
endif()

# цвета
option(CMAKE_COLOR_MAKEFILE "Color makefile" ON)

# Инклюдим файлик с разными хелперами для сборки
include("CMake/Helpers.cmake")

# Если у нас есть компилятор clang на юникс - используем его
if (UNIX)
    message("********* Unix compilation check *********")
    # Используем ccache (если есть) для ускорения сборки
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message("-> ССache will be used for compilation")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)

    # Вариант с путями
    # execute_process(COMMAND which clang OUTPUT_VARIABLE CLANG_PATH)
    # execute_process(COMMAND which clang++ OUTPUT_VARIABLE CLANGXX_PATH)
    # if(NOT ("${CLANG_PATH}" STREQUAL ""))
    find_program(CLANG_FOUND clang)
    if(CLANG_FOUND)
        message("-> Clang will be used for compilation")
        set(CMAKE_C_COMPILER "clang")
        set(CMAKE_CXX_COMPILER "clang++")
        # Скрываем ворнинги на clang + цвета
        set(CUSTOM_COMPILER_C_FLAGS "-Wno-error=unused-command-line-argument -Qunused-arguments -fcolor-diagnostics")
        set(CUSTOM_COMPILER_CXX_FLAGS "-Wno-error=unused-command-line-argument -Qunused-arguments -fcolor-diagnostics")
    endif(CLANG_FOUND)
    message("**********************************")
endif()

# Инициализация проекта
project(${PROJECT_NAME} C CXX)

enable_testing()

####################################################
# Библиотека GLEW REQUIRED
####################################################
# find_package(GLEW)
if(NOT GLEW_FOUND)   # если не нашли, то добавляем из исходников
    message("GLEW lib not found, build from sources")
    set (CMAKE_C_FLAGS "${CUSTOM_COMPILER_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CUSTOM_COMPILER_CXX_FLAGS}")
    add_subdirectory(${LIBS_FOLDER}/glew)
    set(LIBS_DEPENCY_NAMES ${LIBS_DEPENCY_NAMES} libglew)	# То, что указывается в project
    include_directories(${LIBS_FOLDER}/glew/include)
    set(LIBS_LINK_FOLDERS "${LIBS_LINK_FOLDERS} ${CMAKE_BINARY_DIR}/lib")
    message("CMAKE GLEW FOLDER = ${CMAKE_BINARY_DIR}/lib")
    set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} libglew)		# то, что указывается в add_library
    add_definitions(-DGLEW_STATIC)
else()
    include_directories(${GLEW_INCLUDE_DIRS})
    set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} ${GLEW_LIBRARIES})
endif()

####################################################
# Конфигурация библиотеки libvpx
####################################################
# Определение платформы для оптимальной сборки
if(APPLE)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_static/OSX/include")
    set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} "${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_static/OSX/lib/libvpx.a")
elseif(UNIX AND NOT WIN32 AND NOT APPLE)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_static/Linux-x64/include")
    set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} "${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_static/Linux-x64/lib/libvpx.a")
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_src/Sources" "${CMAKE_BINARY_DIR}/libvpx")
    include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_src/Sources" "${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx_src/SourcesConfigureFiles")
    set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} "libvpx")

    # file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/libs/libvpx" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/libs")

    # # Пути и зависимости
    # set(LIBS_DEPENCY_NAMES ${LIBS_DEPENCY_NAMES} libvpx)
    # set(LIBS_LINK_NAMES ${LIBS_LINK_NAMES} "${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx/compileResult/lib/libvpx.a")

    # set(LIBVPX_TARGET "generic-gnu")
    # find_program(YASM_FOUND yasm)  # YASM assembler для кодека
    # if(YASM_FOUND)
    #     if (APPLE)
    #         set(LIBVPX_TARGET "x86_64-darwin15-gcc")
    #     elseif (UNIX AND NOT WIN32 AND NOT APPLE)
    #         set(LIBVPX_TARGET "x86_64-linux-gcc")
    #     elseif (MSYS)
    #         set(LIBVPX_TARGET "x86_64-win64-gcc")
    #     elseif (WIN32)
    #         set(LIBVPX_TARGET "x86-win32-gcc")
    #     endif()
    # endif(YASM_FOUND)

    # # Конфигурация кодека
    # set(LIBVPX_CONFIGURE_PARAMS "--target=${LIBVPX_TARGET}" "--enable-pic" "--disable-install-docs" "--disable-install-bins" "--disable-unit-tests" "--disable-docs" "--disable-examples" "--prefix=${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx/compileResult")
    # include(ExternalProject)
    # ExternalProject_Add(libvpx
    #                     PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx"
    #                     SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx"
    #                     CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx/configure" ${LIBVPX_CONFIGURE_PARAMS}
    #                     INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx/compileResult"
    #                     BUILD_IN_SOURCE 1
    #                     BUILD_COMMAND "make"
    #                     INSTALL_COMMAND make install
    # )
    # include_directories(${CMAKE_CURRENT_BINARY_DIR}/libs/libvpx/compileResult/include)
endif()


####################################################
# Библиотеки собираемые из исходников
####################################################
# Прописываем инклюды в которых будем искать хедеры библиотек проекта
message("******** Lib include dirs ***********")
foreach(INCLUDE_DIR ${LIBS_INCLUDE_FOLDERS})
	include_directories(${LIBS_FOLDER}${INCLUDE_DIR})
	message(${LIBS_FOLDER}${INCLUDE_DIR})
endforeach(INCLUDE_DIR)

# Запускаем конфигурацию подпроектов
foreach(LIBRARY_DIR ${LIBS_FOLDERS})
	message("******** Subproject start ***********")
	message("Project with path = ${LIBS_FOLDER}${LIBRARY_DIR}")
	# Кастомные флаги сборки библиотек
	set (CMAKE_C_FLAGS "${CUSTOM_COMPILER_C_FLAGS}")
  	set (CMAKE_CXX_FLAGS "${CUSTOM_COMPILER_CXX_FLAGS}")
	add_subdirectory(${LIBS_FOLDER}${LIBRARY_DIR} ${CMAKE_BINARY_DIR}/libs/${LIBRARY_DIR})
	# message("LIBS = ${${LIBRARY_DIR}_LIBRARIES}")
	# message("includes = ${GLFW_INCLUDE_DIRS}")
	# message("libs = ${GLFW_PKG_DEPS}")
	# message("pkg = ${GLFW_PKG_LIBS}")
endforeach()

####################################################
# Системные библиотеки
####################################################
if (APPLE)
    set (SYSTEM_LIBS "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-ObjC" "z")
endif ()
if (UNIX AND NOT WIN32 AND NOT APPLE)
    set (SYSTEM_LIBS rt Xrandr Xinerama Xi Xxf86vm Xcursor m c dl z GL GLU pthread X11)
endif()
if (WIN32)
    set (SYSTEM_LIBS opengl32)
endif ()
if (MSYS)
    set (SYSTEM_LIBS )
endif ()
message("******** System libs list ***********")
message("${SYSTEM_LIBS}")


####################################################
# Настройка сборки текущего проекта
####################################################
# Сразу отключаем ненужные предупреждения на Windows
if (MSYS)
	add_definitions ( -D_CRT_SECURE_NO_WARNINGS )
	add_definitions ( -D_SCL_SECURE_NO_WARNINGS )
endif ()

# Не кастомные флаги компиляции
if(APPLE)
    # TODO: пробросить линковку
    set (PROJECT_LINK_FLAGS "-stdlib=libc++")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -stdlib=libc++")
endif ()
if (UNIX)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CUSTOM_COMPILER_C_FLAGS} -O0 -Wall -g -std=c++11")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CUSTOM_COMPILER_CXX_FLAGS} -O0 -Wall -g -std=c++11")
endif ()
if (MSYS)
    # микрософту не нужен флаг -std=c++11
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O0 /g /Wall")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O0 /g /Wall")
endif ()
if (WIN32)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -std=c++11")
endif ()

# Вывод информации о настройке копиляции (Функция в CMake/Helpers.cmake)
print_current_cmake_config()


####################################################
# Исходники
####################################################
# Запускаем поиск иходников
foreach(DIR ${SRC_FOLDERS})
	# собираем список файлов в папке (не рекурсивно, не GLOB_RECURSE !)
	# можно еще обобщить, задавая список нужных расширений
	file(GLOB HEADERS "${DIR}/*.h")
	file(GLOB SOURCES "${DIR}/*.cpp" "${DIR}/*.c" "${DIR}/*.cc")
	# Формирование групп исходников для красивого отображения в проекте
	# для имен групп в xcode нужен обратный слэш
	string(REGEX REPLACE "/" "\\\\" GROUP_NAME ${DIR})
    # source_group("Sources\\${GROUP_NAME}" FILES ${HEADERS} ${SOURCES})
	source_group("${GROUP_NAME}" FILES ${HEADERS} ${SOURCES})
	# сцепляем пути
	list(APPEND ALL_HEADERS ${HEADERS})
	list(APPEND ALL_SOURCES ${SOURCES})
endforeach(DIR)
message("******** Sources list ***********")
message("${ALL_HEADERS}")
message("${ALL_SOURCES}")

# тип сборки
if(NOT CMAKE_BUILD_TYPE)
  set( CMAKE_BUILD_TYPE Debug CACHE STRING "Debug" FORCE )
endif()


####################################################
# Генерация
####################################################
message("******** Generating started ***********")
# Из найденных исходников выставляем генерацию бинарника
add_executable(${EXECUTABLE_NAME} ${ALL_HEADERS} ${ALL_SOURCES})

# Конкретные пути к бинарнику
set_target_properties( ${EXECUTABLE_NAME}
    PROPERTIES
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib"
    # RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}
)
# set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
# set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

# Создаем символьную ссылку на ресурсы (Для исполняемго файла, после выбора)
if (WIN32)
    string(REGEX REPLACE "/" "\\\\" SOURCE_RES_PATH "${CMAKE_SOURCE_DIR}/res")
    string(REGEX REPLACE "/" "\\\\" RESULT_RES_PATH "${CMAKE_BINARY_DIR}/res/")
    message("COPY RES COMMAND = xcopy ${SOURCE_RES_PATH} ${RESULT_RES_PATH} /y")
    add_custom_command (
        TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND xcopy ${SOURCE_RES_PATH} ${RESULT_RES_PATH} /y
    )
else()
    add_custom_command (
        TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND rm -f ${CMAKE_BINARY_DIR}/res
        COMMAND ln -s ../../res ${CMAKE_BINARY_DIR}/res
    )
endif()

# Зависимости сборки
add_dependencies (${EXECUTABLE_NAME} ${LIBS_DEPENCY_NAMES})

# Библиотеки, с которыми будем ликовать
# link_directories(${LIBS_LINK_FOLDERS})
target_link_libraries (${EXECUTABLE_NAME} ${LIBS_LINK_NAMES} ${SYSTEM_LIBS})
