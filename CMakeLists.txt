cmake_minimum_required(VERSION 3.16)

project(ryuzaki
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system json locale)
find_package(Threads REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(spdlog REQUIRED)

add_executable(ryuzaki
    src/main.cpp
    src/logger.cpp
    src/controllers/PingController.cpp
    src/controllers/SearchController.cpp

    third_party/sqlite/sqlite3.c
)

target_link_libraries(ryuzaki PRIVATE
    Boost::system
    Boost::json
    Boost::locale
    Threads::Threads
    Drogon::Drogon
    spdlog::spdlog
)

target_include_directories(ryuzaki PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/sqlite
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(ryuzaki PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
    )
endif()

target_compile_features(ryuzaki PRIVATE cxx_std_20)

target_compile_definitions(ryuzaki PRIVATE
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>

    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
    $<$<CONFIG:MinSizeRel>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
   AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
    target_compile_options(ryuzaki PRIVATE -fcoroutines)
endif()






################################   TEST   #################################

include(CTest)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)

    add_executable(ryuzaki_tests
        tests/normalizers/DefaultNormalizerTest.cpp
        tests/normalizers/LowerNormalizerTest.cpp
        tests/normalizers/OnlyNumericNormalizerTest.cpp
        tests/normalizers/NoneNormalizerTest.cpp
    )

    target_link_libraries(ryuzaki_tests PRIVATE
        GTest::gtest_main
        Boost::system
        Boost::json
        Boost::locale
        Threads::Threads
        Drogon::Drogon
        spdlog::spdlog
    )

    target_include_directories(ryuzaki_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    include(GoogleTest)

    gtest_discover_tests(ryuzaki_tests)
endif()


###########################################################################