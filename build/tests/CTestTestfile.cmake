# CMake generated Testfile for 
# Source directory: C:/Users/roast/Documents/GitHub/celeritas/tests
# Build directory: C:/Users/roast/Documents/GitHub/celeritas/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[celeritas_tests]=] "C:/Users/roast/Documents/GitHub/celeritas/build/tests/Debug/celeritas_tests.exe")
  set_tests_properties([=[celeritas_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;8;add_test;C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[celeritas_tests]=] "C:/Users/roast/Documents/GitHub/celeritas/build/tests/Release/celeritas_tests.exe")
  set_tests_properties([=[celeritas_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;8;add_test;C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[celeritas_tests]=] "C:/Users/roast/Documents/GitHub/celeritas/build/tests/MinSizeRel/celeritas_tests.exe")
  set_tests_properties([=[celeritas_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;8;add_test;C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[celeritas_tests]=] "C:/Users/roast/Documents/GitHub/celeritas/build/tests/RelWithDebInfo/celeritas_tests.exe")
  set_tests_properties([=[celeritas_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;8;add_test;C:/Users/roast/Documents/GitHub/celeritas/tests/CMakeLists.txt;0;")
else()
  add_test([=[celeritas_tests]=] NOT_AVAILABLE)
endif()
