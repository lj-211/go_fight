#!/usr/bin/python
#--coding:utf-8--
import common.com_utils
import common.file_utils

import os
import sys
import getopt
import io

gen_dir = "../src/protocol/"
proto_dir = "../proto_file/"
cur_pwd = os.getcwd() + "/"
gen_msg_file_dirs = ["../src/gate/msg_process/", "../src/game/msg_process/"]

# 删除协议相关代码
def delete_proto_code(proto_name):
	files = common.file_utils.get_file_with_word(proto_dir, "*" + file_name + "*")

def gen_proto_code(file_name):
	common.com_utils.print_red("当前工作路径: " + os.getcwd())
	common.com_utils.print_red("协议目录: " + proto_dir)
	common.com_utils.print_red("生成路径: " + gen_dir)
	
	files = common.file_utils.get_file_with_word(proto_dir, file_name + ".proto")
	print "协议文件列表: "
	for f in files:
		common.com_utils.print_normal("生成协议文件: " + f)
		cmd = "./protoc --cpp_out=" + gen_dir + " -I=" + cur_pwd + proto_dir + " " + cur_pwd + f
		os.system(cmd)

	for f in files:
		file_name = os.path.split(f)
		real_name = os.path.splitext(file_name[1])[0]
		if "msg/" in f:
			common.com_utils.print_notice("文件[" + f  + "]属于消息类型,需要生成类型函数")
			cmd = "sed /" + real_name + "/d " + gen_dir + "protocol.h > protocol.h"
			os.system(cmd)
			cmd = "mv -f ./protocol.h " + gen_dir + "protocol.h"
			os.system(cmd)

			cmd = "sed /\"return message_" + real_name + "\"/d " + gen_dir + "protocol.cpp > protocol.cpp"
			print cmd
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)

			cmd = "sed -i '/add get_type/itemplate<> int get_type<" + real_name  + "::Req>();' " + gen_dir + "protocol.h"
			os.system(cmd)

			cmd = "sed -i '/add_get_type/itemplate<> int get_type<" + real_name  + "::Req>() { return message_" + real_name + "_Req_; }' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			cmd = "sed -i '/add get_type/itemplate<> int get_type<" + real_name  + "::Res>();' " + gen_dir + "protocol.h"
			os.system(cmd)

			cmd = "sed -i '/add_get_type/itemplate<> int get_type<" + real_name  + "::Res>() { return message_" + real_name + "_Res_; }' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			# cpp
			cmd = "sed /" + real_name + ".pb.h/d " + gen_dir + "protocol.h > protocol.h"
			os.system(cmd)
			cmd = "mv -f ./protocol.h " + gen_dir + "protocol.h"
			os.system(cmd)
			cmd = "sed -i '/include protocol/i#include \"msg/" + real_name  +".pb.h\"' " + gen_dir + "protocol.h"
			os.system(cmd)

			fun_name_req = "new_" + real_name + "_req"
			fun_req = "net::MsgData* " + fun_name_req + "() { return GO_NEW(" + real_name  + "::Req);  }"
			cmd = "sed /" + fun_name_req + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/new func/i" + fun_req + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			delete_fun_name_req = "delete_" + real_name + "_req"
			delete_fun_req = "void " + delete_fun_name_req + "(net::MsgData* ptr) { GO_DELETE((" + real_name  +"_Req*)ptr, " + real_name  + "_Req); }"
			cmd = "sed /" + delete_fun_name_req + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/delete func/i" + delete_fun_req + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			process_req = "process_" + real_name + "_req"
			fun_process_req = "extern void process_" + real_name + "_req(net::MsgNode* msg);"
			cmd = "sed /" + process_req + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/process func/i" + fun_process_req + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			fun_name_res = "new_" + real_name + "_res"
			fun_res = "net::MsgData* " + fun_name_res + "() { return GO_NEW(" + real_name  + "::Res);  }"
			cmd = "sed /" + fun_name_res + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/new func/i" + fun_res + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			delete_fun_name_res = "delete_" + real_name + "_res"
			delete_fun_res = "void " + delete_fun_name_res + "(net::MsgData* ptr) { GO_DELETE((" + real_name  +"_Res*)ptr, " + real_name  + "_Res); }"
			cmd = "sed /" + delete_fun_name_res + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/delete func/i" + delete_fun_res + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			process_res = "process_" + real_name + "_res"
			fun_process_res = "extern void process_" + real_name + "_res(net::MsgNode* msg);"
			cmd = "sed /" + process_res + "/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/process func/i" + fun_process_res + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			regist_req = "net::regist_msg_processer(get_type<" + real_name  + "::Req>(), &" + fun_name_req + ", &" + delete_fun_name_req  + ",&" + process_req + ");" 
			cmd = "sed /\(get_type\<" + real_name + "::Req/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/regist msg/i" + regist_req + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)
			regist_res = "net::regist_msg_processer(get_type<" + real_name  + "::Res>(), &" + fun_name_res + ", &" + delete_fun_name_res + ",&"  + process_res + ");" 
			cmd = "sed /\(get_type\<" + real_name + "::Res/d " + gen_dir + "protocol.cpp > ./protocol.cpp"
			os.system(cmd)
			cmd = "mv -f ./protocol.cpp " + gen_dir + "protocol.cpp"
			os.system(cmd)
			cmd = "sed -i '/regist msg/i" + regist_res + "' " + gen_dir + "protocol.cpp"
			os.system(cmd)

			# 生成服务器处理函数
			file_content = []
			file_content.append("#include \"msg/" + real_name + ".pb.h\"\n\n")
			file_content.append("#include \"net/go_net.h\"\n")
			file_content.append("#include \"logger/logger.h\"\n\n")
			line = "void "
			line += process_req;
			line += "(net::MsgNode* msg) {"
			file_content.append(line)
			file_content.append("\n")
			line = "}"
			file_content.append(line)
			file_content.append("\n")
			line = "void "
			line += process_res;
			line += "(net::MsgNode* msg) {"
			file_content.append(line)
			file_content.append("\n")
			line = "}"
			file_content.append(line)
			file_content.append("\n")

			for d in gen_msg_file_dirs:
				file_path = d + real_name + ".cpp"
				if False:
					try:
						os.remove(file_path)
					except Exception,e:
						print "delete file"

				if os.path.exists(file_path):
					common.com_utils.print_normal("文件已经存在: " + file_path)
				else:
					common.com_utils.print_notice("创建文件: " + file_path)
					f = io.open(file_path, "wb+")
					f.writelines(file_content)
					f.flush()
					f.close()
		else:
			# def & config
			cmd = "sed /" + real_name + ".pb.h/d " + gen_dir + "protocol.h > protocol.h"
			os.system(cmd)
			cmd = "mv -f ./protocol.h " + gen_dir + "protocol.h"
			os.system(cmd)
			if "def/" in f:
				cmd = "sed -i '/include protocol/i#include \"def/" + real_name  +".pb.h\"' " + gen_dir + "protocol.h"
			if "cfg/" in f:
				cmd = "sed -i '/include protocol/i#include \"cfg/" + real_name  +".pb.h\"' " + gen_dir + "protocol.h"
			os.system(cmd)
	
	# 删除临时文件
	if os.path.exists("./protocol.h"):
		os.remove("./protocol.h")
	if os.path.exists("./protocol.h"):
		os.remove("./protocol.cpp")

	common.com_utils.print_normal("替换文件名: ")
	cc_files = common.file_utils.get_file_with_word("../src/", "*.cc")
	for f in cc_files:
		portion = os.path.splitext(f)
		new_name = portion[0] + ".cpp"
		common.com_utils.print_normal("替换: " + f)
		common.com_utils.print_normal("为: " + new_name)
		os.rename(f, new_name)

if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'f:a')
	except Exception, e:
		common.com_utils.print_error("argument error: " + str(e))
		sys.exit(0)

	gen_name = ""
	for o, a in opts:
		if o in ('-f'):
			gen_name = a
		if o in ('-a'):
			gen_name = "*"

	if gen_name == "":
		common.com_utils.print_error("没有正确的设置生成文件")
		sys.exit(0)

	gen_proto_code(gen_name)
