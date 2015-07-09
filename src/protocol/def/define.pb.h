// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: def/define.proto

#ifndef PROTOBUF_def_2fdefine_2eproto__INCLUDED
#define PROTOBUF_def_2fdefine_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_def_2fdefine_2eproto();
void protobuf_AssignDesc_def_2fdefine_2eproto();
void protobuf_ShutdownFile_def_2fdefine_2eproto();


enum MessageType {
  message_Shutdown_Req_ = 0,
  message_Shutdown_Res_ = 1,
  message_begin_ = 20000,
  message_Regist_Req_ = 20001,
  message_Regist_Res_ = 20002
};
bool MessageType_IsValid(int value);
const MessageType MessageType_MIN = message_Shutdown_Req_;
const MessageType MessageType_MAX = message_Regist_Res_;
const int MessageType_ARRAYSIZE = MessageType_MAX + 1;

const ::google::protobuf::EnumDescriptor* MessageType_descriptor();
inline const ::std::string& MessageType_Name(MessageType value) {
  return ::google::protobuf::internal::NameOfEnum(
    MessageType_descriptor(), value);
}
inline bool MessageType_Parse(
    const ::std::string& name, MessageType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MessageType>(
    MessageType_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::MessageType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::MessageType>() {
  return ::MessageType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_def_2fdefine_2eproto__INCLUDED
