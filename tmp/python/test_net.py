#!/usr/bin/python
#--coding:utf-8--
import echo_pb2

import asyncore, socket
import threading
import struct
import time

#收到data最大長度
MAX_RECV = 4096

#連線server的socket
class client(asyncore.dispatcher):

	def __init__(self, host, port):
		asyncore.dispatcher.__init__(self)
		self.RecvData = ""
		
		self.echo_msg = echo_pb2.Echo.Req()
		self.echo_msg.code = 1
		self.echo_msg.str = "abcdefeg"
		self.echo_buf = self.echo_msg.SerializeToString();
		self.SendData = struct.pack("ii"+str(len(self.echo_buf))+"s", 1, len(self.echo_buf), self.echo_buf)
		print "----------------"
		print self.SendData
		#和server建立連線
		self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self.connect( (host, port) )

		

	def handle_connect(self):
		print 'connect!!'

	def handle_close(self):
		print "disconnection : " + self.getpeername()[0]
		self.close()

	#收到的data
	def handle_read(self):
		self.RecvData += self.recv(MAX_RECV)
		print "接收到的数据    : [" + str(len(self.RecvData)) + "]"
		print "接收到的数据内容: ----[" + self.RecvData + "]"
		if len(self.RecvData) >= self.echo_msg.ByteSize():
			print (self.RecvData)
			msg_type, msg_len, recv_buf = struct.unpack("ii"+str(len(self.echo_buf))+"s", self.RecvData)
			print msg_type
			print msg_len
			print recv_buf
			self.echo_msg.ParseFromString(recv_buf)
			print self.echo_msg.code
			print self.echo_msg.str
			self.echo_msg.code = self.echo_msg.code + 1	
			#self.echo_msg.str = "abcdefg"
			self.echo_buf = self.echo_msg.SerializeToString()
			self.SendData = struct.pack("ii"+str(len(self.echo_buf))+"s", 1, len(self.echo_buf), self.echo_buf)
			self.handle_write()
			self.RecvData = ""
			#sum_len = msg_len + 8
			#if len(self.RecvData) >= sum_len:
			#	buf = self.RecvData[:sum_len]
			#	self.RecvData = self.RecvData[sum_len:]
			#	buf = buf[8:]
			#	self.echo_msg.ParseFromString(buf)
			#	print "接收到数据:"
			#	print "code: " + str(self.echo_msg.code)
			#	print "str:  " + self.echo_msg.str

	#送出data
	def handle_write(self):
		print "Send data ..." + str(len(self.SendData))
		send_byte = 0
		if len(self.SendData) > 0:
			self_byte = self.send(self.SendData)
			print "发送了字节数: " + str(self_byte)
		if send_byte > 0:
			print "-------------------------------------"
			send_out = self.SendData[:send_byte]
			self.SendData = self.SendData[send_byte:]
			print "发送内容为    : " + send_out
			print "SendData长度为: " + str(len(self.SendData))
			self.handle_write()
			print "-------------------------------------"
		else:
			self.SendData = ""
			print "send all!!"
			#print "SendData长度为: " + str(len(self.SendData))
	#自動偵測送出永遠失敗
	def writable(self):
		return False
  
#等待server傳送訊息的thread
class send_server_thread(threading.Thread):
	def __init__(self,host,port):
		self.client = client(host, port)
		threading.Thread.__init__ ( self )
	def run(self):
		try:
			asyncore.loop()
		except:
			pass

class input_thread(threading.Thread):
	def __init__(self,client_thread):
		self.client_thread = client_thread
		threading.Thread.__init__ ( self )
	def run(self):
		while 1:
			self.client_thread.client.handle_write()
			time.sleep(2)

#主程式
if __name__ == "__main__":
	client_thread = send_server_thread("192.168.1.253", 4444)
	client_thread.start()
	#input_thread(client_thread).start()
	client_thread.client.handle_write()
