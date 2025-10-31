# Client UDP to send data related to sensing capabilities

# Configure the output folder for the corresponding allocation of the executable in build/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples/usurp)



# Agrega el ejecutable
add_executable(usurpdata usurpdata.c)

# Include the necessary directories for linking the headers
target_include_directories(usurpdata PRIVATE
    ${CMAKE_SOURCE_DIR}/examples/emulator/agent  # Solo para sm_new.h si es necesario
    ${CMAKE_SOURCE_DIR}/src/util
)

# Bind of the necessary libraries to the UDP client
target_link_libraries(usurpdata 
    PRIVATE
    pthread
)

