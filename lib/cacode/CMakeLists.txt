add_library(cacode OBJECT
        lambda_calculator.cpp
        lubksb.cpp
        ludcmp.cpp
        nrutil.cpp
        cacode.h
        )
set_target_properties(cacode PROPERTIES COMPILE_FLAGS "${BIOSNAKE_CXX_FLAGS}" LINK_FLAGS "${BIOSNAKE_CXX_FLAGS}")
