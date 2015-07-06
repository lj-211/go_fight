#include "net/go_net.h"

#include "thread/thread_util.h"
#include "utils/time_util.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
//#include <linux/tcp.h>
#include <netinet/tcp.h>
#include <errno.h>

namespace {

bool s_debug_net = true;

struct MsgProcesser {
	net::msg_process mp_;
	net::new_msg_data nmd_;

	MsgProcesser()
		: mp_(NULL),
		nmd_(NULL) {
	}
};

MAP(int, MsgProcesser*) s_msg_processers;

const char* s_net_log_prefix = "[net]";

int ES_INIT_FAIL = 1024;

struct event_base* s_main_base = NULL;
//struct event* s_main_event = NULL;
net::AcceptThread s_accept_thread_info;

pthread_t s_last_thread_id = -1;

// 统计数据
struct GlobalNetStat {
	thread::Mutex gns_mutex_;
	uint64_t curr_conns_;
	uint64_t reject_conns_;
};
GlobalNetStat s_gns_stat;

// 网络全局变量
thread::Mutex s_conns_map_lock;
MAP(int, net::Connection*) s_conns_map;

thread::Mutex s_connect_conns_lock;
LIST(net::ConnInfo*) s_connect_conns;
connector_fail_callback s_cntor_callback = NULL;

MAP(pthread_t, net::ThreadData*) s_threads_datas;

// 网络参数
bool s_set_parameters = false;
net::NetSetting s_net_setting;

// 日志记录器
log_error_raw net_log_error;
log_trace_raw net_log_trace;

pthread_cond_t s_init_cond;
//int s_thread_num = 4;
int s_init_count = 0;
thread::Mutex s_init_lock;

void init_internal_parameter() {
	pthread_cond_init(&s_init_cond, NULL);
}

void deinit_internal_parameter() {
	pthread_cond_destroy(&s_init_cond);
}

// master线程等待所有线程初始化完成
void wait_for_thread_init() {
	s_init_lock.Hold();
	while (s_init_count < s_net_setting.net_thread_num_) {
		pthread_cond_wait(&s_init_cond, &s_init_lock.GetMutex());
	}
	s_init_lock.Release();
}

// worker线程初始化完成后通知master线程
void register_thread_init() {
	s_init_lock.Hold();
	++s_init_count;
	pthread_cond_signal(&s_init_cond);
	s_init_lock.Release();
}
} // end namespace annoymous

namespace net {

struct ConnInfo {
	int sfd_;
	int event_flag_;
	enum net::ConnState state_;
	PeerInfo peer_info_;
	Connection* pre_new;
	
	ConnInfo()
		: pre_new(NULL) {
	}
};

void regist_connector_callback(connector_fail_callback cb) {
	s_cntor_callback = cb;
}

bool push_worker_new_conn(ConnInfo* ci) {
	do {
		if (ci->pre_new == NULL) {
			break;
		}

		int _thread_idx = (s_last_thread_id + 1) % s_net_setting.net_thread_num_;
		s_last_thread_id = _thread_idx;
		ThreadData* _td = s_threads_datas[_thread_idx];
		// 在初始化的时候已经保障了基本数据准确,这里不需要检查指针
		//_td->id_ = _thread_idx;
		_td->new_connections_.Push(ci);

		char buf[1];
		buf[0] = 'c';
		if (write(_td->notify_send_fd_, buf, 1) != 1) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, 
					"[net]通知worker线程新发起连接失败,socket id为: %d", 
					ci->sfd_);
			}
		}

		return true;
	} while (false);

	return false;
}

bool give_worker_new_conn(int sfd, const sockaddr_in& addr, ConnState conn_state, int evt_flag) {
	do {
		ConnInfo* _ci = GO_NEW(ConnInfo);
		if (_ci == NULL) {
			break;
		}

		_ci->sfd_ = sfd;
		_ci->state_ = conn_state;
		_ci->event_flag_ = evt_flag;
		_ci->peer_info_.port_ = ntohs(addr.sin_port); 
		strncpy(_ci->peer_info_.ip_, inet_ntoa(addr.sin_addr), 32);

		int _thread_idx = (s_last_thread_id + 1) % s_net_setting.net_thread_num_;
		s_last_thread_id = _thread_idx;
		ThreadData* _td = s_threads_datas[_thread_idx];
		// 在初始化的时候已经保障了基本数据准确,这里不需要检查指针
		//_td->id_ = _thread_idx;
		_td->new_connections_.Push(_ci);

		char buf[1];
		buf[0] = 'c';
		if (write(_td->notify_send_fd_, buf, 1) != 1) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]通知worker线程新连接失败,socket id为: %d", 
					sfd);
			}
		}

		return true;
	} while (false);

	close(sfd);
	return false;
}

bool update_event(Connection* conn, short evt);
bool new_connection(Connection* conn) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	// 这里必须是while循环
	// ET触发,单次事件包含多次accept事件,如果你不处理,在没有新连接的情况下,
	// 就会漏掉新建的连接
	while (true) {
		int sfd = accept(conn->sfd_, (struct sockaddr *)&addr, &addrlen);

		if (sfd < 0) {
			if (false && net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]接收连接错误,错误原因为: %s", 
					strerror(errno));
			}
			break;			
		}

		// set socket non-block
		if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK) < 0) { 
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]接收连接时,设置socket为非阻塞错误,socket id为: %d", sfd);
			}
			close(sfd);
			continue;
		}

		give_worker_new_conn(sfd, addr, CS_WAITING, EV_READ | EV_WRITE | EV_PERSIST);

		// 修改全局数据
		s_gns_stat.gns_mutex_.Hold();
		++s_gns_stat.curr_conns_;
		s_gns_stat.gns_mutex_.Release();
	}

	update_event(conn, EV_READ);

	return false;
}

bool update_event(Connection* conn, short evt);

void message_format(struct evbuffer *buffer,
    const struct evbuffer_cb_info *info, void *arg)
{
	if (arg == NULL || info->n_deleted > 0) {
		return;
	}
	Connection* conn = static_cast<Connection*>(arg);
	if (buffer != conn->read_buffer_) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]收到读取Buffer和连接属性不匹配");
		}
		return;
	}

	// 读取前8个字节
	int sum_size = info->orig_size + info->n_added;
	if (sum_size > 8) {
		int n = evbuffer_peek(buffer, 8, NULL, NULL, 0);
		if (n <= 0) {
			return;
		}
		struct evbuffer_iovec* vecs = 
			static_cast<evbuffer_iovec*>(GO_MALLOC(sizeof(evbuffer_iovec) * n));
		evbuffer_peek(buffer, 8, NULL, vecs, n);
		
		char tmp_len[4] = {0};
		int sum = 0;
		for (int i = 0; i < n; ++i) {
			bool done = false;
			char* buf = static_cast<char*>(vecs[i].iov_base);
			//memcpy(tmp_len, buf+4, 4);
			for (size_t j = 0; j < vecs[i].iov_len; ++j) {
				++sum;
				if (sum > 4 && sum < 9) {
					memcpy(tmp_len+sum-5, buf+j, 1);
				}
				if (sum >= 9) {
					done = true;
					break;
				}
			}
			if (done) {
				break;
			}
		}

		int msg_len = *((int*)tmp_len);

		if (sum_size < (msg_len + 8)) {
			return;
		}

		char* msg = (char*)GO_MALLOC(8+msg_len);
		evbuffer_remove(buffer, (void*)msg, msg_len + 8);
		do {
			MsgNode* msg_node = GO_NEW(MsgNode);
			msg_node->msg_type_ = *((int*)msg);
			msg_node->msg_conn_ = conn;
			MAP(int, MsgProcesser*)::iterator pro_iter = 
				s_msg_processers.find(msg_node->msg_type_);
			if (pro_iter == s_msg_processers.end()) {
				// 如果是未识别的类型,则断开连接
				conn_close(conn);
				GO_DELETE(msg_node, MsgNode);
				break;
			}

			msg_node->msg_data_ = (*pro_iter->second->nmd_)();
			msg_node->msg_data_->ParseFromArray(msg+8, msg_len);

			// 压缩到队列中
			conn->conn_read_msgs_.push_back(msg_node);
			if (s_debug_net && net_log_trace) {
				net_log_trace(__FILE__, __LINE__, "网络层收到[%s:%d]的消息%d", conn->info_.ip_, 
					conn->info_.port_, msg_node->msg_type_);
			}
		} while (false);

		GO_FREE(msg);
	}
}

void on_read(Connection* conn) {
	if (conn == NULL) {
		return;
	}

	ConnState cs = conn_get_state(conn);
	if (cs == CS_CLOSE) {
		return;
	}

	do {
		int fd = conn->sfd_;
		// -1 means read as many as it can
		int ret = evbuffer_read(conn->read_buffer_, fd, -1);
		// EOF
		if (ret == 0) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]对方网络连接断开,描述符%d,连接信息(ip:%s port:%d)",
						conn->sfd_, conn->info_.ip_, conn->info_.port_);
			}
			conn_close(conn);
			break;
		} else if (ret < 0) {
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]读取socket错误,描述符%d,连接信息(ip:%s port:%d),错误原因%s",
							conn->sfd_, conn->info_.ip_, conn->info_.port_, strerror(errno));
				}
				conn_close(conn);
				break;
			} else {
				if (false == update_event(conn, EV_READ)) {
					if (net_log_error) {
						net_log_error(__FILE__, __LINE__, "[net]on_read更新连接事件错误,描述符%d,连接信息(ip:%s port:%d)",
								conn->sfd_, conn->info_.ip_, conn->info_.port_);
					}
					conn_close(conn);
					break;
				}
			}
		} else {
			conn->can_read_ = true;
		}

		return;
	} while (false);

	conn->can_read_ = false;

	return;
}

void free_msg_buf(const void *data, size_t len, void *arg) {
	void* tmp = const_cast<void*>(data);
	GO_FREE(tmp);
}

void on_write(Connection* conn) {
	if (conn == NULL) {
		return;
	}

	ConnState cs = conn_get_state(conn);
	if (cs == CS_CLOSE) {
		return;
	}

	// 向Buffer中写入数据
	while (conn->conn_write_msgs_.size() > 0) {
		MsgNode* msg_node = conn->conn_write_msgs_.front();
		conn->conn_write_msgs_.pop_front();
		int msg_data_size = msg_node->msg_data_->ByteSize();
		char* buf = (char*)GO_MALLOC(msg_data_size+8);
		// 1. 写入消息类型
		*((int*)buf) = msg_node->msg_type_;
		// 2. 写入消息长度
		*((int*)buf+4) = msg_data_size;
		// 3. 写入消息内容
		msg_node->msg_data_->SerializeToArray(buf+8, msg_data_size);
		evbuffer_add_reference(conn->write_buffer_, buf, msg_data_size+8, free_msg_buf, NULL);
		// 通过libevent源码分析,Connection释放evbuffer时,会清理引用的
		// 内存块
	}

	do {
		size_t buffer_len = evbuffer_get_length(conn->write_buffer_);
		if (buffer_len == 0) {
			// 没有可写数据,将状态置为可写
			if (conn->can_write_ == false && net_log_trace) {
				//if (net_log_trace) {
				net_log_trace(__FILE__, __LINE__, "连接%d没有可写数据,置为可写状态", conn->sfd_dup_for_write_);
			}
			conn->can_write_ = true;
		} else {
			int ret = evbuffer_write(conn->write_buffer_, conn->sfd_dup_for_write_);
			if (ret == -1) {
				if ((errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
					if (false == update_event(conn, EV_WRITE)) {
						if (net_log_error) {
							net_log_error(__FILE__, __LINE__, "[net]on_write更新连接事件错误,描述符%d,连接信息(ip:%s port:%d)",
								conn->sfd_, conn->info_.ip_, conn->info_.port_);
						}
						break;
					}
				} else {
					conn->can_write_ = false;
				}
			} else {
				if (s_debug_net && net_log_trace) {
					net_log_trace(__FILE__, __LINE__, "发送数据长度为%d", ret);
				}
				conn->can_write_ = true;
				evbuffer_drain(conn->write_buffer_, ret);
			}
		}

		return;
	} while (false);

	conn_close(conn);
	return;
}

void process_event(Connection* conn, int evt) {
	if (conn == NULL) {
		return;
	}

	ConnState cs = conn_get_state(conn);
	if (cs == CS_LISTEN) {
		new_connection(conn);
	} else if ((evt & EV_READ) == EV_READ) {
		on_read(conn);
	} else if ((evt & EV_WRITE) == EV_WRITE) {
		on_write(conn);
	}
}

void event_handler(const int fd, const short which, void *arg) {
	if (arg == NULL) {
		return;
	}

	Connection* c = static_cast<net::Connection*>(arg);
	if (which == EV_READ && fd != c->sfd_) {
		conn_close(c);
		return;
	}
	if (which == EV_WRITE && fd != c->sfd_dup_for_write_) {
		conn_close(c);
		return;
	}

	process_event(c, (int)which);
}

int new_socket_nonblock() {
	int sfd;
	int flags;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]创建socket失败");
		}
		return -1;
	}

	if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 ||
			fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(sfd);
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]设置socket文件描述符为非阻塞失败");
		}

		return -1;
	}

	return sfd;
}

Connection* create_conn(const int sfd, ConnState conn_state, event_base* ebase, Connection* pre_new);
int server_listen(const char* addr_str, int port) {
	int sfd = new_socket_nonblock();
	if (sfd == -1) {
		return sfd;
	}

	do {
		int flags = 1;
		struct linger ling = {0, 0};
		{
			setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
			setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
			setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
			// keepalive
			{
				int keep_alive = 1;
				int keep_idle = 60;
				int keep_interval = 5;
				int keep_count = 3;
				setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive , sizeof(keep_alive));
				setsockopt(sfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keep_idle , sizeof(keep_idle));
				setsockopt(sfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval , sizeof(keep_interval));
				setsockopt(sfd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count));
			}

			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			inet_aton(addr_str, &(addr.sin_addr));
			addr.sin_port = htons(port);

			if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "%s绑定socket失败,ip为: %s, 端口为: %d",
							s_net_log_prefix, addr_str, port);
				}
				break;
			}

			// listen的backlog参数影响连接建立过程
			if (0 != listen(sfd, 32)) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]监听文件描述符失败");
				}
				break;
			}

			// 创建连接数据结构
			Connection* conn = create_conn(sfd, CS_LISTEN, s_accept_thread_info.base_, NULL);
			if (conn == NULL) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]创建监听连接失败,退出程序.");
				}
				break;
			}
		}

		return sfd;
	} while (false);

	if (sfd != -1) {
		close(sfd);
	}
	sfd = -1;

	return sfd;
}

Connection* create_conn(const int sfd, ConnState conn_state, event_base* ebase, Connection* pre_new) {
	Connection* _ret = NULL;

	do {
		if (sfd < 0) {
			_ret = NULL;
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]创建连接数据失败,原因: 文件描述符小于0");
			}
			break;
		}

		if (ebase == NULL) {
			_ret = NULL;
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]创建连接数据失败,原因: 事件驱动器为空");
			}
			break;
		}

		// 检查文件描述符是否已经存在
		{
			thread::AutoLockMutex auto_lock(&s_conns_map_lock);
			if (s_conns_map.find(sfd) != s_conns_map.end()) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]创建连接失败,原因: 重复的socket id - %d", sfd);
				}
				break;
			}
		}


		if (pre_new == NULL) {
			_ret = GO_NEW(Connection);
		} else {
			_ret = pre_new;
		}
		init_connection(_ret);

		// set attribute
		{
			_ret->sfd_ = sfd;
			if (conn_state != CS_LISTEN) {
				_ret->sfd_dup_for_write_ = dup(sfd);
				_ret->state_ = conn_state;
			}

			// read
			event_set(&_ret->read_event_, sfd, EV_READ | EV_ET, event_handler, (void *)_ret);
			event_base_set(ebase, &_ret->read_event_);

			if (conn_state != CS_LISTEN) {
				// write
				event_set(&_ret->write_event_, _ret->sfd_dup_for_write_, EV_WRITE | EV_ET, 
						event_handler, (void *)_ret);
				event_base_set(ebase, &_ret->write_event_);
			}

			if (event_add(&_ret->read_event_, 0) == -1) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]增加新连接读事件错误,连接socket id为%d,连接状态为%d",
							sfd, (int)conn_state);
				}
				break;
			}
			if (conn_state != CS_LISTEN) {
				if (event_add(&_ret->write_event_, 0) == -1) {
					if (net_log_error) {
						net_log_error(__FILE__, __LINE__, "[net]增加新连接写事件错误,连接socket id为%d,连接状态为%d",
								_ret->sfd_dup_for_write_, (int)conn_state);
					}
					break;
				}
			}
		}

		s_conns_map_lock.Hold();
		s_conns_map[sfd] = _ret;
		s_conns_map_lock.Release();

		// 设置网络全局数据
		s_gns_stat.gns_mutex_.Hold();
		++(s_gns_stat.curr_conns_);
		s_gns_stat.gns_mutex_.Release();

		if (net_log_trace) {
			net_log_trace(__FILE__, __LINE__, "%s创建新的连接,连接id为%d", s_net_log_prefix, sfd);
		}

		return _ret;
	} while (false);

	if (_ret) {
		if (_ret->sfd_dup_for_write_ != -1) {
			close(_ret->sfd_dup_for_write_);
			_ret->sfd_dup_for_write_ = -1;
		}
		if (_ret->sfd_ != -1) {
			close(_ret->sfd_);
			_ret->sfd_ = -1;
		}
		clear_connection(_ret);
		// 不是预分配的连接,由外面负责释放
		if (pre_new == NULL) {
			GO_DELETE(_ret, Connection);
		}
		_ret = NULL;
	}

	return _ret;
}

bool update_event(Connection* conn, short evt) {
	if (conn == NULL) {
		return false;
	}

	struct event_base* _base = NULL;
	struct event* _event = NULL;
	int fd = -1;
	int event_flag = 0;
	if (evt == EV_READ) {
		_base = conn->read_event_.ev_base;
		_event = &conn->read_event_;
		fd = conn->sfd_;
		event_flag = EV_READ | EV_ET;
	} else if (evt == EV_WRITE) {
		_base = conn->write_event_.ev_base;
		_event = &conn->write_event_;
		fd = conn->sfd_dup_for_write_;
		event_flag = EV_WRITE | EV_ET;
	}

	if (_base == NULL) {
		return false;
	}

	if (event_del(_event) == -1) {
		return false;
	}

	event_set(_event, fd, event_flag, event_handler, (void *)conn);
	event_base_set(_base, _event);
	if (event_add(_event, 0) == -1) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]更新事件失败失败,文件描述符为%d", fd);
		}
		return false;
	}

	return true;
}

void regist_net_log(log_error_raw e_log, log_trace_raw t_log) {
	net_log_error = e_log;
	net_log_trace = t_log;
}

void thread_init(int num_thread, event_base* main_base);
void* accept_thread_worker(void* ptr) {
	if (ptr == NULL) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]接收连接线程任务错误,退出程序!");
		}
		exit(ES_INIT_FAIL);
	}
	// 1. 初始化数据处理线程任务
	thread_init(s_net_setting.net_thread_num_, s_main_base);
	// 2. 等待处理数据线程初始化
	wait_for_thread_init();
	// 3. 进入主事件循环
	event_base* main_base = static_cast<event_base*>(ptr);
	event_base_loop(main_base, 0);

	return NULL;
}

void conn_internal_close(Connection* conn);
void* data_thread_worker(void* ptr) {
	if (ptr == NULL) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]数据处理线程任务错误,退出程序");
			exit(ES_INIT_FAIL);
		}
	}

	// 通知accept线程,数据处理线程操作完成
	register_thread_init();

	// 进入数据event循环
	ThreadData* td = static_cast<ThreadData*>(ptr);
	if (td->event_ == NULL) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]数据处理线程没有正确的事件数据,退出程序");
		}
		exit(ES_INIT_FAIL);
	}
	while (true) {
		time_t now = util::go_time::get_s_time();

		// 将ThreadData中的发送消息分配到Connection中
		td->write_msgs_lock_.Hold();
		while (td->td_write_msgs_.size() > 0) {
			MsgNode* msg_node = td->td_write_msgs_.front();
			td->td_write_msgs_.pop_front();

			// 查找connection
			uint64_t key = (uint64_t)msg_node->msg_conn_;
			MAP(uint64_t, Connection*)::iterator conn_iter = 
				td->thread_connections_.find(key);
			if (conn_iter == td->thread_connections_.end() || conn_iter->second == NULL) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, "[net]消息的连接失效,消息类型%d", 
						msg_node->msg_type_);
				}
				continue;
			}

			conn_iter->second->conn_write_msgs_.push_back(msg_node);
			if (s_debug_net && net_log_trace) {
				net_log_error(__FILE__, __LINE__, "连接[%s:%d]收到消息", 
					conn_iter->second->info_.ip_,
					conn_iter->second->info_.port_);
			}
		}
		td->write_msgs_lock_.Release();

		// 将Connection中的消息放到线程队列中,供逻辑层调用
		{
			MAP(uint64_t, Connection*)::iterator msg_iter = td->thread_connections_.begin();
			while (msg_iter != td->thread_connections_.end()) {
				Connection* tmp_conn = msg_iter->second;
				while (tmp_conn->conn_read_msgs_.size() > 0) {
					td->td_read_msgs_lock_.Hold();
					td->td_read_msgs_.push_back(tmp_conn->conn_read_msgs_.front());
					tmp_conn->conn_read_msgs_.pop_front();
					td->td_read_msgs_lock_.Release();
				}
				++msg_iter;
			}
		}

		// 1. 处理connector
		{
			thread::AutoLockMutex tmp(&s_connect_conns_lock);
			LIST(ConnInfo*)::iterator new_conn_iter = s_connect_conns.begin();
			while (new_conn_iter != s_connect_conns.end()) {
				ConnInfo* ci = *new_conn_iter;
				if (false == push_worker_new_conn(ci)) {
					if (s_cntor_callback) {
						s_cntor_callback((uint64_t)ci->pre_new, false);
					}

					GO_DELETE(ci->pre_new, Connection);
					GO_DELETE(ci, ConnInfo);
					close(ci->sfd_);
					return 0;
				}

				s_connect_conns.erase(new_conn_iter);
				new_conn_iter = s_connect_conns.begin();
			}
		}

		// 2. 处理依然是可读状态fd
		MAP(uint64_t, Connection*)::iterator io_iter = td->thread_connections_.begin();
		while (io_iter != td->thread_connections_.end()) {
			Connection* tmp_conn = io_iter->second;
			ConnState cs = conn_get_state(tmp_conn);
			if (cs != CS_CLOSE) {
				if (tmp_conn->can_read_) {
					process_event(tmp_conn, EV_READ);
				}
				if (tmp_conn->can_write_) {
					process_event(tmp_conn, EV_WRITE);
				}
			}
			++io_iter;
		}
		// 3. epoll_wait
		event_base_loop(td->event_, EVLOOP_ONCE | EVLOOP_NONBLOCK);
		// 4. 处理异常连接
		VECTOR(uint64_t) remove_ids;
		MAP(uint64_t, Connection*)::iterator tc_iter = td->thread_connections_.begin();
		while (tc_iter != td->thread_connections_.end()) {
			if (conn_get_state(tc_iter->second) == CS_CLOSE) {
				// 给断开的连接增加一个连接断开消息
				MsgNode* msg_node = GO_NEW(MsgNode);
				msg_node->msg_type_ = 0;
				msg_node->msg_conn_ = tc_iter->second;
				td->td_read_msgs_lock_.Hold();
				td->td_read_msgs_.push_back(msg_node);
				td->td_read_msgs_lock_.Release();
				// 立即关闭并清理连接数据,但是保留Connection对象
				conn_internal_close(tc_iter->second);

				remove_ids.push_back(tc_iter->first);
				td->for_delete_connections_[tc_iter->second->conn_stat_.close_time_] = tc_iter->second;
			}

			++tc_iter;
		}
		for (size_t i = 0; i < remove_ids.size(); ++i) {
			td->thread_connections_.erase(remove_ids[i]);
		}

		MAP(uint64_t, Connection*)::iterator delete_iter = td->for_delete_connections_.begin();
		while (delete_iter != td->for_delete_connections_.end()) {
			if ((now - delete_iter->second->conn_stat_.close_time_) > (2 * 60)) {
				//conn_internal_close(delete_iter->second);
				GO_DELETE(delete_iter->second, Connection);
			} else {
				break;
			}
			td->for_delete_connections_.erase(delete_iter);
			delete_iter = td->for_delete_connections_.begin();
		}
	}

	return NULL;
}

void thread_libevent_process(int fd, short which, void *arg) {
	if (arg == NULL) {
		return;
	}
	ThreadData* _td = static_cast<ThreadData*>(arg);
	char buf[1];
	if (read(fd, buf, 1) != 1) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]无法读取pipe数据");
		}
		return;
	}

	if (buf[0] == 'c') {
		ConnInfo* ci = _td->new_connections_.Pop();

		do {
			Connection* conn = create_conn(ci->sfd_, ci->state_, _td->event_, ci->pre_new);
			if (conn == NULL) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, 
						"%s工作线程接收连接失败,socket id为%d,(ip:%s port:%d)",
						s_net_log_prefix, fd, ci->peer_info_.ip_, ci->peer_info_.port_);
				}
				if (ci->sfd_ != -1) {
					close(ci->sfd_);
				}
				break;
			}
			if (_td->thread_connections_.find(ci->sfd_) != _td->thread_connections_.end()) {
				if (net_log_error) {
					net_log_error(__FILE__, __LINE__, 
						"%ssocket文件描述符已存在线程队列中,socket id为%d,(ip:%s port:%d)",
						s_net_log_prefix, fd, ci->peer_info_.ip_, ci->peer_info_.port_);
				}
				if (ci->sfd_ != -1) {
					close(ci->sfd_);
				}
				break;
			}
			_td->thread_connections_[(uint64_t)conn] = conn;
			conn->td_ = _td;
			memcpy((void*)&conn->info_, (void*)&ci->peer_info_, sizeof(PeerInfo));

			if (ci->pre_new != NULL && s_cntor_callback) {
				s_cntor_callback((uint64_t)ci->pre_new, true);
			}
		} while (false);

		if (ci->pre_new != NULL) {
			if (s_cntor_callback) {
				s_cntor_callback((uint64_t)ci->pre_new, false);
			}
		
			GO_DELETE(ci->pre_new, Connection);
		}
		GO_DELETE(ci, ConnInfo);
	}
}

bool setup_thread(ThreadData* td) {
	if (td == NULL) {
		return false;
	}

	td->event_ = event_init();
	if (td->event_ == NULL) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]创建线程事件主体失败,退出程序");
		}
		exit(ES_INIT_FAIL);
	}

	event_set(&td->notify_event_, td->notify_recv_fd_, EV_READ | EV_WRITE | EV_PERSIST, 
		thread_libevent_process, (void*)td);
	event_base_set(td->event_, &td->notify_event_);
	if (event_add(&td->notify_event_, 0) == -1) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]设置线程事件错误,退出事件");
		}
		return false;
	}

	return true;
}

void thread_init(int num_thread, event_base* main_base) {
	// s_threads_datas
	for (int i = 0; i < num_thread; ++i) {
		ThreadData* _td = GO_NEW(ThreadData);

		int fd[2];
		int pipe_ret = pipe(fd);
		if (pipe_ret != 0) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]初始化线程失败,退出游戏,原因为%s", 
					strerror(errno));
				exit(ES_INIT_FAIL);
			}
		}

		_td->notify_recv_fd_ = fd[0];
		_td->notify_send_fd_ = fd[1];

		if (setup_thread(_td) == false) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]初始化线程数据错误,退出程序");
			}
			exit(ES_INIT_FAIL);
		}

		bool result = thread::create_worker(_td->id_, data_thread_worker, (void*)_td);
		if (result == false) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]初始化线程错误,退出程序");
			}
			exit(ES_INIT_FAIL);
		}

		s_threads_datas[i] = _td;
	}
}

void conn_set_state(Connection* conn, ConnState state) {
	conn->state_mutex_.Hold();
	if (conn->state_ != state) {
		conn->state_ = state;
		if (conn->state_ == CS_CLOSE) {
			// 关闭事件不需要加锁
			conn->conn_stat_.close_time_ = util::go_time::get_s_time();
		}
	}
	conn->state_mutex_.Release();
}

ConnState conn_get_state(Connection* conn) {
	ConnState ret;	
	conn->state_mutex_.Hold();
	ret = conn->state_;
	conn->state_mutex_.Release();

	return ret;
}

void init_connection(Connection* conn) {
	if (conn == NULL) {
		return;
	}

	conn->sfd_ = -1;
	conn->sfd_dup_for_write_ = -1;
	conn->can_read_ = false;
	conn->can_write_ = false;

	conn->read_buffer_ = evbuffer_new();
	evbuffer_add_cb(conn->read_buffer_, message_format, conn);
	conn->write_buffer_ = evbuffer_new();
}

void clear_connection(Connection* conn) {
	if (conn == NULL) {
		return;
	}

	if (conn->read_buffer_) {
		evbuffer_free(conn->read_buffer_);
	}

	if (conn->write_buffer_) {
		evbuffer_free(conn->write_buffer_);
	}
}

bool net_pre_set_parameter(NetSetting& ns) {
	do {
		// 检查参数设置,如果有不合理参数返回错误
		// 1. 最大连接数
		if (ns.max_connections_ > 0) {
			s_net_setting.max_connections_ = ns.max_connections_;
		} else {
			break;
		}
		// 2. 地址
		s_net_setting.ip_addr_ = ns.ip_addr_;
		// 3. 端口
		s_net_setting.listen_port_ = ns.listen_port_;
		// 4. 网络线程数量
		s_net_setting.net_thread_num_ = ns.net_thread_num_;
		
		s_set_parameters = true;

		return true;

	} while (false);

	return false;
}

bool net_init() {
	do {
		if (s_set_parameters == false) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]没有完整设置网络系统参数,初始化失败.");
			}
			break;
		}

		// 1. 初始化基本数据
		//	 1) 全局统计数据
		//	 2) xxxx

		// 2. 创建主event
		s_main_base = event_init();
		s_accept_thread_info.base_ = s_main_base;

		// 3. 创建监听
		int listen_fd = server_listen(s_net_setting.ip_addr_.c_str(), s_net_setting.listen_port_);
		if (listen_fd == -1) {
			if (net_log_error) {
				net_log_error(__FILE__, __LINE__, "[net]创建监听连接失败,退出程序.");
			}
			exit(ES_INIT_FAIL);
		} else {
			if (net_log_trace) {
				net_log_trace(__FILE__, __LINE__, "[net]创建监听连接%d", listen_fd);
			}	
		}
		
		// 4. 启动accept线程,并进入主事件循环
		pthread_t accept_thread;
		if (!thread::create_worker(accept_thread, accept_thread_worker, (void*)s_accept_thread_info.base_)) {
			break;
		}
		s_accept_thread_info.thread_id_ = accept_thread;

		return true;
	} while (false);

	return false;
}

void net_deinit() {
}

void conn_internal_close(Connection* conn) {
	if (net_log_trace) {
		net_log_trace(__FILE__, __LINE__, "删除连接,读[%d]写[%d]", conn->sfd_, 
			conn->sfd_dup_for_write_);
	}

	// 从全局列表中删除
	s_conns_map_lock.Hold();
	MAP(int, Connection*)::iterator iter = s_conns_map.find(conn->sfd_);
	if (iter != s_conns_map.end()) {
		s_conns_map.erase(iter);
	}
	s_conns_map_lock.Release();

	clear_connection(conn);

	event_del(&conn->read_event_);
	event_del(&conn->write_event_);

	if (conn->sfd_dup_for_write_ != -1) {
		close(conn->sfd_dup_for_write_);
	}
	if (conn->sfd_dup_for_write_ != -1) {
		close(conn->sfd_);
	}

	s_gns_stat.gns_mutex_.Hold();
	--s_gns_stat.curr_conns_;
	s_gns_stat.gns_mutex_.Release();
}

void conn_close(Connection* conn) {
	conn_set_state(conn, CS_CLOSE);
}

uint64_t net_connect(const char* name, const char* ip, int port) {
	if (s_cntor_callback == NULL) {
		// 如果没有设置连接器回调,不允许连接
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]在创建主动连接前,你必须注册创建连接失败回调");
		}
		return 0;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	inet_aton(ip, &(addr.sin_addr));
	addr.sin_port = port;

	if (connect(fd, (struct sockaddr*)(&addr), sizeof(addr)) != 0) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]连接%s,ip: %s port: %d失败",
				name, ip, port);
		}
		close(fd);

		return 0;
	}

	Connection* new_conn = GO_NEW(Connection);	

	ConnInfo* _ci = GO_NEW(ConnInfo);
	_ci->sfd_ = fd;
	_ci->state_ = CS_CONNECT;
	_ci->event_flag_ = EV_READ | EV_WRITE | EV_PERSIST;
	_ci->peer_info_.port_ = ntohs(addr.sin_port); 
	_ci->pre_new = new_conn;
	strncpy(_ci->peer_info_.ip_, inet_ntoa(addr.sin_addr), 32);

	thread::AutoLockMutex tmp(&s_connect_conns_lock);
	s_connect_conns.push_back(_ci);

	return (uint64_t)new_conn;
}

void get_net_msgs(DEQUE(MsgNode*)& output) {
	for (size_t i = 0; i < s_threads_datas.size(); ++i) {
		ThreadData* td = s_threads_datas[i];
		td->td_read_msgs_lock_.Hold();
		while (td->td_read_msgs_.size() > 0) {
			output.push_back(td->td_read_msgs_.front());
			td->td_read_msgs_.pop_front();
		}
		td->td_read_msgs_lock_.Release();
	}
}

} // end namespace net

// for msg
namespace net {
void init_msg_node() {
}

bool regist_msg_processer(int msg_type, new_msg_data init, msg_process mp) {
	MAP(int, MsgProcesser*)::iterator iter = s_msg_processers.find(msg_type);
	if (iter != s_msg_processers.end()) {
		return false;
	}

	s_msg_processers[msg_type] = GO_NEW(MsgProcesser);
	s_msg_processers[msg_type]->mp_ = mp;
	s_msg_processers[msg_type]->nmd_ = init;

	return true;
}

void message_process(MsgNode* mn) {
	if (mn == NULL) {
		return;
	}

	MAP(int, MsgProcesser*)::iterator iter = s_msg_processers.find(mn->msg_type_);
	if (iter == s_msg_processers.end()) {
		if (net_log_error) {
			net_log_error(__FILE__, __LINE__, "[net]没有处理器的消息类型%d", mn->msg_type_);
		}
	} else {
		if (mn->msg_data_ != NULL && iter->second->mp_ != NULL) {
			iter->second->mp_(mn);
		}
	}
}
} // end namespace net
