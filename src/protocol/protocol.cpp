#include "protocol.h"

#include "net/go_net.h"
#include "memory/mem_define.h"

// example
/*
net::regist_msg_processer(1, &new_echo_req, &process_echo_req);
net::MsgData* new_echo_req() { return GO_NEW(Echo::Req); }
extern void process_echo_req(net::MsgNode* msg);
*/

net::MsgData* new_Shutdown_req() { return GO_NEW(Shutdown::Req);  }
net::MsgData* new_Shutdown_res() { return GO_NEW(Shutdown::Res);  }
// new func
void delete_Shutdown_req(net::MsgData* ptr) { GO_DELETE((Shutdown_Req*)ptr, Shutdown_Req); }
void delete_Shutdown_res(net::MsgData* ptr) { GO_DELETE((Shutdown_Res*)ptr, Shutdown_Res); }
// delete func
extern void process_Shutdown_req(net::MsgNode* msg);
extern void process_Shutdown_res(net::MsgNode* msg);
// process func

void protocol_init() {
net::regist_msg_processer(get_type<Shutdown::Req>(), &new_Shutdown_req, &delete_Shutdown_req,&process_Shutdown_req);
net::regist_msg_processer(get_type<Shutdown::Res>(), &new_Shutdown_res, &delete_Shutdown_res,&process_Shutdown_res);
// regist msg
}

void protocol_deinit() {
	// no use
}
