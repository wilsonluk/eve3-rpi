cmake_minimum_required(VERSION 2.4)

project(case_culture_lcd)

include_directories(include)

add_executable(app	
	src/raspberry.c
	src/process.c
	src/Eve2_81x.c
	include/Arduino_AL.h
	include/Eve2_81x.h
	include/MatrixEve2Conf.h
	include/process.h
)

target_link_libraries( lcd bcm2835 )
