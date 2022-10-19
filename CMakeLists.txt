cmake_minimum_required (VERSION 3.8)
set (CMAKE_CXX_STANDARD_REQUIRED 17)

project ("LabReportInstancing")

# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)

add_subdirectory(external)

target_link_libraries(${PROJECT_NAME} 
	raylib
)

target_link_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_custom_command (TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>
)