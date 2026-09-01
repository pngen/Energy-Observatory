add_executable(eobsv_example_record
  record_energy.cpp)
target_link_libraries(eobsv_example_record PRIVATE EObservatory::eobsv)

if(MSVC)
  target_compile_options(eobsv_example_record PRIVATE /W4 /permissive- /Zc:__cplusplus)
  if(EOBSV_WARNINGS_AS_ERRORS)
    target_compile_options(eobsv_example_record PRIVATE /WX)
  endif()
  if(EOBSV_ENABLE_SANITIZERS)
    target_compile_options(eobsv_example_record PRIVATE /fsanitize=address)
  endif()
endif()
