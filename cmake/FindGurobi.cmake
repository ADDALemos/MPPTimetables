





        find_path(GUROBI_INCLUDE_DIR
                NAMES gurobi_c++.h
                PATHS "/Library/gurobi810/mac64/include"
                )

        find_library(GUROBI_LIBRARY
                NAMES gurobi
                gurobi81

                PATHS
                "/Library/gurobi810/mac64/lib"

                )


        find_library(GUROBI_CXX_LIBRARY
                NAMES
                libgurobi_c++.a
                PATHS
                "/Library/gurobi810/mac64/lib"
                )

        # Binary dir for DLLs
        find_path(GUROBI_BIN_DIR
                NAMES "gurobi60"
                PATHS "/Library/gurobi810/mac64/bin"
                DOC "Directory containing the GUROBI DLLs"
                )

        set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}")
        set(GUROBI_LIBRARIES "${GUROBI_CXX_LIBRARY};${GUROBI_LIBRARY}")

        # use c++ headers as default
        # set(GUROBI_COMPILER_FLAGS "-DIL_STD" CACHE STRING "Gurobi Compiler Flags")

        include(FindPackageHandleStandardArgs)
        # handle the QUIETLY and REQUIRED arguments and set LIBCPLEX_FOUND to TRUE
        # if all listed variables are TRUE
        find_package_handle_standard_args(GUROBI DEFAULT_MSG
                GUROBI_CXX_LIBRARY GUROBI_LIBRARY GUROBI_INCLUDE_DIR)

        mark_as_advanced(GUROBI_INCLUDE_DIR GUROBI_LIBRARY GUROBI_CXX_LIBRARY GUROBI_BIN_DIR)



