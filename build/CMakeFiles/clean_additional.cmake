# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/VM_SIC_GUI_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/VM_SIC_GUI_autogen.dir/ParseCache.txt"
  "CMakeFiles/VM_SIC_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/VM_SIC_autogen.dir/ParseCache.txt"
  "VM_SIC_GUI_autogen"
  "VM_SIC_autogen"
  )
endif()
