# Copyright 2014 Google Inc. All Rights Reserved.
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
#
# GYP include file for building the protobuf library. This defines the targets
# that are used in the 'lite' version of the runtime library. They are in turn
# included into the 'protobuf_lite_lib' and 'protobuf_lib' targets defined in
# protobuf.gyp.

{
  'sources': [
    'config.h',
    'src/src/google/protobuf/extension_set.cc',
    'src/src/google/protobuf/extension_set.h',
    'src/src/google/protobuf/generated_message_util.cc',
    'src/src/google/protobuf/generated_message_util.h',
    'src/src/google/protobuf/message_lite.cc',
    'src/src/google/protobuf/message_lite.h',
    'src/src/google/protobuf/repeated_field.cc',
    'src/src/google/protobuf/repeated_field.h',
    'src/src/google/protobuf/wire_format_lite.cc',
    'src/src/google/protobuf/wire_format_lite.h',
    'src/src/google/protobuf/wire_format_lite_inl.h',
    'src/src/google/protobuf/stubs/atomicops.h',
    'src/src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc',
    'src/src/google/protobuf/stubs/atomicops_internals_x86_msvc.h',
    'src/src/google/protobuf/stubs/common.cc',
    'src/src/google/protobuf/stubs/common.h',
    'src/src/google/protobuf/stubs/hash.h',
    'src/src/google/protobuf/stubs/map_util.h',
    'src/src/google/protobuf/stubs/once.cc',
    'src/src/google/protobuf/stubs/once.h',
    'src/src/google/protobuf/stubs/platform_macros.h',
    'src/src/google/protobuf/stubs/shared_ptr.h',
    'src/src/google/protobuf/stubs/stl_util.h',
    'src/src/google/protobuf/stubs/stringprintf.cc',
    'src/src/google/protobuf/stubs/stringprintf.h',
    'src/src/google/protobuf/stubs/template_util.h',
    'src/src/google/protobuf/stubs/type_traits.h',
    'src/src/google/protobuf/io/coded_stream.cc',
    'src/src/google/protobuf/io/coded_stream.h',
    'src/src/google/protobuf/io/coded_stream_inl.h',
    'src/src/google/protobuf/io/zero_copy_stream.cc',
    'src/src/google/protobuf/io/zero_copy_stream.h',
    'src/src/google/protobuf/io/zero_copy_stream_impl_lite.cc',
    'src/src/google/protobuf/io/zero_copy_stream_impl_lite.h',
  ],
}
