#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "msg/Shutdown.pb.h"
#include "def/define.pb.h"
// include protocol h file

void protocol_init();

void protocol_deinit();

template<class T> int get_type() { return 0; }
template<> int get_type<Shutdown::Req>() { return message_Shutdown_Req_; }
template<> int get_type<Shutdown::Res>() { return message_Shutdown_Res_; }
// add get_type

#endif
