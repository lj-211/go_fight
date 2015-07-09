#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "msg/Shutdown.pb.h"
#include "msg/Regist.pb.h"
#include "def/define.pb.h"
// include protocol h file

void protocol_init();

void protocol_deinit();

template<class T> int get_type() { return 0; }
template<> int get_type<Shutdown::Req>();
template<> int get_type<Shutdown::Res>();
template<> int get_type<Regist::Req>();
template<> int get_type<Regist::Res>();
// add get_type

#endif
