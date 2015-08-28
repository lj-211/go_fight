#include "msg/Regist.pb.h"

#include "protocol.h"

#include "net/go_net.h"
#include "logger/logger.h"

extern uint64_t s_game_conn;

void regist_to_game() {
    net::MsgNode* node = net::get_msgnode(get_type<Regist::Req>());
    if (node == NULL) {
        return;
    }
    Regist::Req* req = static_cast<Regist::Req*>(node->msg_data_);
    req->set_my_id(GATE_SERVER + 1);
    node->msg_conn_ = (net::Connection*)s_game_conn;

    net::net_send(node);
}

void process_Regist_req(net::MsgNode* msg) {
}
void process_Regist_res(net::MsgNode* msg) {
}
