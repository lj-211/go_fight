#include "msg/Shutdown.pb.h"

#include "net/go_net.h"
#include "logger/logger.h"

void process_Shutdown_req(net::MsgNode* msg) {
	ERROR_LOG("%s", "----------------------------------------");
}
void process_Shutdown_res(net::MsgNode* msg) {
}
