cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Cplex)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})


set(SOURCE_FILES main.cpp Course.h Instance.h Lecture.h Class.h Room.h Subpart.h Unavailability.h Student.h distribution.h DistributionRequired.h DistributionPenalty.h Constraint.h WithLimit.h NoLimit.h Limits.h ILP.h ILP.cpp)
add_executable(timetabler ${SOURCE_FILES} )
target_link_libraries(timetabler ${CPLEX_LIBRARIES})