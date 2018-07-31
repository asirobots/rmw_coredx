// Copyright 2015 Twin Oaks Computing, Inc.
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

#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include <rmw/rmw.h>
#include <rmw/types.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/impl/cpp/macros.hpp>
#include <rcutils/logging_macros.h>

#include <dds/dds.hh>
#include <dds/dds_builtinDataReader.hh>

#include "rmw_coredx_cpp/identifier.hpp"
#include "rmw_coredx_types.hpp"
#include "util.hpp"
#include "names.hpp"

#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************
 */
rmw_client_t *
rmw_create_client( const rmw_node_t                   * node,
                  const rosidl_service_type_support_t * type_supports,
                  const char                          * service_name,
                  const rmw_qos_profile_t             * qos_profile )
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p service_name: '%s' ]",
    __FUNCTION__,
    node, service_name );
  
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, toc_coredx_identifier,
    return NULL)

  const rosidl_service_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_SERVICE_TYPESUPPORT(type_supports, type_support);

  if (!qos_profile) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return nullptr;
  }

  auto node_info = static_cast<CoreDXNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return NULL;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return NULL;
  }

  const service_type_support_callbacks_t * callbacks =
    static_cast<const service_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  char * request_partition_str = nullptr;
  char * response_partition_str = nullptr;
  char * service_str = nullptr;
  char * request_topic_name = nullptr;
  char * reply_topic_name = nullptr;
  if ( !rmw_coredx_process_service_name(
                                        service_name,
                                        qos_profile->avoid_ros_namespace_conventions,
                                        &service_str,
                                        &request_partition_str,
                                        &response_partition_str))
    {
      RMW_SET_ERROR_MSG("error processing service_name");
      return NULL;
    }
  
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_client_t * client = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;
  DDS::DataReader * response_datareader = nullptr;
  DDS::DataWriter * request_datawriter = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * requester = nullptr;
  void * buf = nullptr;
  CoreDXStaticClientInfo * client_info = nullptr;

  // Begin inializing elements.
  client = rmw_client_allocate();
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    goto fail;
  }

  if (!get_datareader_qos(participant, qos_profile, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  if (!get_datawriter_qos(participant, qos_profile, datawriter_qos)) {
    // error string was set within the function
    goto fail;
  }

  request_topic_name = rcutils_format_string(allocator, "%s%sRequest", ros_service_requester_prefix, service_name);
  reply_topic_name = rcutils_format_string(allocator, "%s%sReply", ros_service_response_prefix, service_name);

  requester = callbacks->create_requester(
    participant, service_str,
    request_topic_name,
    reply_topic_name,
    &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&response_datareader),
    reinterpret_cast<void **>(&request_datawriter),
    &rmw_allocate);
  if (!requester) {
    RMW_SET_ERROR_MSG("failed to create requester");
    goto fail;
  }
  if (!response_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }
  rmw_free(service_str);
  service_str = nullptr;

  rmw_free( request_topic_name );
  request_topic_name = nullptr;

  rmw_free( reply_topic_name );
  reply_topic_name = nullptr;

  // update partition in the service subscriber 
  if ( response_partition_str &&
       (strlen(response_partition_str) != 0) ) {
    DDS::Subscriber * dds_subscriber = nullptr;
    DDS::SubscriberQos subscriber_qos;
    dds_subscriber = response_datareader->get_subscriber();
    DDS::ReturnCode_t status = dds_subscriber->get_qos( subscriber_qos );
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default subscriber qos");
      goto fail;
    }
    subscriber_qos.partition.name.resize( 1 );
    subscriber_qos.partition.name[0] = response_partition_str;
    dds_subscriber->set_qos(subscriber_qos);
    subscriber_qos.partition.name[0] = nullptr;
  }
  rmw_free( response_partition_str );
  response_partition_str = nullptr;

  // update partition in the service publisher 
  if ( request_partition_str &&
       (strlen(request_partition_str) != 0) ) {
    DDS::Publisher * dds_publisher = nullptr;
    DDS::PublisherQos publisher_qos;
    dds_publisher = request_datawriter->get_publisher();
    DDS::ReturnCode_t status = dds_publisher->get_qos( publisher_qos );
    if (status != DDS_RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default subscriber qos");
      goto fail;
    }
    publisher_qos.partition.name.resize( 1 );
    publisher_qos.partition.name[0] = request_partition_str;
    dds_publisher->set_qos(publisher_qos);
    publisher_qos.partition.name[0] = nullptr;
  }
  rmw_free( request_partition_str );
  request_partition_str = nullptr;
  
  read_condition = response_datareader->create_readcondition(
     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CoreDXStaticClientInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticClientInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(client_info, buf, goto fail, CoreDXStaticClientInfo)
  buf = nullptr;  // Only free the client_info pointer; don't need the buf pointer anymore.
  client_info->requester_ = requester;
  client_info->callbacks_ = callbacks;
  client_info->response_datareader_ = response_datareader;
  client_info->read_condition_ = read_condition;

  client->implementation_identifier = toc_coredx_identifier;
  client->data = client_info;
  client->service_name = do_strdup( service_name );
  if (!client->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p ret: %p ]",
    __FUNCTION__,
    node, client );
  
  return client;
fail:
  rmw_free( request_partition_str );
  rmw_free( response_partition_str );
  rmw_free( service_str );
  
  if (client_info) {
    if (response_datareader && client_info->read_condition_ ) {
      response_datareader->delete_readcondition(client_info->read_condition_);
    }
    
    // deallocate requester (currently allocated with new elsewhere)
    if (callbacks && client_info->requester_) {
      callbacks->destroy_requester(client_info->requester_, &rmw_free);
    }
    
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
                                           client_info->~CoreDXStaticClientInfo(), CoreDXStaticClientInfo)
      rmw_free(client_info);
  }
  
  if (client) {
    if (client->service_name) {
      rmw_free(const_cast<char *>(client->service_name));
    }
    rmw_client_free(client);
  }
  
  if (buf) {
    rmw_free(buf);
  }
  
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ FAILED ]",
    __FUNCTION__ );
  
  return NULL;
}


#if defined(__cplusplus)
}
#endif
  
