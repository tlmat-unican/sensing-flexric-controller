cmake_minimum_required(VERSION 3.15) # setting this is required
project(new_sm)            # this sets the project name

set(SM_ENCODING_NEW_SRC
                      ../sm_proc_data.c 
                      new_sm_agent.c 
                      new_sm_ric.c 
                     ../../util/byte_array.c 
                     ../../util/alg_ds/alg/defer.c 
                     ../../util/alg_ds/alg/eq_float.c 
                     ../../util/alg_ds/ds/seq_container/seq_arr.c 
                     ../../util/alg_ds/ds/seq_container/seq_ring.c 
                     ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
                     ../../util/alg_ds/ds/assoc_container/bimap.c
                      ie/new_data_ie.c 
                      )


if(SM_ENCODING_NEW STREQUAL "PLAIN")
  set(SM_ENCODING_NEW_SRC_PLAIN 
    # This dependency sucks!
    $<TARGET_OBJECTS:e2ap_ran_func_obj>
    ${SM_ENCODING_NEW_SRC}
    enc/new_enc_plain.c 
    dec/new_dec_plain.c 
    )

  # Shared
  add_library(new_sm SHARED ${SM_ENCODING_NEW_SRC_PLAIN} )
  target_compile_options(new_sm PRIVATE -fPIC -fvisibility=hidden)

  # Static
  add_library(new_sm_static STATIC ${SM_ENCODING_NEW_SRC_PLAIN}) 

elseif(SM_ENCODING_NEW STREQUAL "ASN" )
  message(FATAL_ERROR "NEW SM ASN not implemented")
elseif(SM_ENCODING_NEW STREQUAL "FLATBUFFERS" )
  message(FATAL_ERROR "NEW SM FB not implemented")
else()
  message(FATAL_ERROR "Unknown SM encoding type ")
endif()

target_compile_definitions(new_sm PUBLIC ${SM_ENCODING_NEW} ${E2AP_VERSION}  ${KPM_VERSION}  )
target_compile_definitions(new_sm_static PUBLIC ${SM_ENCODING_NEW} ${E2AP_VERSION}  ${KPM_VERSION}  )

