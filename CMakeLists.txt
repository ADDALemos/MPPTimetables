cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


set(SOURCE_FILES ParserXML.h problem/Course.h utils/HardwareStats.h problem/Instance.h problem/Lecture.h problem/Class.h problem/ClusterbyRoom.h problem/Room.h problem/Subpart.h problem/Unavailability.h problem/Student.h  randomGenerator/Perturbation.h randomGenerator/Perturbation.cpp  utils/Stats.h utils/TimeUtil.h  problem/Time.h problem/Curriculum.h problem/ConstraintShort.h utils/HardwareStats.h)
add_executable(timetabler ${SOURCE_FILES} main.cc)



