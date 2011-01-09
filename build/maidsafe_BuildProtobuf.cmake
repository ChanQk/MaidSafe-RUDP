#==============================================================================#
#                                                                              #
#  Copyright (c) 2010 maidsafe.net limited                                     #
#  All rights reserved.                                                        #
#                                                                              #
#  Redistribution and use in source and binary forms, with or without          #
#  modification, are permitted provided that the following conditions are met: #
#                                                                              #
#      * Redistributions of source code must retain the above copyright        #
#        notice, this list of conditions and the following disclaimer.         #
#      * Redistributions in binary form must reproduce the above copyright     #
#        notice, this list of conditions and the following disclaimer in the   #
#        documentation and/or other materials provided with the distribution.  #
#      * Neither the name of the maidsafe.net limited nor the names of its     #
#        contributors may be used to endorse or promote products derived from  #
#        this software without specific prior written permission.              #
#                                                                              #
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  #
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
#  POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Written by maidsafe.net team                                                #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Module used to build Google Protocol Buffers libs & compiler.               #
#                                                                              #
#==============================================================================#

IF(MSVC)
  UNSET(PROTOBUF_SLN CACHE)
  UNSET(PROTOBUF_VSPROJECTS_DIR CACHE)
  FIND_FILE(PROTOBUF_SLN NAMES protobuf.sln PATHS ${${PROJECT_NAME}_SOURCE_DIR}/../thirdpartylibs/protobuf/vsprojects NO_DEFAULT_PATH)
  # Make a copy of .sln file to work with to avoid modified .sln being accidentally committed to repository
  IF(NOT PROTOBUF_SLN)
    SET(ERROR_MESSAGE "\nCould not find Google Protocol Buffers source.\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}The protobuf source should be in the thirdpartylibs directory.  ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/protobuf\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If protobuf is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOC_EXE_DIR=<Path to protoc directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_ROOT_DIR=<Path to protobuf root directory>\n")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ENDIF()
  FILE(COPY ${PROTOBUF_SLN} DESTINATION ${${PROJECT_NAME}_BINARY_DIR})
  FILE(RENAME ${${PROJECT_NAME}_BINARY_DIR}/protobuf.sln ${${PROJECT_NAME}_SOURCE_DIR}/../thirdpartylibs/protobuf/vsprojects/protobuf_copy.sln)
  UNSET(PROTOBUF_SLN CACHE)
  FIND_FILE(PROTOBUF_SLN NAMES protobuf_copy.sln PATHS ${${PROJECT_NAME}_SOURCE_DIR}/../thirdpartylibs/protobuf/vsprojects NO_DEFAULT_PATH)
  GET_FILENAME_COMPONENT(PROTOBUF_VSPROJECTS_DIR ${PROTOBUF_SLN} PATH)
  MESSAGE("-- Upgrading Google Protocol Buffers solution")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Upgrade OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Protocol Buffers debug libraries")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Debug|Win32" /Project libprotobuf OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Building Google Protocol Buffers release libraries and compiler")
  EXECUTE_PROCESS(COMMAND devenv ${PROTOBUF_SLN} /Build "Release|Win32" /Project protoc OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  EXECUTE_PROCESS(COMMAND CMD /C CALL extract_includes.bat WORKING_DIRECTORY ${PROTOBUF_VSPROJECTS_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  GET_FILENAME_COMPONENT(PROTOBUF_ROOT_DIR ${PROTOBUF_VSPROJECTS_DIR} PATH)
ELSE()
  UNSET(PROTOBUF_CONFIGURE CACHE)
  SET(PROTOBUF_SRC_DIR ${${PROJECT_NAME}_SOURCE_DIR}/../thirdpartylibs/protobuf)
  FIND_FILE(PROTOBUF_AUTOGEN autogen.sh PATHS ${PROTOBUF_SRC_DIR} NO_DEFAULT_PATH)
  IF(NOT PROTOBUF_AUTOGEN)
    SET(ERROR_MESSAGE "\nCould not find Google Protocol Buffers source.\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}The protobuf source should be in the thirdpartylibs directory.  ")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}You can download it at http://code.google.com/p/protobuf\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If protobuf is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_INC_DIR=<Path to protobuf include directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_LIB_DIR=<Path to protobuf lib directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOC_EXE_DIR=<Path to protoc directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_ROOT_DIR=<Path to protobuf root directory>\n")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ENDIF()
  MESSAGE("-- Generating configure file for Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_AUTOGEN} WORKING_DIRECTORY ${PROTOBUF_SRC_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(${RESVAR} EQUAL 0)
    FIND_FILE(PROTOBUF_CONFIGURE configure PATHS ${PROTOBUF_SRC_DIR} NO_DEFAULT_PATH)
  ENDIF()
  IF(NOT PROTOBUF_CONFIGURE)
    SET(ERROR_MESSAGE "${OUTVAR}\n${ERRVAR}\nCould not create configure file for Google Protocol Buffers.\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}This needs autoconf and libtool to complete successfully.")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}If protobuf is already installed, run:\n")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_INC_DIR=<Path to protobuf include directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_LIB_DIR=<Path to protobuf lib directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}${ERROR_MESSAGE_CMAKE_PATH} -DPROTOC_EXE_DIR=<Path to protoc directory> and/or")
    SET(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR_MESSAGE_CMAKE_PATH} -DPROTOBUF_ROOT_DIR=<Path to protobuf root directory>\n")
    MESSAGE(FATAL_ERROR "${ERROR_MESSAGE}")
  ENDIF()
  SET(PROTOBUF_ROOT_DIR ${CMAKE_BINARY_DIR}/thirdpartylibs/protobuf CACHE PATH "Path to Google Protocol Buffers root directory" FORCE)
  FILE(MAKE_DIRECTORY ${PROTOBUF_ROOT_DIR})
  GET_FILENAME_COMPONENT(PROTOBUF_SRC_DIR ${PROTOBUF_CONFIGURE} PATH)
  MESSAGE("-- Configuring Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND sh ${PROTOBUF_CONFIGURE} --prefix=${PROTOBUF_ROOT_DIR} --enable-shared=no WORKING_DIRECTORY ${PROTOBUF_SRC_DIR} OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}")
  ENDIF()
  MESSAGE("-- Making Google Protocol Buffers")
  MESSAGE("     This may take a few minutes...")
  EXECUTE_PROCESS(COMMAND make -C ${PROTOBUF_SRC_DIR} install OUTPUT_VARIABLE OUTVAR RESULT_VARIABLE RESVAR ERROR_VARIABLE ERRVAR)
  IF(NOT ${RESVAR} EQUAL 0)
    MESSAGE("${OUTVAR}\n${ERRVAR}")
  ENDIF()
ENDIF()
INCLUDE(${${PROJECT_NAME}_ROOT}/build/maidsafe_FindProtobuf.cmake)
