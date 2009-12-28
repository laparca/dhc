###############################################################################
#CMake build script for hash cracker

###############################################################################
#Core setup
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( cracker )

INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckFunctionExists)

###############################################################################
#Build settings - configurable
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET( LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
#TODO: enable this depending on gcc version
#SET( CMAKE_CXX_FLAGS "-O3 -march=core2 -mtune=core2 -fno-strict-aliasing -Wall")
IF(WIN32)
	SET( CMAKE_CXX_FLAGS "")
	SET( CMAKE_CXX_FLAGS_DEBUG "" )
ELSE()
	SET( CMAKE_CXX_FLAGS "-O3 -fno-strict-aliasing -Wall")
	SET( CMAKE_CXX_FLAGS_DEBUG "-g3" )
ENDIF()

#Grab a config.h if we have one
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

###############################################################################
#Sanity checks - DO NOT EDIT

#Set platform-specific flags
IF(UNIX)
	SET( PIC_FLAG "-fPIC")
ELSE()
	SET( PIC_FLAG "")
ENDIF()

#Try to figure out what CPU we have
SET(AMD64 0)
SET(X86 0)
MESSAGE(STATUS "Autodetecting CPU type")
IF(UNIX)
	IF("$ENV{TARGET}" MATCHES "64")
		MESSAGE(STATUS "CPU type overridden by TARGET environment variable - amd64")
		SET( ARCH_SWITCH "-m64")
		SET( AMD64 1)	
	ELSEIF("$ENV{TARGET}" MATCHES "86")
		MESSAGE(STATUS "CPU type overridden by TARGET environment variable - x86")
		SET( ARCH_SWITCH "-m32")
		SET( X86 1)	
	ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
		MESSAGE(STATUS "Autodetecting CPU type - amd64")
		SET( ARCH_SWITCH "-m64")
		SET( AMD64 1)
	ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "i.86")
		MESSAGE(STATUS "Autodetecting CPU type - x86")
		SET( ARCH_SWITCH "-m32")
		SET(X86 1)
	ELSE()
		MESSAGE(ERROR "Could not autodetect CPU type!")
	ENDIF()
ELSE()
	#Windows only has 32 bit builds atm
	MESSAGE(STATUS "Autodetecting CPU type - x86")
	SET( ARCH_SWITCH "/EHsc")
	SET(X86 1)
ENDIF()

#See if we have CUDA installed
MESSAGE(STATUS "Check for CUDA")
FIND_PROGRAM(NVCC NAMES nvcc PATH /usr/local/cuda/bin) 
IF (NVCC)
	SET(CUDA_ENABLED 1)
	MESSAGE(STATUS "Check for CUDA - found")
ELSE()
	SET(CUDA_ENABLED 0)
	MESSAGE(STATUS "Check for CUDA - not found")
ENDIF()

#See if CUDA is set up for the current architecture
MESSAGE(STATUS "Checking CUDA configuration")
IF(CUDA_ENABLED)
	SET(CMAKE_REQUIRED_FLAGS ${ARCH_SWITCH})
	SET(CMAKE_REQUIRED_LIBRARIES "cuda")
	CHECK_CXX_SOURCE_COMPILES("#include <cuda.h>\nint main(int argc, char* argv[]) { cuMemcpy2D(0); return 0; }" CUDA_COMPILES)
	
	IF(CUDA_COMPILES)
	ELSE()
		MESSAGE(ERROR "CUDA test code failed to compile!")
		MESSAGE(ERROR "Check that you have the 64 bit version of the CUDA libraries installed.")
	ENDIF()
ENDIF()

#See if we have NASM installed
MESSAGE(STATUS "Check for NASM")
FIND_PROGRAM(NASM NAMES nasm PATH /usr/bin) 
IF (NASM)
	SET(NASMFOUND 1)
	MESSAGE(STATUS "Check for NASM - found")
ELSE()
	SET(NASMFOUND 0)
	MESSAGE(STATUS "Check for NASM - not found")
ENDIF()

IF( UNIX )
	CHECK_FUNCTION_EXISTS( clock_gettime LINUX )
	IF( !LINUX )
		CHECK_FUNCTION_EXISTS( gettimeofday BSD )
		SET(BSD 1)
	ELSE()
		SET(LINUX 1)
		MESSAGE(STATUS "Check for SELinux")
		FIND_PROGRAM(SELINUX_PATH "/usr/sbin/selinuxenabled")
		IF(SELINUX_PATH)
			MESSAGE(STATUS "Check for SELinux - found")
			SET(SELINUX 1)
		ELSE()
			MESSAGE(STATUS "Check for SELinux - not found")
			SET(SELINUX 0)
		ENDIF()
	ENDIF( !LINUX )
ENDIF( UNIX )

IF( APPLE )
	SET( MACOSX 1 )
	SET( OS_SWITCH "read_only_relocs suppress")
ENDIF( APPLE )

IF( WIN32)
	SET( WINDOWS 1 )
ENDIF()

CONFIGURE_FILE(config.h.in ${CMAKE_BINARY_DIR}/config.h)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PIC_FLAG} ${ARCH_SWITCH}"  )
SET(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${PIC_FLAG} ${ARCH_SWITCH}"  )

###############################################################################
#Binary modules
ADD_SUBDIRECTORY(agent)
ADD_SUBDIRECTORY(stresstest)

###############################################################################
#Documentation	
ADD_CUSTOM_TARGET(doc
	COMMAND doxygen Doxyfile
	)
	
