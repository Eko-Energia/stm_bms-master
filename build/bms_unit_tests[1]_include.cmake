if(EXISTS "C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests.exe")
  if(NOT EXISTS "C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests[1]_tests.cmake" OR
     NOT "C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests[1]_tests.cmake" IS_NEWER_THAN "C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests.exe" OR
     NOT "C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests[1]_tests.cmake" IS_NEWER_THAN "${CMAKE_CURRENT_LIST_FILE}")
    include("C:/Program Files/CMake/share/cmake-3.29/Modules/GoogleTestAddTests.cmake")
    gtest_discover_tests_impl(
      TEST_EXECUTABLE [==[C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests.exe]==]
      TEST_EXECUTOR [==[]==]
      TEST_WORKING_DIR [==[C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build]==]
      TEST_EXTRA_ARGS [==[]==]
      TEST_PROPERTIES [==[]==]
      TEST_PREFIX [==[]==]
      TEST_SUFFIX [==[]==]
      TEST_FILTER [==[]==]
      NO_PRETTY_TYPES [==[FALSE]==]
      NO_PRETTY_VALUES [==[FALSE]==]
      TEST_LIST [==[bms_unit_tests_TESTS]==]
      CTEST_FILE [==[C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests[1]_tests.cmake]==]
      TEST_DISCOVERY_TIMEOUT [==[60]==]
      TEST_XML_OUTPUT_DIR [==[]==]
    )
  endif()
  include("C:/Users/brych/OneDrive/Pulpit/PROJEKTY/STM32/EKO_ENERGY/1.19.0/BMS-Master/build/bms_unit_tests[1]_tests.cmake")
else()
  add_test(bms_unit_tests_NOT_BUILT bms_unit_tests_NOT_BUILT)
endif()
