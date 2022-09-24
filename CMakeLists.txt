cmake_minimum_required(VERSION 3.5.0)
project(libjsonparser)

set(JSON_PARSER_SRC
	json.c
	json.h
)

ADD_LIBRARY(
    libjsonparser
	STATIC
    ${JSON_PARSER_SRC}
)