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
rmw_service_t *
rmw_create_service( const rmw_node_t                    * node,
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
  if (!rmw_coredx_process_service_name(
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
  DDS::DataReader * request_datareader = nullptr;
  DDS::DataWriter * reply_datawriter = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataWriterQos datawriter_qos;
  void * replier = nullptr;
  void * buf = nullptr;
  CoreDXStaticServiceInfo * service_info = nullptr;
  rmw_service_t * service = nullptr;
  
  // Begin initializing elements.
  service = rmw_service_allocate();
  if (!service) {
    RMW_SET_ERROR_MSG("service handle is null");
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

  replier = callbacks->create_replier(
    participant, service_str,
    request_topic_name,
    reply_topic_name,
    &datareader_qos, &datawriter_qos,
    reinterpret_cast<void **>(&request_datareader),
    reinterpret_cast<void **>(&reply_datawriter),
    &rmw_allocate);
  if (!replier) {
    RMW_SET_ERROR_MSG("failed to create replier");
    goto fail;
  }
  if (!request_datareader) {
    RMW_SET_ERROR_MSG("data reader handle is null");
    goto fail;
  }
  rmw_free( service_str );
  service_str = nullptr;

  std::cout << "coredx request_topic_name: " << request_datareader->get_topicdescription()->get_name() << std::endl;
  std::cout << "coredx reply_topic_name: " << reply_datawriter->get_topic()->get_name() << std::endl;

  rmw_free( request_topic_name );
  request_topic_name = nullptr;

  rmw_free( reply_topic_name );
  reply_topic_name = nullptr;

  // update partition in the service subscriber 
  // if ( response_partition_str &&
  //      (strlen(response_partition_str) != 0) ) {
  //   DDS::Subscriber * dds_subscriber = nullptr;
  //   DDS::SubscriberQos subscriber_qos;
  //   dds_subscriber = request_datareader->get_subscriber();
  //   DDS::ReturnCode_t status = dds_subscriber->get_qos( subscriber_qos );
  //   if (status != DDS::RETCODE_OK) {
  //     RMW_SET_ERROR_MSG("failed to get default subscriber qos");
  //     goto fail;
  //   }
  //   subscriber_qos.partition.name.resize( 1 );
  //   subscriber_qos.partition.name[0] = response_partition_str;
  //   dds_subscriber->set_qos(subscriber_qos);
  //   subscriber_qos.partition.name[0] = nullptr;
  // }
  // rmw_free( response_partition_str );
  // response_partition_str = nullptr;
  
  // update partition in the service publisher 
  // if ( (request_partition_str) &&
  //      (strlen(request_partition_str) != 0) ) {
  //   DDS::Publisher * dds_publisher = nullptr;
  //   DDS::PublisherQos publisher_qos;
  //   dds_publisher = reply_datawriter->get_publisher();
  //   DDS::ReturnCode_t status = dds_publisher->get_qos( publisher_qos );
  //   if (status != DDS_RETCODE_OK) {
  //     RMW_SET_ERROR_MSG("failed to get default subscriber qos");
  //     goto fail;
  //   }
  //   publisher_qos.partition.name.resize( 1 );
  //   publisher_qos.partition.name[0] = request_partition_str;
  //   dds_publisher->set_qos(publisher_qos);
  //   publisher_qos.partition.name[0] = nullptr;
  // }
  // rmw_free( request_partition_str );
  // request_partition_str = nullptr;
  // DDS::Topic * dds_request_topic = reply_datawriter->get_topic();
  // rcutils_allocator_t    allocator   = rcutils_get_default_allocator();
  // char* request_topic_name = rcutils_format_string(allocator, "%s/%s", ros_topic_prefix, dds_request_topic->get_name());
  // dds_request_topic->set_name(request_topic_name);
  // rmw_free( request_topic_name );
  // request_topic_name = nullptr;

  read_condition = request_datareader->create_readcondition(
     DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CoreDXStaticServiceInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticServiceInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(service_info, buf, goto fail, CoreDXStaticServiceInfo)
  buf = nullptr;  // Only free the service_info pointer; don't need the buf pointer anymore.
  service_info->replier_ = replier;
  service_info->callbacks_ = callbacks;
  service_info->request_datareader_ = request_datareader;
  service_info->read_condition_ = read_condition;
  service->implementation_identifier = toc_coredx_identifier;
  service->data = service_info;
  service->service_name = do_strdup( service_name );
  if (!service->service_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p ret: %p ]",
    __FUNCTION__,
    node, service );
  
  return service;
fail:
  rmw_free( request_partition_str ); 
  rmw_free( response_partition_str );
  rmw_free( service_str );
  rmw_free( request_topic_name );
  rmw_free( reply_topic_name );
  
  if (service) {
    rmw_service_free(service);
  }
  if (request_datareader) {
    if (read_condition) {
      if (request_datareader->delete_readcondition(read_condition) != DDS_RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking readcondition while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    DDS::Subscriber * sub = request_datareader->get_subscriber();
    if ((!sub) || (sub->delete_datareader(request_datareader) != RMW_RET_OK)) {
      std::stringstream ss;
      ss << "leaking datareader while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (service_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      service_info->~CoreDXStaticServiceInfo(), CoreDXStaticServiceInfo)
    rmw_free(service_info);
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

