set(NIFTI_ZNZLIB_NAME ${NIFTI_PACKAGE_PREFIX}znz)

add_nifti_library(${NIFTI_ZNZLIB_NAME} znzlib.c )
target_link_libraries( ${NIFTI_ZNZLIB_NAME} PUBLIC ${NIFTI_ZLIB_LIBRARIES} )
if(${ZLIB_FOUND})
  target_include_directories(${NIFTI_ZNZLIB_NAME} PUBLIC
                            ${ZLIB_INCLUDE_DIR}
                            )

endif()
set_target_properties(
  ${NIFTI_ZNZLIB_NAME}
  PROPERTIES
    PUBLIC_HEADER ${CMAKE_CURRENT_LIST_DIR}/znzlib.h
    )
target_compile_definitions(${NIFTI_ZNZLIB_NAME} PUBLIC  ${ZNZ_COMPILE_DEF})
# Set library version if building shared libs.
if(BUILD_SHARED_LIBS)
    get_lib_version_vars("znzlib_version.h" ZNZLIB_VERSION ZNZLIB_MAJOR_VERSION)
    set_target_properties(${NIFTI_ZNZLIB_NAME}
      PROPERTIES
        ${NIFTI_LIBRARY_PROPERTIES}
        VERSION ${ZNZLIB_VERSION}
        SOVERSION ${ZNZLIB_MAJOR_VERSION}
        )
    target_compile_definitions(${NIFTI_ZNZLIB_NAME} PRIVATE ZNZ_BUILD_SHARED)
    target_compile_definitions(${NIFTI_ZNZLIB_NAME} INTERFACE ZNZ_USE_SHARED)
endif()
install_nifti_target(${NIFTI_ZNZLIB_NAME})

if(NIFTI_BUILD_TESTING AND ZLIB_FOUND AND NOT WIN32)
  # znzprintf is only compiled under COMPILE_NIFTIUNUSED_CODE, and not at
  # all on Windows, so the test builds its own copy of znzlib.c with that
  # definition and is registered only where the function exists.
  add_executable(znzprintf_test znzprintf_test.c znzlib.c)
  target_compile_definitions(znzprintf_test PRIVATE COMPILE_NIFTIUNUSED_CODE)
  target_include_directories(znzprintf_test PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${ZLIB_INCLUDE_DIR})
  target_link_libraries(znzprintf_test PRIVATE ${NIFTI_ZLIB_LIBRARIES})
  add_test(NAME znzprintf_truncation
           COMMAND $<TARGET_FILE:znzprintf_test> ${CMAKE_CURRENT_BINARY_DIR}/znzprintf_test.gz)
endif()
