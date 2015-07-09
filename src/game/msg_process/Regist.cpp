#include "msg/Regist.pb.h"

#include "net/go_net.h"
#include "logger/logger.h"

void process_Regist_req(net::MsgNode* msg) {
	Regist::Req* req = static_cast<Regist::Req*>(msg->msg_data_);

	int id = req->my_id();
	if ((id & GATE_SERVER) == GATE_SERVER) {
		TRACE_LOG("收到网关服务器的注册连接,ID为%d", id);
	} else {
		TRACE_LOG("逻辑服务器不接受的服务器类型,ID为%d", id);
		net::conn_close(msg->msg_conn_);
	}
}
void process_Regist_res(net::MsgNode* msg) {
}
