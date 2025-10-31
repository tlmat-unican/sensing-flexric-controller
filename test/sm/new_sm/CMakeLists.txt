set(SM_ENCODING_NEW "PLAIN" CACHE STRING "The NEW SM encoding to use")
set_property(CACHE SM_ENCODING_NEW PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected NEW SM_ENCODING: ${SM_ENCODING_NEW}")


if(SM_ENCODING_NEW  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_new_sm
                      main.c 
                      ../../rnd/fill_rnd_data_new.c
                      ../../../src/util/time_now_us.c
              )
elseif(SM_ENCODING_NEW STREQUAL "ASN")
  message(FATAL_ERROR "Not implemented ASN encoding")

elseif(SM_ENCODING_NEW STREQUAL "FLATBUFFERS")
  message(FATAL_ERROR "Not implemented ASN encoding")

endif()

target_compile_definitions(test_new_sm PUBLIC ${SM_ENCODING_NEW})
target_link_libraries(test_new_sm PUBLIC new_sm_static -pthread)

enable_testing()
add_test(Unit_test_NEW test_new_sm)
