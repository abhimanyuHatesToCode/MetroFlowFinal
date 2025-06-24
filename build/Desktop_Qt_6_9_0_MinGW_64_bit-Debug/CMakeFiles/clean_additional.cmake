# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\MetroOptimization_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\MetroOptimization_autogen.dir\\ParseCache.txt"
  "MetroOptimization_autogen"
  )
endif()
