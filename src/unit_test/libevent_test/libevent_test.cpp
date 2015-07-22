#include "net/go_net.h"
#include "logger/logger.h"

#include "protobuf_test/echo.pb.h"

#include <unistd.h>

namespace {
    DEQUE(net::MsgNode*) s_recv_msgs;
};

net::MsgData* new_echo_req() {
    return GO_NEW(Echo::Req);
}
void process_echo_req(net::MsgNode* msg) {
    Echo::Req* req = static_cast<Echo::Req*>(msg->msg_data_);
    if (req == NULL) {
        return;
    }

    DEBUG_LOG("收到Echo::Req消息[%d,%s]", req->code(), req->str().c_str());

    int code = req->code();
    std::string str = req->str();
    net::MsgNode* node = GO_NEW(net::MsgNode);
    node->msg_type_ = 1;
    Echo::Req* ptr = static_cast<Echo::Req*>(new_echo_req());
    ptr->set_code(code);
    ptr->set_str(str);
    node->msg_data_ = ptr;
    node->msg_conn_ = msg->msg_conn_;

    net::ThreadData* _td = msg->msg_conn_->td_;
    _td->write_msgs_lock_.Hold();
    _td->td_write_msgs_.push_back(node);
    _td->write_msgs_lock_.Release();
}

// debug_log
// trace_log
bool test_net() {
    net::regist_net_log(&logger::error_log, &logger::trace_log);

    net::NetSetting ns;
    ns.max_connections_ = 1024;
    ns.ip_addr_ = "192.168.1.253";
    ns.listen_port_ = 4444;

    if (!net::net_pre_set_parameter(ns)) {
        return false;
    }

    if (!net::net_init()) {
        return false;
    }

    net::regist_msg_processer(1, &new_echo_req, &process_echo_req);
    //net::regist_msg_processer(2, &new_echo_res, &process_echo_res);

    usleep(2000);
    DEBUG_LOG_NOARG("初始化完成");

    while (true) {
        while (s_recv_msgs.size() > 0) {
            net::MsgNode* msg_node = s_recv_msgs.front();
            s_recv_msgs.pop_front();

            net::message_process(msg_node);

            GO_DELETE(msg_node, MsgNode);
        }
        usleep(20);
        net::get_net_msgs(s_recv_msgs);    
        if (s_recv_msgs.size() > 0) {
            DEBUG_LOG("收到网络消息%d", s_recv_msgs.size());
        }
    }

    return true;
}
