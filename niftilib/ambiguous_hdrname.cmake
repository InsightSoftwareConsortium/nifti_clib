# Drives the ambiguous-filename path, which must report the clash to its
# caller. A plain add_test cannot tell exit 1 from the abort it replaced.
file(REMOVE_RECURSE ${DIR})
file(MAKE_DIRECTORY ${DIR})

execute_process(COMMAND ${TOOL} -make_im -new_dim 3 4 4 4 0 0 0 0
                        -prefix ${DIR}/amb.nii RESULT_VARIABLE mk1)
execute_process(COMMAND ${TOOL} -make_im -new_dim 3 4 4 4 0 0 0 0
                        -prefix ${DIR}/amb.nii.gz RESULT_VARIABLE mk2)
if(NOT mk1 STREQUAL "0" OR NOT mk2 STREQUAL "0")
  message(FATAL_ERROR "could not create the ambiguous pair: ${mk1} ${mk2}")
endif()

execute_process(COMMAND ${TOOL} -disp_hdr -infiles ${DIR}/amb
                RESULT_VARIABLE rv ERROR_VARIABLE err OUTPUT_VARIABLE out)
if(NOT rv STREQUAL "1")
  message(FATAL_ERROR "expected exit 1, got '${rv}'\n${err}")
endif()
if(NOT err MATCHES "Multiple possible filenames")
  message(FATAL_ERROR "missing ambiguity diagnostic:\n${err}")
endif()

# Unambiguous names must still resolve, so that returning NULL
# unconditionally could not pass.
file(REMOVE ${DIR}/amb.nii.gz)
execute_process(COMMAND ${TOOL} -disp_hdr -infiles ${DIR}/amb
                RESULT_VARIABLE rv2 ERROR_VARIABLE err2 OUTPUT_VARIABLE out2)
if(NOT rv2 STREQUAL "0")
  message(FATAL_ERROR "unambiguous name failed with '${rv2}'\n${err2}")
endif()
