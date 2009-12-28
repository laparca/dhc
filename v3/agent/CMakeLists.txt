INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

###############################################################################
#version.h generation
ADD_CUSTOM_TARGET(version ALL
	)

ADD_CUSTOM_COMMAND(
	TARGET version
	COMMAND chmod 700 updateversion.sh
	COMMAND ./updateversion.sh
	#COMMENT "Embedding version number..."
	)
	
###############################################################################
#Assembly compilation
IF(WINDOWS)
	SET(BIN_FORMAT win32)
ELSEIF(AMD64)
	SET(BIN_FORMAT elf64)
ELSE()
	SET(BIN_FORMAT elf)
ENDIF()

IF(MACOSX)
 SET(BIN_FORMAT macho)
ENDIF()


SET(ASM_OUTPUTS "")
IF(AMD64)
	FOREACH(SRC
		md5_amd64.asm
		)
		
		#Get some file names
		STRING( REGEX REPLACE "\\.asm" ".o" DEST ${SRC})
		
		#Compilation command
		ADD_CUSTOM_COMMAND(
			OUTPUT ${DEST}
			COMMAND nasm -f ${BIN_FORMAT} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} -o ${CMAKE_CURRENT_BINARY_DIR}/${DEST}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
			COMMENT "Building asm object ${DEST}"
			)

		#Add to list of outputs
		SET(ASM_OUTPUTS ${ASM_OUTPUTS} ${DEST})
	ENDFOREACH()
ENDIF()

###############################################################################
#CUDA compilation
SET(PTX_OUTPUTS "")
SET(CUDA_OUTPUTS "")
IF(${CUDA_ENABLED})
	FOREACH(SRC
		md4_kernel.cu
		md5_kernel.cu
		md5crypt_kernel.cu
		ntlm_kernel.cu
		sha1_kernel.cu
		)
		
		#Get some file names
		STRING( REGEX REPLACE "\\_kernel.cu" ".ptx" CO_FNAME ${SRC})
		STRING( REGEX REPLACE "\\_kernel.cu" "_placeholder.cpp" PHOLDER_FNAME ${SRC})
		
		#Compilation command
		ADD_CUSTOM_COMMAND(
			OUTPUT ${PROJECT_BINARY_DIR}/ptx/${CO_FNAME}
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${PHOLDER_FNAME}
			COMMAND nvcc -ptx ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} -o ${PROJECT_BINARY_DIR}/ptx/${CO_FNAME} -I ${CMAKE_BINARY_DIR} -Xcompiler ${CMAKE_CXX_FLAGS} ${ARCH_SWITCH}
			COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/${PHOLDER_FNAME}
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
			COMMENT "Building CUDA object ptx/${CO_FNAME}"
			)
		
		
		SET(PTX_OUTPUTS "${CMAKE_BINARY_DIR}/ptx/${CO_FNAME};${CUVIS_PTX_OUTPUTS}")
		SET(CUDA_OUTPUTS ${CUDA_OUTPUTS} ${PHOLDER_FNAME})
	ENDFOREACH()
ENDIF()

###############################################################################
#C++ compilation and linking
ADD_EXECUTABLE(agent
	BaseN.cpp
	ComputeThreadProc.cpp
	ControllerLink.cpp
	DeviceMemory.cpp
	LowLevel.cpp
	MD4MeetInTheMiddleAttack.cpp
	Thread.cpp
	UnitTesting.cpp
	XmlParser.cpp
	
	${ASM_OUTPUTS}
	${CUDA_OUTPUTS}
	main.cpp
)

ADD_DEPENDENCIES(agent version)

###############################################################################
#Linker settings
IF(WINDOWS)
	IF(CUDA_ENABLED)
		TARGET_LINK_LIBRARIES(agent
			cuda libcurl)
	ELSE()
		TARGET_LINK_LIBRARIES(agent
			libcurl)
	ENDIF()
ELSE()
	IF(CUDA_ENABLED)
		TARGET_LINK_LIBRARIES(agent
			m pthread dl curl cuda)
	ELSE()
		TARGET_LINK_LIBRARIES(agent
			m pthread dl curl)
	ENDIF()
ENDIF()
