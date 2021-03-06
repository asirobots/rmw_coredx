// Copyright 2014-2015 Open Source Robotics Foundation, Inc.
// Modifications copyright (C) 2017 Twin Oaks Computing, Inc.
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

#ifndef ROSIDL_TYPESUPPORT_COREDX_CPP__MESSAGE_TYPE_SUPPORT_H_
#define ROSIDL_TYPESUPPORT_COREDX_CPP__MESSAGE_TYPE_SUPPORT_H_

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rmw/types.h"

typedef struct message_type_support_callbacks_t
{
  const char * package_name;
  const char * message_name;
  // Function to register type with given dds_participant
  bool (* register_type)(void * dds_participant, const char * type_name);
  // Function to publish a ROS message with a given DDS data_writer
  bool (* publish)(void * dds_data_writer, const void * ros_message);
  // Function to take a ROS message from a dds data reader
  bool (* take)(
    void * dds_data_reader, bool ignore_local_publications, void * ros_message, bool * taken,
    void * sending_publication_handle);
  bool (* convert_ros_to_dds)(
    const void * untyped_ros_message,
    void * untyped_data_message);
  bool (* convert_dds_to_ros)(
    const void * untyped_data_message,
    void * untyped_ros_message);
  bool (* serialize)( const void    * untyped_ros_msg,
		      rmw_serialized_message_t *buf );
  bool (* deserialize)( void    * untyped_ros_msg,
			const rmw_serialized_message_t *buf );
  void * (*alloc_ros_msg)( rcutils_allocator_t * allocator );
  void   (*free_ros_msg)( void * ros_msg, rcutils_allocator_t * allocator );
} message_type_support_callbacks_t;

#endif  /* ROSIDL_TYPESUPPORT_COREDX_CPP__MESSAGE_TYPE_SUPPORT_H_ */
