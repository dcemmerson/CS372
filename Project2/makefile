#this makefile is based off the work from https://www.youtube.com/watch?v=_r7i5X0rXJk

#Filename: makefile
#Author: Dane Emmerson
#Date: 02/09/2020
#Description: This makefile compiles the client portion of a client-server
#		chat program. See included README.

CXX = gcc
#CXXFLAGS = -std=c11
CXXFLAGS += -Wall
CXXFLAGS += -pedantic-errors
CXXFLAGS += -g
LDFLAGS = -lboost_date_time

SRCS = server_ftp.c main.c
OBJS =  server_ftp.o main.o
HEADERS = server_ftp.h


#server: ${OBJS} ${HEADERS}
#	${CXX} ${LDFLAGS} ${OBJS_S} -o server
server_ftp: ${OBJS} ${HEADERS}
	${CXX} ${OBJS} ${LDFLAGS} -o server_ftp

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.c)

.PHONY: clean all
clean: 
	rm $(OBJS) server_ftp
