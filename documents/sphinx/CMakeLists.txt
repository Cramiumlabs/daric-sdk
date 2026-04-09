cmake_minimum_required(VERSION 3.9) # doxygen_add_docs is available from 3.9

project(
	"Daric LL API"
  VERSION "0.3.0"
  DESCRIPTION "Daric LL API")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED breathe)

# enable the SCE code
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D DRV_SCE_ENABLED=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D DRV_SCE_ENABLED=1")

# base Doxygen configuration 
set(DOXYGEN_GENERATE_HTML NO)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_EXTRACT_ALL NO)
set(DOXYGEN_EXTRACT_STATIC NO)
set(DOXYGEN_EXTRACT_PRIVATE NO)
set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
set(DOXYGEN_PREDEFINED "DRV_SCE_ENABLED=1")
set(DOXYGEN_INCLUDE_PATH "../../drivers/Inc" "../../../project/Activecard/user/Inc/config/")

# if PDF in [YES, Y, 1, …]
if($ENV{PDF})
  message(STATUS "*** enable LaTeX for PDF generation")
  set(DOXYGEN_GENERATE_LATEX YES)
else()
  message(STATUS "disable LaTeX")
endif()

file(GLOB_RECURSE DRIVER_FILES 
  "../../drivers/Src/*.c" 
  "../../drivers/Src/*.h" 
  "../../drivers/Inc/*.h"
  "../../middleware/crossbar/crypto/*.c"
  "../../middleware/crossbar/crypto/*.h"
)

doxygen_add_docs(
  daric_doxygen 
  ${DRIVER_FILES}
)

# Build the target by issuing: cmake --build . --target foobar_sphinx
sphinx_add_docs(
  daric_sphinx
  BREATHE_PROJECTS daric_doxygen
  BUILDER html
  SOURCE_DIRECTORY docs)
