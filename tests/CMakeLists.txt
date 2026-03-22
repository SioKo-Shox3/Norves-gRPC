find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(norves_core_tests
    core/status_test.cpp
    core/grpc_channel_test.cpp
    core/grpc_server_test.cpp
)

target_link_libraries(norves_core_tests
    PRIVATE
        Norves::gRPC
        GTest::gtest
        GTest::gtest_main
)

gtest_discover_tests(norves_core_tests)
