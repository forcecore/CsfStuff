# For windows platforms, Visual Studio recognizes this file. No problem.
project(CsfStuff)

set(CMAKE_CXX_STANDARD 11)

add_executable (csf2str csf2str.cpp)
add_executable (str2csf str2csf.cpp common.cpp)
add_executable (merge_str merge_str.cpp common.cpp)
