cmake_minimum_required(VERSION 3.20)
project(NorvesGRPC
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Game-oriented general-purpose network library wrapping gRPC"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Dependencies
# ==============================================================================
find_package(Threads REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)

# ==============================================================================
# Proto file generation
# ==============================================================================
set(NORVES_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(NORVES_PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${NORVES_PROTO_GEN_DIR})

function(norves_generate_proto PROTO_FILE)
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(PROTO_SRC "${NORVES_PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDR "${NORVES_PROTO_GEN_DIR}/${PROTO_NAME}.pb.h")
    set(GRPC_SRC "${NORVES_PROTO_GEN_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(GRPC_HDR "${NORVES_PROTO_GEN_DIR}/${PROTO_NAME}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
        COMMAND protobuf::protoc
        ARGS
            --proto_path=${NORVES_PROTO_DIR}
            --cpp_out=${NORVES_PROTO_GEN_DIR}
            --grpc_out=${NORVES_PROTO_GEN_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
            ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC/Protobuf C++ sources for ${PROTO_NAME}.proto"
    )

    set(PROTO_GENERATED_SRCS ${PROTO_SRC} ${GRPC_SRC} PARENT_SCOPE)
    set(PROTO_GENERATED_HDRS ${PROTO_HDR} ${GRPC_HDR} PARENT_SCOPE)
endfunction()

norves_generate_proto("${NORVES_PROTO_DIR}/norves/core.proto")
set(CORE_PROTO_SRCS ${PROTO_GENERATED_SRCS})
set(CORE_PROTO_HDRS ${PROTO_GENERATED_HDRS})

# ==============================================================================
# Norves-gRPC Library
# ==============================================================================
set(NORVES_SOURCES
    src/core/status.cpp
    src/core/grpc_channel.cpp
    src/core/grpc_server.cpp
    src/core/grpc_client.cpp
    src/session/session.cpp
    src/session/session_manager.cpp
    ${CORE_PROTO_SRCS}
)

add_library(norves_grpc ${NORVES_SOURCES})
add_library(Norves::gRPC ALIAS norves_grpc)

target_include_directories(norves_grpc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NORVES_PROTO_GEN_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(norves_grpc
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
        Threads::Threads
)

target_compile_features(norves_grpc PUBLIC cxx_std_20)

# ==============================================================================
# Examples
# ==============================================================================
option(NORVES_BUILD_EXAMPLES "Build example programs" ON)
if(NORVES_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==============================================================================
# Tests
# ==============================================================================
option(NORVES_BUILD_TESTS "Build unit tests" ON)
if(NORVES_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
