cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(GUROBI)
if (GUROBI_FOUND)
    set(COMISO_GUROBI_CONFIG_FILE_SETTINGS "#define COMISO_GUROBI_AVAILABLE 1")
    list(APPEND COMISO_INCLUDE_DIRECTORIES ${GUROBI_INCLUDE_DIRS})
    #  list( APPEND COMISO_LINK_DIRECTORIES    ${GUROBI_LIBRARY_DIR} )
    list(APPEND COMISO_LINK_LIBRARIES ${GUROBI_LIBRARIES})
else ()
    message(STATUS "GUROBI not found!")
    set(COMISO_GUROBI_CONFIG_FILE_SETTINGS "#define COMISO_GUROBI_AVAILABLE 0")
endif ()
find_package(Cplex)
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})
include_directories(SYSTEM ${GUROBI_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp problem/Course.h problem/Instance.h problem/Lecture.h problem/Class.h problem/Room.h problem/Subpart.h problem/Unavailability.h problem/Student.h problem/distribution.h problem/DistributionRequired.h problem/DistributionPenalty.h problem/Constraint.h problem/WithLimit.h problem/NoLimit.h problem/Limits.h solver/ILPExecuter.h randomGenerator/Perturbation.cpp randomGenerator/Perturbation.h solver/GurobiExecuter.h solver/CplexExecuter.h stats/Stats.h solver/roomLecture.h solver/GurobiAux.h solver/GRASP.cpp solver/GRASP.h)
add_executable(timetabler ${SOURCE_FILES})
target_link_libraries(timetabler ${GUROBI_LIBRARIES} ${Boost_LIBRARIES} ${CPLEX_LIBRARIES})


