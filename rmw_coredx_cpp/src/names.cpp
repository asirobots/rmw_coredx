// Copyright 2015 Twin Oaks Computing, Inc.
// Modifications copyright (C) 2017-2018 Twin Oaks Computing, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rmw/get_topic_names_and_types.h>
#include <rmw/get_service_names_and_types.h>

#include <rcutils/logging_macros.h>
#include <rcutils/format_string.h>
#include <rcutils/types.h>
#include <rcutils/split.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"
#include "names.hpp"

#include "rosidl_typesupport_coredx_c/identifier.h"
#include "rosidl_typesupport_coredx_cpp/identifier.hpp"

/* *******************************************************
 */
// shouldn't these be promoted to rwm 'generic' level?
const char * const ros_topic_prefix = "rt";
const char * const ros_service_requester_prefix = "rq";
const char * const ros_service_response_prefix = "rr";

std::vector<std::string> _ros_prefixes =
{ros_topic_prefix, ros_service_requester_prefix, ros_service_response_prefix};

std::string
_get_ros_prefix_if_exists(const std::string & topic_name)
{
  for (auto prefix : _ros_prefixes) {
    if (topic_name.rfind(std::string(prefix) + "/", 0) == 0) {
      return prefix;
    }
  }
  return "";
}

/* *******************************************************
 */
bool
rmw_coredx_process_topic_name(
  const char * topic_name,
  bool avoid_ros_namespace_conventions,
  char ** topic_str,
  char ** partition_str)
{
  bool success = true;
  rcutils_string_array_t name_tokens = rcutils_get_zero_initialized_string_array();
  rcutils_allocator_t    allocator   = rcutils_get_default_allocator();

  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ topic_name: '%s' avoid_ros_namespace_conv: %s ]",
                          __FUNCTION__,
                          topic_name,
                          avoid_ros_namespace_conventions?"YES":"NO" );

  if ( rcutils_split_last( topic_name, '/', allocator, &name_tokens ) != RCUTILS_RET_OK )
    {
      RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
        success = false;
      goto end;
    }
  
  if ( name_tokens.size == 1 )
    {
      if ( !avoid_ros_namespace_conventions )
        {
          *topic_str = rcutils_format_string(allocator, "%s/%s", ros_topic_prefix, name_tokens.data[0]);
        }
      else
        {
          *topic_str = do_strdup( name_tokens.data[0] );
        }
    }
  else if ( name_tokens.size == 2 )
    {
      if ( avoid_ros_namespace_conventions )
        {
          // no ros_topic_prefix, so store the user's namespace directly
          *partition_str = do_strdup( name_tokens.data[0] );
        }
      else
        {
          // concat the ros_topic_prefix with the user's namespace
          char * concat_str =
            rcutils_format_string( allocator, "%s/%s", ros_topic_prefix, name_tokens.data[0] );
          if ( !concat_str )
            {
              RMW_SET_ERROR_MSG("could not allocate memory for partition string");
              success = false;
              goto end;
            }
          *partition_str = do_strdup(concat_str);
          allocator.deallocate( concat_str, allocator.state );
        }
      *topic_str = do_strdup( name_tokens.data[1] );
    }
  else
    {
      RMW_SET_ERROR_MSG("incorrectly formatted topic name");
      success = false;
    }
  
  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ %d : '%s' -> '%s' / '%s' ]",
                          __FUNCTION__,
                          name_tokens.size,
                          topic_name, 
                          *topic_str,
                          *partition_str );

 end:
  if ( rcutils_string_array_fini( &name_tokens ) != RCUTILS_RET_OK )
    {
      fprintf(stderr, "Failed to destroy the token string array\n");
    }
  return success;
}
  

/* *******************************************************
 */
bool
rmw_coredx_process_service_name(
  const char * service_name,
  bool avoid_ros_namespace_conventions,
  char ** service_str,
  char ** request_partition_str,
  char ** response_partition_str)
{
  bool success = true;
  rcutils_string_array_t name_tokens = rcutils_get_zero_initialized_string_array();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ service_name: '%s' avoid_ros_namespace_conv: %s ]",
                          __FUNCTION__,
                          service_name,
                          avoid_ros_namespace_conventions?"YES":"NO" );
  
  if ( rcutils_split_last(service_name, '/', allocator, &name_tokens) != RCUTILS_RET_OK ) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    success = false;
    goto end;
  }
  if ( name_tokens.size == 1 ) {
    if ( !avoid_ros_namespace_conventions ) {
      *request_partition_str = do_strdup( ros_service_requester_prefix );
      *response_partition_str = do_strdup( ros_service_response_prefix );
    }
    *service_str = do_strdup( name_tokens.data[0] );
  } else if ( name_tokens.size == 2 ) {
    if ( avoid_ros_namespace_conventions ) {
      // no ros_service_*_prefix, so store the user's namespace directly
      *request_partition_str = do_strdup(name_tokens.data[0]);
      *response_partition_str = do_strdup(name_tokens.data[0]);
    } else {
      // concat the ros_service_*_prefix with the user's namespace
      char * request_concat_str = rcutils_format_string(
        allocator,
        "%s/%s", ros_service_requester_prefix, name_tokens.data[0]);
      if (!request_concat_str) {
        RMW_SET_ERROR_MSG("could not allocate memory for partition string")
        success = false;
        goto end;
      }
      char * response_concat_str = rcutils_format_string(
        allocator,
        "%s/%s", ros_service_response_prefix, name_tokens.data[0]);
      if (!response_concat_str) {
        allocator.deallocate(request_concat_str, allocator.state);
        RMW_SET_ERROR_MSG("could not allocate memory for partition string")
        success = false;
        goto end;
      }
      *request_partition_str = do_strdup(request_concat_str);
      *response_partition_str = do_strdup(response_concat_str);
      allocator.deallocate(request_concat_str, allocator.state);
      allocator.deallocate(response_concat_str, allocator.state);
    }
    *service_str = do_strdup(name_tokens.data[1]);
  } else {
    RMW_SET_ERROR_MSG("Illformated service name")
  }

  RCUTILS_LOG_DEBUG_NAMED(
                          "rmw_coredx_cpp",
                          "%s[ %d : '%s' -> '%s' / req: '%s' | rep: '%s' ]",
                          __FUNCTION__,
                          name_tokens.size,
                          service_name, 
                          *service_str,
                          *request_partition_str,
                          *response_partition_str );
end:
  // free that memory
  if (rcutils_string_array_fini(&name_tokens) != RCUTILS_RET_OK) {
    fprintf(stderr, "Failed to destroy the token string array\n");
  }

  return success;
}
