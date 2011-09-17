INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

###############################################################################
#C++ compilation and linking
ADD_EXECUTABLE(stresstest
	../agent/ControllerLink.cpp
	../agent/Mutex.cpp
	../agent/Thread.cpp
	../agent/XmlParser.cpp
	main.cpp
)

###############################################################################
#Linker settings
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(stresstest libcurl)
ELSE()
	TARGET_LINK_LIBRARIES(stresstest m pthread dl curl)
ENDIF()
