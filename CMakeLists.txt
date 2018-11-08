cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I/usr/local/Cellar/libxml++/2.40.1/include/libxml++-2.6/ -I/usr/local/Cellar/glibmm/2.58.0/include/giomm-2.4/ -I/usr/local/Cellar/glibmm/2.58.0/include/glibmm-2.4/")
set(CMAKE_CXX_STANDARD_LIBRARIES "-lxml2" )

set(SOURCE_FILES main.cpp Course.h Instance.h Lecture.h Class.h Room.h Subpart.h Unavailability.h Student.h distribution.h DistributionRequired.h DistributionPenalty.h Constraint.h WithLimit.h NoLimit.h Limits.h)
add_executable(timetabler ${SOURCE_FILES} )

