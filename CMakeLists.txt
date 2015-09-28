cmake_minimum_required(VERSION 3.1)

project(json-parser)

add_library(json-parser json.c)
set_property(TARGET json-parser PROPERTY C_STANDARD 99) # or 11
target_include_directories(json-parser PUBLIC .)

option(JSON_TRACK_SOURCE "Enables source tracking via compile definition")
if(JSON_TRACK_SOURCE)
	target_compile_definitions(json-parser PUBLIC JSON_TRACK_SOURCE)
endif()

# detect if we need libm
include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(pow HasPow)
if(NOT HasPow)
	list(APPEND CMAKE_REQUIRED_LIBRARIES m)
	CHECK_FUNCTION_EXISTS(pow HasPowLibm)
	if(HasPowLibm)
		target_link_libraries(json-parser PUBLIC m)
	else()
		message(FATAL_ERROR "The C pow() function is required by json-parser")
	endif()
endif()

install(FILES json.h DESTINATION include)
install(TARGETS json-parser ARCHIVE DESTINATION lib)

enable_testing()

option(BUILD_PYTHON_MODULE "Enable experimental support for building the python bindings and running the python tests with CMake")
if(BUILD_PYTHON_MODULE)
	find_package(PythonInterp 2 EXACT) # the Python tests and bindings are not compatible with Python 3
	if(PYTHONINTERP_FOUND)
		get_filename_component(PYTHON_DIRECTORY ${PYTHON_EXECUTABLE} DIRECTORY)

		add_library(json-python-module SHARED bindings/python/wrap_json.c)
		set_target_properties(json-python-module PROPERTIES
			OUTPUT_NAME "jsonparser.pyd"
			PREFIX ""
			SUFFIX ""
			C_STANDARD 99
		)
		if(WIN32)
			target_include_directories(json-python-module PUBLIC "${PYTHON_DIRECTORY}/include")
			target_link_libraries(json-python-module "${PYTHON_DIRECTORY}/libs/libpython${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.a")
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				target_compile_definitions(json-python-module PUBLIC MS_WIN64)
			endif()
		else()
			target_include_directories(json-python-module PUBLIC "${PYTHON_DIRECTORY}/../include/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
			target_link_libraries(json-python-module "${PYTHON_DIRECTORY}/../lib/libpython${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.a")
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				message(WARNING "Linking to libpython${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.a will likely fail unless it is 64-bit or built with -fPIC")
			endif()
		endif()
		add_test(
			NAME python-tests
			COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/tests/test.py"
			WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/tests/"
		)
		set_property(
			TEST python-tests
			PROPERTY ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}"
		)
	endif()
endif()

if(NOT WIN32) # test_json.c does not work on Windows
	add_executable(example-c examples/test_json.c)
	target_link_libraries(example-c json-parser)
	add_test(
		NAME example-c-test
		COMMAND example-c "valid-0000.json"
		WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/tests/"
	)
endif()

# Convenience for the possible future
#add_executable(example-cpp examples/test_json.cpp)
#set_property(TARGET example-cpp PROPERTY CXX_STANDARD 11) # to test the features conditional on C++11
#target_link_libraries(example-cpp json-parser)
#add_test(
#	NAME example-cpp-test
#	COMMAND example-cpp "valid-0000.json"
#	WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/tests/"
#)
