include(FindEXPAT)

set(NIFTI_CIFTILIB_NAME ${NIFTI_PACKAGE_PREFIX}cifti)

add_nifti_library(${NIFTI_CIFTILIB_NAME} afni_xml.c afni_xml_io.c )
set_target_properties(
  ${NIFTI_CIFTILIB_NAME}
  PROPERTIES
    PRIVATE_HEADER  "${CMAKE_CURRENT_LIST_DIR}/afni_xml.h;${CMAKE_CURRENT_LIST_DIR}/afni_xml_io.h"
    )
target_link_libraries( ${NIFTI_CIFTILIB_NAME} PUBLIC EXPAT::EXPAT ${NIFTI_PACKAGE_PREFIX}nifti2)
# Set lib version when buildung shared libs.
if(BUILD_SHARED_LIBS)
    set_target_properties(${NIFTI_CIFTILIB_NAME} PROPERTIES ${NIFTI_LIBRARY_PROPERTIES})
    target_compile_definitions(${NIFTI_CIFTILIB_NAME} PRIVATE CIFTI_BUILD_SHARED)
    target_compile_definitions(${NIFTI_CIFTILIB_NAME} INTERFACE CIFTI_USE_SHARED)
endif()
install_nifti_target(${NIFTI_CIFTILIB_NAME})


if(NIFTI_BUILD_APPLICATIONS)
  add_nifti_executable(${NIFTI_PACKAGE_PREFIX}afni_xml_tool afni_xml_tool.c)
  target_link_libraries(${NIFTI_PACKAGE_PREFIX}afni_xml_tool PUBLIC ${NIFTI_CIFTILIB_NAME})
  add_nifti_executable(${NIFTI_PACKAGE_PREFIX}cifti_tool cifti_tool.c)
  target_link_libraries(${NIFTI_PACKAGE_PREFIX}cifti_tool PUBLIC ${NIFTI_CIFTILIB_NAME})
  install_nifti_target(${NIFTI_PACKAGE_PREFIX}afni_xml_tool)
  install_nifti_target(${NIFTI_PACKAGE_PREFIX}cifti_tool)
endif()

if(NIFTI_BUILD_TESTING AND NIFTI_BUILD_APPLICATIONS)
  set(TEST_PREFIX "${NIFTI_PACKAGE_PREFIX}cifti")
  # An unrecognized MatrixIndicesMap child is skipped, not dispatched on.
  add_test( NAME ${TEST_PREFIX}_mim_summary_unknown_child
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -as_cext -eval_cext -eval_type show_summary
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/mim_unknown_child.xml )
  # A recognized child is still displayed.
  add_test( NAME ${TEST_PREFIX}_mim_summary_known_child
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -as_cext -eval_cext -eval_type show_summary
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/mim_known_child.xml )
  set_tests_properties( ${TEST_PREFIX}_mim_summary_known_child
                        PROPERTIES PASS_REGULAR_EXPRESSION "BrainModel" )

  # The CIFTI extension is found even when another extension precedes it.
  add_test( NAME ${TEST_PREFIX}_tool_cext_not_first
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/cext_second_extension.nii
                    -disp_cext )
  set_tests_properties( ${TEST_PREFIX}_tool_cext_not_first
            PROPERTIES PASS_REGULAR_EXPRESSION "<MapName>demo</MapName>"
                       FAIL_REGULAR_EXPRESSION "no CIFTI extension" )

  # A read failure must be reported, not taken for the end of the document.
  # The input is a directory, which opens on POSIX and fails at the first
  # read; Windows refuses the open instead, so there is no read to fail.
  if( NOT WIN32 )
    add_test( NAME ${TEST_PREFIX}_xml_read_error
              COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}afni_xml_tool>
                      -input ${CMAKE_CURRENT_LIST_DIR}/testdata )
    set_tests_properties( ${TEST_PREFIX}_xml_read_error
              PROPERTIES PASS_REGULAR_EXPRESSION "failed to read XML file" )
  endif()

  # A readable document must still parse, and must not report a read error.
  add_test( NAME ${TEST_PREFIX}_xml_read_ok
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}afni_xml_tool>
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/mim_known_child.xml
                    -verb 2 )
  set_tests_properties( ${TEST_PREFIX}_xml_read_ok
            PROPERTIES PASS_REGULAR_EXPRESSION "xlist read"
                       FAIL_REGULAR_EXPRESSION "failed to read XML file" )

  # An extension payload that fills esize-8 with no NUL must not be read
  # past its end; the regex pins that the payload is still parsed.
  add_test( NAME ${TEST_PREFIX}_tool_unterminated_cext
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/cext_unterminated.nii
                    -eval_cext -eval_type show_summary )
  set_tests_properties( ${TEST_PREFIX}_tool_unterminated_cext
            PROPERTIES PASS_REGULAR_EXPRESSION "MapName : demo" )

  # Nesting past AXML_MAX_DEPTH must stay inside the skip block rather than
  # walk off the fixed stack; the regex pins that the rest still parses.
  add_test( NAME ${TEST_PREFIX}_deep_nesting
            COMMAND $<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -as_cext -eval_cext -eval_type show_summary
                    -input ${CMAKE_CURRENT_LIST_DIR}/testdata/deep_nesting.xml )
  set_tests_properties( ${TEST_PREFIX}_deep_nesting
            PROPERTIES PASS_REGULAR_EXPRESSION "BrainModel" )

  # A -output path that cannot be opened must be reported and refused, not
  # written to anyway.
  add_test( NAME ${TEST_PREFIX}_write_failure
            COMMAND ${CMAKE_COMMAND}
                    -DTOOL=$<TARGET_FILE:${NIFTI_PACKAGE_PREFIX}cifti_tool>
                    -DINPUT=${CMAKE_CURRENT_LIST_DIR}/testdata/cext_unterminated.nii
                    -DBADOUT=${CMAKE_CURRENT_BINARY_DIR}/no_such_dir/out.txt
                    -DGOODOUT=${CMAKE_CURRENT_BINARY_DIR}/write_failure_ok.txt
                    -P ${CMAKE_CURRENT_LIST_DIR}/test_write_failure.cmake )
endif()
