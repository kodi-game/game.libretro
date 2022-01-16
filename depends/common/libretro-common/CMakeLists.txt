cmake_minimum_required(VERSION 3.0)
project(libretro-common)

INSTALL(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)
