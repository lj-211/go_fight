#include "protocol.h"

#include "net/go_net.h"
#include "memory/mem_define.h"

// example
/*
net::regist_msg_processer(1, &new_echo_req, &process_echo_req);
net::MsgData* new_echo_req() { return GO_NEW(Echo::Req); }
extern void process_echo_req(net::MsgNode* msg);
*/

template<> int get_type<Shutdown::Req>() { return message_Shutdown_Req_; }
template<> int get_type<Shutdown::Res>() { return message_Shutdown_Res_; }
template<> int get_type<Regist::Req>() { return message_Regist_Req_; }
template<> int get_type<Regist::Res>() { return message_Regist_Res_; }
// add_get_type
net::MsgData* new_Shutdown_req() { return GO_NEW(Shutdown::Req);  }
net::MsgData* new_Shutdown_res() { return GO_NEW(Shutdown::Res);  }
net::MsgData* new_Regist_req() { return GO_NEW(Regist::Req);  }
net::MsgData* new_Regist_res() { return GO_NEW(Regist::Res);  }
// new func
void delete_Shutdown_req(net::MsgData* ptr) { GO_DELETE((Shutdown_Req*)ptr, Shutdown_Req); }
void delete_Shutdown_res(net::MsgData* ptr) { GO_DELETE((Shutdown_Res*)ptr, Shutdown_Res); }
void delete_Regist_req(net::MsgData* ptr) { GO_DELETE((Regist_Req*)ptr, Regist_Req); }
void delete_Regist_res(net::MsgData* ptr) { GO_DELETE((Regist_Res*)ptr, Regist_Res); }
// delete func
extern void process_Shutdown_req(net::MsgNode* msg);
extern void process_Shutdown_res(net::MsgNode* msg);
extern void process_Regist_req(net::MsgNode* msg);
extern void process_Regist_res(net::MsgNode* msg);
// process func

void protocol_init() {
net::regist_msg_processer(get_type<Shutdown::Req>(), &new_Shutdown_req, &delete_Shutdown_req,&process_Shutdown_req);
net::regist_msg_processer(get_type<Shutdown::Res>(), &new_Shutdown_res, &delete_Shutdown_res,&process_Shutdown_res);
net::regist_msg_processer(get_type<Regist::Req>(), &new_Regist_req, &delete_Regist_req,&process_Regist_req);
net::regist_msg_processer(get_type<Regist::Res>(), &new_Regist_res, &delete_Regist_res,&process_Regist_res);
// regist msg
}

void protocol_deinit() {
    // no use
}
