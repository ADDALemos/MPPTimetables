cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(Cplex)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})

set(SOURCE_FILES problem/Course.h problem/Instance.h problem/Lecture.h problem/Class.h problem/Room.h problem/Subpart.h problem/Unavailability.h problem/Student.h problem/distribution.h problem/DistributionRequired.h problem/DistributionPenalty.h problem/Constraint.h problem/WithLimit.h problem/NoLimit.h problem/Limits.h solver/ILPExecuter.h randomGenerator/Perturbation.h randomGenerator/Perturbation.cpp  utils/Stats.h   solver/LocalSearch.cpp solver/LocalSearch.h  utils/TimeUtil.h solver/LSDivided.h problem/ClassbyRoom.h problem/Time.h problem/Curriculum.h problem/ConstraintShort.h)
add_executable(timetabler ${SOURCE_FILES} main.cpp )
target_link_libraries(timetabler ${CPLEX_LIBRARIES})


