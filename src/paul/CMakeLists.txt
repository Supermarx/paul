project(paul)

find_package(Boost COMPONENTS system program_options regex locale date_time filesystem REQUIRED)

include_directories(SYSTEM
                    ${Boost_INCLUDE_DIRS}
                    ${SuperMarx_INCLUDE_DIRS})

add_executable(paul main.cpp)
target_link_libraries(paul
					  supermarx-api
					  ${Boost_LIBRARIES})

include_directories(${paul_INCLUDE_DIRS})
