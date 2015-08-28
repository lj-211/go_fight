#include "game_tool/game_logic.h"
#include "msg/Regist.pb.h"
#include "protocol.h"
#include "net/go_net.h"
#include "logger/logger.h"


extern uint64_t s_data_conn;

void regist_to_data() {
    net::MsgNode* node = net::get_msgnode(get_type<Regist::Req>());
    if (node == NULL) {
        return;
    }
    Regist::Req* req = static_cast<Regist::Req*>(node->msg_data_);
    req->set_my_id(GAME_SERVER + 1);
    node->msg_conn_ = (net::Connection*)s_data_conn;

    net::net_send(node);
}

void process_Regist_req(net::MsgNode* msg) {
    Regist::Req* req = static_cast<Regist::Req*>(msg->msg_data_);

    int id = req->my_id();
    if ((id & GATE_SERVER) == GATE_SERVER) {
        register_server(id, (uint64_t)msg->msg_conn_);
        TRACE_LOG("收到网关服务器的注册连接,ID为%d", id);
    } else {
        TRACE_LOG("逻辑服务器不接受的服务器类型,ID为%d", id);
        net::conn_close(msg->msg_conn_);
    }
}
void process_Regist_res(net::MsgNode* msg) {
}
