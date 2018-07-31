// generated from rosidl_typesupport_coredx_cpp/resource/srv__type_support.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating <srv>__type_support.cpp files
@#
@# Context:
@#  - spec (rosidl_parser.ServiceSpecification)
@#    Parsed specification of the .srv file
@#  - get_header_filename_from_msg_name (function)
@#######################################################################
@
#ifdef CoreDX_GLIBCXX_USE_CXX11_ABI_ZERO
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__rosidl_typesupport_coredx_cpp.hpp"

#include <dds/dds.hh>
#include <dds/request_reply.hh>

#include "rmw/error_handling.h"
#include "rosidl_typesupport_coredx_cpp/identifier.hpp"
#include "rosidl_typesupport_coredx_cpp/service_type_support.h"
#include "rosidl_typesupport_coredx_cpp/service_type_support_decl.hpp"

#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name))__struct.hpp"
#include "@(spec.pkg_name)/srv/dds_coredx/@(spec.srv_name)_Request_TypeSupport.hh"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Request'))__rosidl_typesupport_coredx_cpp.hpp"
#include "@(spec.pkg_name)/srv/dds_coredx/@(spec.srv_name)_Response_TypeSupport.hh"
#include "@(spec.pkg_name)/srv/@(get_header_filename_from_msg_name(spec.srv_name + '_Response'))__rosidl_typesupport_coredx_cpp.hpp"

namespace @(spec.pkg_name)
{

namespace srv
{

namespace typesupport_coredx_cpp
{

void * create_requester__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const char * request_topic_name,
  const char * reply_topic_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  using RequesterType = DDS::rpc::Requester<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_participant || !service_name || !untyped_reader) {
    return NULL;
  }
  auto _allocator = allocator ? allocator : &malloc;

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(untyped_participant);
  const DDS::DataReaderQos * datareader_qos = static_cast<const DDS::DataReaderQos *>(untyped_datareader_qos);
  const DDS::DataWriterQos * datawriter_qos = static_cast<const DDS::DataWriterQos *>(untyped_datawriter_qos);
  DDS::rpc::RequesterParams requester_params;
  requester_params
    .domain_participant(participant)
    .service_name(service_name)
    .request_topic_name(request_topic_name)
    .reply_topic_name(reply_topic_name)
    .datareader_qos(*datareader_qos)
    .datawriter_qos(*datawriter_qos);

  RequesterType * requester = static_cast<RequesterType *>(_allocator(sizeof(RequesterType)));
  try {
    new (requester) RequesterType(requester_params);
  } catch (...) {
    RMW_SET_ERROR_MSG("C++ exception during construction of Requester");
    return NULL;
  }

  *untyped_reader = requester->get_reply_datareader();
  *untyped_writer = requester->get_request_datawriter();
  return requester;
}

const char * destroy_requester__@(spec.srv_name)(
  void * untyped_requester,
  void (* deallocator)(void *))
{
  using RequesterType = DDS::rpc::Requester<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  auto requester = static_cast<RequesterType *>(untyped_requester);

  requester->~RequesterType();
  auto _deallocator = deallocator ? deallocator : &free;
  _deallocator(requester);
  return nullptr;
}

int64_t send_request__@(spec.srv_name)(
  void * untyped_requester,
  const void * untyped_ros_request)
{
  using ROSRequestType = @(spec.pkg_name)::srv::@(spec.srv_name)_Request;
  using RequesterType = DDS::rpc::Requester<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  DDS::WriteSample<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  const ROSRequestType & ros_request = *(
    static_cast<const ROSRequestType *>(untyped_ros_request));
  @(spec.pkg_name)::srv::typesupport_coredx_cpp::convert_ros_message_to_dds(
    ros_request, request.data());

  RequesterType * requester = static_cast<RequesterType *>(untyped_requester);

  requester->send_request(request);
  const DDS::SampleIdentity_t & req_id = request.data().header.requestId;
  int64_t sequence_number = (((int64_t)req_id.seqnum.high) << 32) | req_id.seqnum.low;
  return sequence_number;
}

void * create_replier__@(spec.srv_name)(
  void * untyped_participant,
  const char * service_name,
  const char * request_topic_name,
  const char * reply_topic_name,
  const void * untyped_datareader_qos,
  const void * untyped_datawriter_qos,
  void ** untyped_reader,
  void ** untyped_writer,
  void * (*allocator)(size_t))
{
  using ReplierType = DDS::rpc::Replier<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_participant || !service_name || !untyped_reader) {
    return NULL;
  }
  auto _allocator = allocator ? allocator : &malloc;

  DDS::DomainParticipant * participant = static_cast<DDS::DomainParticipant *>(untyped_participant);
  const DDS::DataReaderQos * datareader_qos = static_cast<const DDS::DataReaderQos *>(untyped_datareader_qos);
  const DDS::DataWriterQos * datawriter_qos = static_cast<const DDS::DataWriterQos *>(untyped_datawriter_qos);
  DDS::rpc::ReplierParams replier_params;
  replier_params
    .domain_participant(participant)
    .service_name(service_name)
    .request_topic_name(request_topic_name)
    .reply_topic_name(reply_topic_name)
    .datareader_qos(*datareader_qos)
    .datawriter_qos(*datawriter_qos);

  ReplierType * replier = static_cast<ReplierType *>(_allocator(sizeof(ReplierType)));
  try {
    new (replier) ReplierType(replier_params);
  } catch (...) {
    RMW_SET_ERROR_MSG("C++ exception during construction of Requester");
    return NULL;
  }

  *untyped_reader = replier->get_request_datareader();
  *untyped_writer = replier->get_reply_datawriter();
  return replier;
}

const char * destroy_replier__@(spec.srv_name)(
  void * untyped_replier,
  void (* deallocator)(void *))
{
  using ReplierType = DDS::rpc::Replier<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  auto replier = static_cast<ReplierType *>(untyped_replier);

  replier->~ReplierType();
  auto _deallocator = deallocator ? deallocator : &free;
  _deallocator(replier);
  return nullptr;
}

bool take_request__@(spec.srv_name)(
  void * untyped_replier,
  rmw_request_id_t * request_header,
  void * untyped_ros_request)
{
  using ReplierType = DDS::rpc::Replier<
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
      @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  using ROSRequestType = @(spec.pkg_name)::srv::@(spec.srv_name)_Request;
  if (!untyped_replier || !request_header || !untyped_ros_request) {
    return false;
  }

  ReplierType * replier = static_cast<ReplierType *>(untyped_replier);

  ROSRequestType & ros_request = *static_cast<ROSRequestType *>(untyped_ros_request);

  DDS::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_> request;
  bool taken = replier->take_request(request);
  if (!taken) {
    return false;
  }
  if (!request.info().valid_data) {
    return false;
  }

  bool converted =
    @(spec.pkg_name)::srv::typesupport_coredx_cpp::convert_dds_message_to_ros(request.data(), ros_request);
  if (!converted) {
    return false;
  }

  size_t SAMPLE_IDENTITY_SIZE = 16;
  const DDS::SampleIdentity_t & id = request.data().header.requestId;
  memcpy(&request_header->writer_guid[0], &id.guid, SAMPLE_IDENTITY_SIZE);
  request_header->sequence_number = (((int64_t)id.seqnum.high) << 32) | id.seqnum.low;
  return true;
}

bool take_response__@(spec.srv_name)(
  void * untyped_requester,
  rmw_request_id_t * request_header,
  void * untyped_ros_response)
{
  using RequesterType = DDS::rpc::Requester<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  using ROSResponseType = @(spec.pkg_name)::srv::@(spec.srv_name)_Response;
  if (!untyped_requester || !request_header || !untyped_ros_response) {
    return false;
  }

  RequesterType * requester = static_cast<RequesterType *>(untyped_requester);

  ROSResponseType & ros_response = *static_cast<ROSResponseType *>(untyped_ros_response);

  DDS::Sample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  bool received = requester->take_reply(response);
  if (!received) {
    return false;
  }
  if (!response.info().valid_data) {
    return false;
  }

  int64_t sequence_number =
    (((int64_t)response.related_identity().seqnum.high) << 32) |
    response.related_identity().seqnum.low;
  request_header->sequence_number = sequence_number;

  bool converted =
    @(spec.pkg_name)::srv::typesupport_coredx_cpp::convert_dds_message_to_ros(response.data(), ros_response);
  return converted;
}

bool send_response__@(spec.srv_name)(
  void * untyped_replier,
  const rmw_request_id_t * request_header,
  const void * untyped_ros_response)
{
  using ROSResponseType = const @(spec.pkg_name)::srv::@(spec.srv_name)_Response;
  using ReplierType = DDS::rpc::Replier<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_, @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  if (!untyped_replier || !request_header || !untyped_ros_response) {
    return false;
  }
  
  DDS::WriteSample<@(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_> response;
  ROSResponseType & ros_response = *(reinterpret_cast<ROSResponseType *>(untyped_ros_response));
  bool converted =
    @(spec.pkg_name)::srv::typesupport_coredx_cpp::convert_ros_message_to_dds(ros_response, response.data());
  if (!converted) {
    return false;
  }

  DDS::SampleIdentity_t request_identity;

  size_t SAMPLE_IDENTITY_SIZE = 16;
  memcpy(request_identity.guid.value, &request_header->writer_guid[0], SAMPLE_IDENTITY_SIZE);

  request_identity.seqnum.high = (int32_t)((request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  request_identity.seqnum.low = (uint32_t)(request_header->sequence_number & 0xFFFFFFFF);

  ReplierType * replier = static_cast<ReplierType *>(untyped_replier);

  replier->send_reply(response, request_identity);
  return true;
}
 
// Function to get the type erased dds request datawriter for the requester
void *get_request_datawriter__@(spec.srv_name)(void * untyped_requester)
{
  if (!untyped_requester)
    return NULL;
  using RequesterType = DDS::rpc::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);
  return requester->get_request_datawriter();
}
 
// Function to get the type erased dds reply datawriter for the requester
void * get_reply_datareader__@(spec.srv_name)(void * untyped_requester)
{
  if (!untyped_requester)
    return NULL;
  using RequesterType = DDS::rpc::Requester<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  RequesterType * requester = reinterpret_cast<RequesterType *>(untyped_requester);
  return requester->get_reply_datareader();
}
 
// Function to get the type erased dds request datawriter for the replier
void * get_request_datareader__@(spec.srv_name)(void * untyped_replier)
{
  if (!untyped_replier)
    return NULL;
  using ReplierType = DDS::rpc::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);
  return replier->get_request_datareader();
}
 
// Function to get the type erased dds reply datawriter for the replier
void * get_reply_datawriter__@(spec.srv_name)(void * untyped_replier)
{
  if (!untyped_replier)
    return NULL;
  using ReplierType = DDS::rpc::Replier<
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Request_,
    @(spec.pkg_name)::srv::dds_::@(spec.srv_name)_Response_>;
  ReplierType * replier = reinterpret_cast<ReplierType *>(untyped_replier);
  return replier->get_reply_datawriter();
}
 
static service_type_support_callbacks_t callbacks = {
  "@(spec.pkg_name)",
  "@(spec.srv_name)",
  &create_requester__@(spec.srv_name),
  &destroy_requester__@(spec.srv_name),
  &create_replier__@(spec.srv_name),
  &destroy_replier__@(spec.srv_name),
  &send_request__@(spec.srv_name),
  &take_request__@(spec.srv_name),
  &send_response__@(spec.srv_name),
  &take_response__@(spec.srv_name),
  &get_request_datawriter__@(spec.srv_name),
  &get_reply_datareader__@(spec.srv_name),
  &get_request_datareader__@(spec.srv_name),
  &get_reply_datawriter__@(spec.srv_name),
};

static rosidl_service_type_support_t handle = {
  rosidl_typesupport_coredx_cpp::typesupport_coredx_identifier,
  &callbacks,
  get_service_typesupport_handle_function,
};

}  // namespace typesupport_coredx_cpp

}  // namespace srv

}  // namespace @(spec.pkg_name)

namespace rosidl_typesupport_coredx_cpp
{

template<>
ROSIDL_TYPESUPPORT_COREDX_CPP_EXPORT_@(spec.pkg_name)
const rosidl_service_type_support_t *
get_service_type_support_handle<@(spec.pkg_name)::srv::@(spec.srv_name)>()
{
  return &@(spec.pkg_name)::srv::typesupport_coredx_cpp::handle;
}

}  // namespace rosidl_typesupport_coredx_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_coredx_cpp, @(spec.pkg_name), @(spec.srv_name))() {
  return &@(spec.pkg_name)::srv::typesupport_coredx_cpp::handle;
}

#ifdef __cplusplus
}
#endif
