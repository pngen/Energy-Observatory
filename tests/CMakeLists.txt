add_executable(eobsv_tests
  test_main.cpp
  test_ids.cpp
  test_units.cpp
  test_validation.cpp
  test_provenance.cpp
  test_observation.cpp
  test_energy.cpp
  test_aggregate.cpp)

target_link_libraries(eobsv_tests PRIVATE EObservatory::eobsv)
target_include_directories(eobsv_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  target_compile_options(eobsv_tests PRIVATE /W4 /permissive- /Zc:__cplusplus)
  if(EOBSV_WARNINGS_AS_ERRORS)
    target_compile_options(eobsv_tests PRIVATE /WX)
  endif()
else()
  target_compile_options(eobsv_tests PRIVATE -Wall -Wextra -Wpedantic)
  if(EOBSV_WARNINGS_AS_ERRORS)
    target_compile_options(eobsv_tests PRIVATE -Werror)
  endif()
endif()

if(EOBSV_ENABLE_SANITIZERS)
  if(MSVC)
    target_compile_options(eobsv_tests PRIVATE /fsanitize=address)
  else()
    target_compile_options(eobsv_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(eobsv_tests PRIVATE -fsanitize=address,undefined)
  endif()
endif()

add_test(NAME eobsv_tests COMMAND eobsv_tests)
