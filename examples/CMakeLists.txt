# --- Echo Server ---
add_executable(echo_server echo_server/main.cpp)
target_link_libraries(echo_server PRIVATE Norves::gRPC)
target_include_directories(echo_server
    PRIVATE ${NORVES_PROTO_GEN_DIR}
)

# --- Echo Client ---
add_executable(echo_client echo_client/main.cpp)
target_link_libraries(echo_client PRIVATE Norves::gRPC)
target_include_directories(echo_client
    PRIVATE ${NORVES_PROTO_GEN_DIR}
)
