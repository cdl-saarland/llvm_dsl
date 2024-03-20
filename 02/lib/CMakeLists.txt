# adapted from AdaptiveCpp's libkernel compilation

find_program(CLANG_EXECUTABLE_PATH NAMES clang++-${LLVM_VERSION_MAJOR} clang++-${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} clang++ HINTS ${LLVM_TOOLS_BINARY_DIR} NO_DEFAULT_PATH CACHE STRING)
if(CLANG_EXECUTABLE_PATH MATCHES "-NOTFOUND")
  find_program(CLANG_EXECUTABLE_PATH NAMES clang++-${LLVM_VERSION_MAJOR} clang++-${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} clang++ CACHE STRING)
  if(CLANG_EXECUTABLE_PATH MATCHES "-NOTFOUND")
    message(SEND_ERROR "Could not find clang executable")
  endif()
endif()

function(generate_bitcode_library)
  set(options)
  set(one_value_keywords OUTPUT TRIPLE SOURCE)
  set(multi_value_keywords ADDITIONAL_ARGS)
  cmake_parse_arguments(GENERATE_BITCODE_LIBRARY
    "${options}"
    "${one_value_keywords}"
    "${multi_value_keywords}"
    ${ARGN}
  )

  set(output ${GENERATE_BITCODE_LIBRARY_OUTPUT})
  set(source ${GENERATE_BITCODE_LIBRARY_SOURCE})
  set(triple ${GENERATE_BITCODE_LIBRARY_TRIPLE})
  set(additional ${GENERATE_BITCODE_LIBRARY_ADDITIONAL_ARGS})

  set(target_output_file ${output})
  add_custom_command(
      OUTPUT ${target_output_file}
      COMMAND ${CLANG_EXECUTABLE_PATH} -target ${triple} -fno-exceptions -fno-rtti -ffast-math -O -emit-llvm -std=c++20
                 ${additional} -o ${target_output_file} -c ${CMAKE_CURRENT_SOURCE_DIR}/${source}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
      VERBATIM)

  install(FILES ${target_output_file} DESTINATION lib/bitcode)
endfunction()

function(generate_bitcode_target)
  set(options)
  set(one_value_keywords TARGETNAME TRIPLE)
  set(multi_value_keywords SOURCES ADDITIONAL_ARGS)

  cmake_parse_arguments(GENERATE_BITCODE_TARGET
    "${options}"
    "${one_value_keywords}"
    "${multi_value_keywords}"
    ${ARGN}
  )

  set(target ${GENERATE_BITCODE_TARGET_TARGETNAME})
  set(sources ${GENERATE_BITCODE_TARGET_SOURCES})
  set(triple ${GENERATE_BITCODE_TARGET_TRIPLE})
  set(additional ${GENERATE_BITCODE_TARGET_ADDITIONAL_ARGS})

  set(output_files )
  foreach(source ${sources})
    get_filename_component(target_source ${source} NAME_WE)
    set(output_file ${CMAKE_CURRENT_BINARY_DIR}/libbuiltin-${target}-${target_source}.bc)
    list(APPEND output_files ${output_file})
    
    generate_bitcode_library(
      OUTPUT ${output_file}
      TRIPLE ${triple} 
      SOURCE ${source}
      ADDITIONAL_ARGS ${additional})
  endforeach()

  set(linked_output ${CMAKE_CURRENT_BINARY_DIR}/libbuiltin-${target}-full.bc)
  add_custom_command(
    OUTPUT ${linked_output}
    COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-link -o=${linked_output} ${output_files}
    DEPENDS ${output_files}
  )

  install(FILES ${linked_output} DESTINATION lib/bitcode)

  add_custom_target(libbuiltin-${target} ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${linked_output}
  )

endfunction()

if(NOT DEFINED LLVM_TARGET_TRIPLE)
set(LLVM_TARGET_TRIPLE ${TARGET_TRIPLE})
endif()

generate_bitcode_target(
    TARGETNAME host 
    TRIPLE ${LLVM_TARGET_TRIPLE}
    SOURCES tensor.cpp)

