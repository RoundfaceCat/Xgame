cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing patch step for libssh2")

block(SCOPE_FOR VARIABLES)

execute_process(
  WORKING_DIRECTORY "D:/Xgame/Xgame/build-verify/_deps/libssh2-src"
  COMMAND_ERROR_IS_FATAL LAST
  COMMAND  [====[D:/Cmake/bin/cmake.exe]====] [====[-DLIBSSH2_DIR=D:/Xgame/Xgame/build-verify/_deps/libssh2-src]====] [====[-DMODULES_DIR=D:/Xgame/Xgame/build-verify/_deps/sfml-src/src/SFML/Network/../../../cmake/Modules]====] [====[-P]====] [====[D:/Xgame/Xgame/build-verify/_deps/sfml-src/tools/libssh2/PatchLibssh2.cmake]====]
)

endblock()
