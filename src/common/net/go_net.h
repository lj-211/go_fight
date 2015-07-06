#ifndef GO_NET_H
#define GO_NET_H

#include "go_define.h"
#include "thread/thread_sync.h"

// libevent
#include "event2/event.h"
#include "event.h"

// protobuf
#include "google/protobuf/message.h"

#include <time.h>

typedef void* (*connector_fail_callback)(uint64_t ptr, bool is_ok);

namespace net {
void regist_connector_callback(connector_fail_callback cb);

typedef ::google::protobuf::Message MsgData;
class Connection;

class MsgNode {
public:
	int msg_type_;
	int msg_length_;
	MsgData* msg_data_;
	Connection* msg_conn_;

	MsgNode()
		: msg_data_(NULL),
		msg_conn_(NULL) {
	}
};


struct NetSetting {
	int max_connections_;
	std::string ip_addr_;
	int listen_port_;
	int net_thread_num_;

	NetSetting() {
		max_connections_ = 0;
		ip_addr_ = "";
		net_thread_num_ = 4;
	}
};

// accept 线程数据
struct AcceptThread {
	pthread_t thread_id_;
	event_base* base_;
};

// 注册网络模块的日志输出函数,如果没有注册,则不会输出日志
void regist_net_log(log_error_raw e_log, log_trace_raw t_log);

enum ConnState {
	CS_LISTEN,
	CS_WAITING,
	CS_WAITING_DATA,
	CS_CLOSE,
	CS_CONNECT
};

struct ConnStat {
	thread::Mutex conn_mutex_;
	time_t last_data_time_;
	uint64_t bytes_read_;
	uint64_t bytes_send_;
	time_t close_time_;

	ConnStat()
		: last_data_time_(0),
		bytes_read_(0),
		bytes_send_(0),
		close_time_(0) {
	}
};

struct ThreadData;

struct PeerInfo {
	char ip_[32];
	int port_;
};
struct Connection {
	int sfd_;
	int sfd_dup_for_write_;
	enum ConnState state_;
	thread::Mutex state_mutex_;
	// Connection中只有此数据涉及多线程操作
	// 必须通过conn_set_state操作状态
	ConnStat conn_stat_;
	event read_event_;
	bool can_read_;
	event write_event_;
	bool can_write_;
	ThreadData* td_;
	PeerInfo info_;

	struct evbuffer* read_buffer_;
	DEQUE(MsgNode*) conn_read_msgs_;
	struct evbuffer* write_buffer_;
	DEQUE(MsgNode*) conn_write_msgs_;
};

void conn_set_state(Connection* conn, ConnState state);

ConnState conn_get_state(Connection* conn);

void init_connection(Connection* conn);

void clear_connection(Connection* conn);

struct ConnInfo;
struct ConnQueue {
	LIST(ConnInfo*) connections_;
	thread::Mutex conn_mutex_;
	ConnInfo* Pop() {
		ConnInfo* info = NULL;
		conn_mutex_.Hold();
		if (connections_.size() > 0) {
			info = *connections_.begin();
			connections_.erase(connections_.begin());
		}
		conn_mutex_.Release();

		return info;
	}

	void Push(ConnInfo* info) {
		conn_mutex_.Hold();
		connections_.push_back(info);
		conn_mutex_.Release();
	}
};

struct ThreadData {
	pthread_t id_;
	event_base* event_;
	event notify_event_;

	int notify_recv_fd_;
	int notify_send_fd_;
	ConnQueue new_connections_;
	MAP(uint64_t, Connection*) for_delete_connections_;
	MAP(uint64_t, Connection*) thread_connections_;

	thread::Mutex td_read_msgs_lock_;
	DEQUE(MsgNode*) td_read_msgs_;
	
	thread::Mutex write_msgs_lock_;
	DEQUE(MsgNode*) td_write_msgs_;
};

bool net_pre_set_parameter(NetSetting& ns);
bool net_init();
void net_deinit();

// 如果返回为0,表示连接错误
uint64_t net_connect(const char* name, const char* ip, int port);

void conn_close(Connection* conn);

void get_net_msgs(DEQUE(MsgNode*)& output);

// message function
typedef void (*msg_process)(MsgNode* md);
typedef MsgData* (*new_msg_data)();

// 必须处理返回值
bool regist_msg_processer(int msg_type, new_msg_data init, msg_process mp);
void message_process(MsgNode* mn);
} // end namespace net

#endif
