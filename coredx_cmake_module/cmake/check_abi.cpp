// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#define _GLIBCXX_USE_CXX11_ABI 0

#include <dds/dds.hh>
#include <dds/request_reply.hh>

int main(int, char **)
{
  DDSDomainParticipantFactory * dpf = DDSDomainParticipantFactory::get_instance();
  DDS::DomainParticipantQos qos;
  dpf->get_default_participant_qos( &qos );
  DDSDomainParticipant * p =
    dpf->create_participant(1, &qos, NULL, DDS::STATUS_MASK_NONE);

  DDS::rpc::RequesterParams params;
  params.service_name("foo");
  return 0;
}
