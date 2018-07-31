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
rmw_subscription_t *
rmw_create_subscription( const rmw_node_t        * node,
                         const rosidl_message_type_support_t * type_supports,
                         const char              * topic_name,
                         const rmw_qos_profile_t * qos_policies,
                         bool                      ignore_local_publications )
{
  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p topic_name: '%s' ignore_local_pubs: '%s' ]",
    __FUNCTION__,
    node, topic_name, ignore_local_publications?"yes":"no" );

  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return NULL;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
     node handle,
     node->implementation_identifier, toc_coredx_identifier,
     return NULL)
    
  const rosidl_message_type_support_t * type_support; 
  RMW_COREDX_EXTRACT_MESSAGE_TYPESUPPORT(type_supports, type_support);
    
  if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_policies is null");
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

  const message_type_support_callbacks_t * callbacks =
    static_cast<const message_type_support_callbacks_t *>(type_support->data);
  if (!callbacks) {
    RMW_SET_ERROR_MSG("callbacks handle is null");
    return NULL;
  }
  std::string type_name = _create_type_name(callbacks, "msg");
  char * topic_str     = nullptr;
  char * partition_str = nullptr;
  
  if ( ! rmw_coredx_process_topic_name( topic_name,
                                        qos_policies->avoid_ros_namespace_conventions,
                                        &topic_str,
                                        &partition_str ) )
    {
      RMW_SET_ERROR_MSG("error processing topic_name");
      return NULL;
    }
  
  // Past this point, a failure results in unrolling code in the goto fail block.
  rmw_subscription_t * subscription = nullptr;
  bool registered;
  DDS::SubscriberQos subscriber_qos;
  DDS::ReturnCode_t status;
  DDS::Subscriber * dds_subscriber = nullptr;
  DDS::Topic * topic;
  DDS::TopicDescription * topic_description = nullptr;
  DDS::DataReaderQos datareader_qos;
  DDS::DataReader * topic_reader = nullptr;
  DDS::ReadCondition * read_condition = nullptr;
  void * buf = nullptr;
  CoreDXStaticSubscriberInfo * subscriber_info = nullptr;
  // Begin initializing elements.
  subscription = rmw_subscription_allocate();
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate subscription");
    goto fail;
  }

  registered = callbacks->register_type(participant, type_name.c_str());
  if (!registered) {
    RMW_SET_ERROR_MSG("failed to register type");
    goto fail;
  }

  status = participant->get_default_subscriber_qos(subscriber_qos);
  if (status != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get default subscriber qos");
    goto fail;
  }

  // we have to set the partition array to length 1
  // and then set the partition_str in it
  if ( partition_str && 
       ( strlen(partition_str) != 0) ) {  // only set if not empty
    subscriber_qos.partition.name.resize(1);
    subscriber_qos.partition.name[0] = partition_str; // passing ownership to CoreDX
    RCUTILS_LOG_DEBUG_NAMED(
                            "rmw_coredx_cpp",
                            "%s[ set partition: '%s' ]",
                            __FUNCTION__,
                            subscriber_qos.partition.name[0] );
  }
  
  dds_subscriber = participant->create_subscriber(subscriber_qos, NULL, 0);
  // if ( partition_str && 
  //      ( strlen(partition_str) != 0) ) {  // only set if not empty
  //   subscriber_qos.partition.name[0] = nullptr;
  // }
  if (!dds_subscriber) {
    RMW_SET_ERROR_MSG("failed to create subscriber");
    goto fail;
  }

  topic_description = participant->lookup_topicdescription(topic_str);
  if (!topic_description) {
    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to get default topic qos");
      goto fail;
    }

    topic = participant->create_topic(
      topic_str, type_name.c_str(), default_topic_qos, NULL, 0);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to create topic");
      goto fail;
    }
  } else {
    DDS::Duration_t timeout(0,0);
    topic = participant->find_topic(topic_str, timeout);
    if (!topic) {
      RMW_SET_ERROR_MSG("failed to find topic");
      goto fail;
    }
  }
  rmw_free( topic_str );
  topic_str = nullptr;
  
  if (!get_datareader_qos(participant, qos_policies, datareader_qos)) {
    // error string was set within the function
    goto fail;
  }

  topic_reader = dds_subscriber->create_datareader(
    topic, datareader_qos,
    NULL, 0);
  if (!topic_reader) {
    RMW_SET_ERROR_MSG("failed to create datareader");
    goto fail;
  }

  read_condition = topic_reader->create_readcondition(
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
  if (!read_condition) {
    RMW_SET_ERROR_MSG("failed to create read condition");
    goto fail;
  }

  // Allocate memory for the CoreDXStaticSubscriberInfo object.
  buf = rmw_allocate(sizeof(CoreDXStaticSubscriberInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  // Use a placement new to construct the CoreDXStaticSubscriberInfo in the preallocated buffer.
  RMW_TRY_PLACEMENT_NEW(subscriber_info, buf, goto fail, CoreDXStaticSubscriberInfo)
  buf = nullptr;  // Only free the subscriber_info pointer; don't need the buf pointer anymore.
  subscriber_info->dds_subscriber_ = dds_subscriber;
  subscriber_info->topic_reader_ = topic_reader;
  subscriber_info->read_condition_ = read_condition;
  subscriber_info->callbacks_ = callbacks;
  subscriber_info->ignore_local_publications = ignore_local_publications;

  subscription->implementation_identifier = toc_coredx_identifier;
  subscription->data = subscriber_info;
  subscription->topic_name = do_strdup(topic_name);
  if (!subscription->topic_name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for topic name");
    goto fail;
  }

  RCUTILS_LOG_DEBUG_NAMED(
    "rmw_coredx_cpp",
    "%s[ node: %p ret: %p ]",
    __FUNCTION__,
    node, subscription );
  
  return subscription;
fail:
  if (subscription) {
    rmw_subscription_free(subscription);
  }
  // Assumption: participant is valid.
  if (dds_subscriber) {
    if (topic_reader) {
      if (read_condition) {
        if (topic_reader->delete_readcondition(read_condition) != DDS::RETCODE_OK) {
          std::stringstream ss;
          ss << "leaking readcondition while handling failure at " <<
            __FILE__ << ":" << __LINE__ << '\n';
          (std::cerr << ss.str()).flush();
        }
      }
      if (dds_subscriber->delete_datareader(topic_reader) != DDS::RETCODE_OK) {
        std::stringstream ss;
        ss << "leaking datareader while handling failure at " <<
          __FILE__ << ":" << __LINE__ << '\n';
        (std::cerr << ss.str()).flush();
      }
    }
    if (participant->delete_subscriber(dds_subscriber) != DDS::RETCODE_OK) {
      std::stringstream ss;
      std::cerr << "leaking subscriber while handling failure at " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (subscriber_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_info->~CoreDXStaticSubscriberInfo(), CoreDXStaticSubscriberInfo)
    rmw_free(subscriber_info);
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

