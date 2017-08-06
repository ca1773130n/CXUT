set( CXUTILS_HEADERS
    include/cxut/Parallel.h
    include/cxut/Pattern.h
    include/cxut/Profile.h
)

set( CXUTILS_OBJECTS
    src/lib/Profile.cpp
    src/lib/Parallel.cpp
    ${CXUTILS_HEADERS}
)

set(CMAKE_CXX_FLAGS "-m64 -std=c++11 -fPIC ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILE_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "-g -funwind-tables" ${CMAKE_CXX_FLAGS})
else()
  set(CMAKE_CXX_FLAGS "${CFLAGS_WARN} ${CMAKE_CXX_FLAGS}")
endif()

include_directories( ${CMAKE_SOURCE_DIR}/include )
source_group( "" FILES ${CXUTILS_HEADERS} )
add_library( cxutils SHARED ${CXUTILS_OBJECTS} )

