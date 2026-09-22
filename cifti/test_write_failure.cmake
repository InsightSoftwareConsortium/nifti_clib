# Drives cifti_tool at an unopenable -output path.  A plain add_test cannot
# express this: PASS_REGULAR_EXPRESSION ignores the exit status, so a crash
# that printed the diagnostic first would still pass.

if(NOT TOOL OR NOT INPUT OR NOT BADOUT OR NOT GOODOUT)
  message(FATAL_ERROR "TOOL, INPUT, BADOUT and GOODOUT are all required")
endif()

function(run_tool expect_result out_var)
  execute_process(COMMAND ${TOOL} ${ARGN}
                  RESULT_VARIABLE result
                  OUTPUT_VARIABLE out
                  ERROR_VARIABLE err)
  set(${out_var} "${out}${err}" PARENT_SCOPE)
  if(NOT result STREQUAL "${expect_result}")
    message(FATAL_ERROR
            "expected result ${expect_result}, got '${result}'\n${out}${err}")
  endif()
endfunction()

run_tool(0 bad_disp -input ${INPUT} -disp_cext -output ${BADOUT})
if(NOT bad_disp MATCHES "failed to open")
  message(FATAL_ERROR "missing the open-failure diagnostic:\n${bad_disp}")
endif()

run_tool(0 bad_eval -input ${INPUT} -eval_cext -eval_type show_summary
                    -output ${BADOUT})
if(NOT bad_eval MATCHES "failed to open")
  message(FATAL_ERROR "missing the open-failure diagnostic:\n${bad_eval}")
endif()

# An openable -output must still be written, so that refusing every write
# would not pass.
file(REMOVE ${GOODOUT})
run_tool(0 good_disp -input ${INPUT} -disp_cext -output ${GOODOUT})
if(NOT EXISTS ${GOODOUT})
  message(FATAL_ERROR "nothing written to ${GOODOUT}")
endif()
file(READ ${GOODOUT} written)
if(NOT written MATCHES "MapName")
  message(FATAL_ERROR "extension not displayed:\n${written}")
endif()
