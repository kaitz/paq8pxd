set cdir=%CD%
set path=%cdir%\tools\mingw64\bin\..\;%cdir%\tools\mingw64\x86_64-w64-mingw32\lib;%cdir%\tools\mingw64\x86_64-w64-mingw32\include;%cdir%tools\mingw64\lib\gcc\x86_64-w64-mingw32\14.2.0\include\c++
set gcc=%cdir%\tools\mingw64\bin\g++.exe 
set ar=%cdir%\tools\mingw64\bin\ar.exe 
set ranlib=%cdir%\tools\mingw64\bin\ranlib.exe 
  set include=%cdir%\tools\mingw64\lib\gcc\x86_64-w64-mingw32\14.2.0\include\c++\
%gcc% -O3 -m64 -std=c++11  -DWINDOWS -DMT -masm=intel -mavx2  -include stdint.h -include %include%cstdio   -c preflate/preflate_block_decoder.cpp preflate/preflate_block_reencoder.cpp preflate/preflate_block_trees.cpp preflate/preflate_checker.cpp preflate/preflate_complevel_estimator.cpp preflate/preflate_constants.cpp preflate/preflate_decoder.cpp preflate/preflate_hash_chain.cpp preflate/preflate_info.cpp preflate/preflate_parameter_estimator.cpp preflate/preflate_parser_config.cpp preflate/preflate_predictor_state.cpp preflate/preflate_reencoder.cpp preflate/preflate_seq_chain.cpp preflate/preflate_statistical_codec.cpp preflate/preflate_statistical_debug.cpp preflate/preflate_statistical_model.cpp preflate/preflate_token.cpp preflate/preflate_token_predictor.cpp preflate/preflate_tree_predictor.cpp preflate/support/arithmetic_coder.cpp preflate/support/array_helper.cpp preflate/support/bitstream.cpp preflate/support/bit_helper.cpp preflate/support/const_division.cpp preflate/support/filestream.cpp preflate/support/huffman_decoder.cpp preflate/support/huffman_encoder.cpp preflate/support/huffman_helper.cpp preflate/support/memstream.cpp preflate/support/outputcachestream.cpp preflate/support/support_tests.cpp preflate/support/task_pool.cpp
%ar% r preflate.a preflate_block_decoder.o preflate_block_reencoder.o preflate_block_trees.o preflate_checker.o preflate_complevel_estimator.o preflate_constants.o preflate_decoder.o preflate_hash_chain.o preflate_info.o preflate_parameter_estimator.o preflate_parser_config.o preflate_predictor_state.o preflate_reencoder.o preflate_seq_chain.o preflate_statistical_codec.o preflate_statistical_debug.o preflate_statistical_model.o preflate_token.o preflate_token_predictor.o preflate_tree_predictor.o arithmetic_coder.o array_helper.o bitstream.o bit_helper.o const_division.o filestream.o huffman_decoder.o huffman_encoder.o huffman_helper.o memstream.o outputcachestream.o support_tests.o task_pool.o
%ranlib% preflate.a
del *.o
pause



