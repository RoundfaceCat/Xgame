cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

message(VERBOSE "Executing patch step for ogg")

block(SCOPE_FOR VARIABLES)

execute_process(
  WORKING_DIRECTORY "D:/Xgame/Xgame/build-verify/_deps/ogg-src"
  COMMAND_ERROR_IS_FATAL LAST
  COMMAND  [====[D:/Cmake/bin/cmake.exe]====] [====[-DOGG_DIR=D:/Xgame/Xgame/build-verify/_deps/ogg-src]====] [====[-P]====] [====[D:/Xgame/Xgame/build-verify/_deps/sfml-src/tools/ogg/PatchOgg.cmake]====]
)

endblock()
