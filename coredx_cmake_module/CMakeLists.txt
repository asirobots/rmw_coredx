# Copyright 2014 Open Source Robotics Foundation, Inc.
# Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)

project(coredx_cmake_module)

find_package(ament_cmake REQUIRED)

set(${PROJECT_NAME}_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
find_package(CoreDX MODULE)
if(CoreDX_HOME)
  normalize_path(CoreDX_HOME "${CoreDX_HOME}")
  normalize_path(CoreDX_LIBRARY_DIR "${CoreDX_LIBRARY_DIR}")
  string(REPLACE "${CoreDX_HOME}/" "" CoreDX_LIBRARY_RELATIVE_DIR "${CoreDX_LIBRARY_DIR}")
  
  if(WIN32)
    ament_environment_hooks(env_hook/coredx.bat.in)
  else()
    ament_environment_hooks(env_hook/coredx.sh.in)
  endif()
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package(
  CONFIG_EXTRAS "coredx_cmake_module-extras.cmake"
)

install(DIRECTORY cmake
  DESTINATION share/${PROJECT_NAME})
