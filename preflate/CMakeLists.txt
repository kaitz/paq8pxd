cmake_minimum_required(VERSION 3.2)

project(preflate)

set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OBJDIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(AFTER ${SRCDIR})

function(add_stem2file VAR pattern stem)
    set(TMPV ${${VAR}})
    foreach(IND ${stem})
        string(REGEX REPLACE "^[ ]*([0-9a-zA-Z_%]+)[ ]*$" \\1 IND ${IND})
        string(REPLACE "%STEM%" ${IND} NEXT ${pattern})
        set(TMPV "${TMPV};${NEXT}")
    endforeach()
    set(${VAR} "${TMPV}" PARENT_SCOPE)
endfunction()

set(LIB_SRC "")
add_stem2file(LIB_SRC "${SRCDIR}/preflate_%STEM%.cpp" 
              "block_decoder;block_reencoder;block_trees;\
               complevel_estimator;constants;decoder;hash_chain;info;\
               parameter_estimator;parser_config;predictor_state;reencoder;\
               seq_chain;statistical_codec;statistical_model;token;\
               token_predictor;tree_predictor")
add_stem2file(LIB_SRC "${SRCDIR}/support/%STEM%.cpp"
              "arithmetic_coder;array_helper;bit_helper;bitstream;const_division;\
               filestream;huffman_decoder;huffman_encoder;huffman_helper;memstream;\
               outputcachestream;task_pool")

set(DEMO_SRC "${SRCDIR}/main.cpp")
add_stem2file(DEMO_SRC "${SRCDIR}/preflate_%STEM%.cpp" 
              "checker;statistical_debug")
add_stem2file(DEMO_SRC "${SRCDIR}/support/%STEM%.cpp"
              "support_tests")

set(DEMO2_SRC "${SRCDIR}/main2.cpp")

set(LIB_HDR "")
add_stem2file(LIB_HDR "${SRCDIR}/preflate_%STEM%.h" 
              "block_decoder;block_reencoder;block_trees;checker;\
               complevel_estimator;constants;decoder;hash_chain;info;\
               parameter_estimator;parser_config;predictor_state;reencoder;\
               seq_chain;statistical_codec;statistical_model;token;\
               token_predictor;tree_predictor")
add_stem2file(LIB_HDR "${SRCDIR}/support/%STEM%.h" 
              "arithmetic_coder;array_helper;bitstream;bit_helper;const_division;\
               filestream;huffman_decoder;huffman_encoder;huffman_helper;\
               memstream;outputcachestream;task_pool")

add_definitions(-DZ_SOLO -DNO_GZIP)
set(DEMO_HDR "")
add_stem2file(DEMO_HDR "${SRCDIR}/preflate_%STEM%.h" 
              "checker")
add_stem2file(DEMO_HDR "${SRCDIR}/support/%STEM%.h" 
              "support_tests")

set(DEMO2_HDR "")

add_library(preflate STATIC ${LIB_SRC} ${LIB_HDR})
add_executable(preflate_demo ${DEMO_SRC} ${DEMO_HDR})
add_executable(preflate_demo2 ${DEMO2_SRC} ${DEMO2_HDR})
target_link_libraries(preflate_demo preflate)
target_link_libraries(preflate_demo2 preflate)
