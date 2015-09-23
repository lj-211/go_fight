#!/usr/bin/python
#--coding:utf-8--
import echo_pb2

from socket import *
import threading
import struct
import time
import threading
import signal
import sys, getopt
import random

#收到data最大長度
MAX_RECV = 2048*20000*2
SUB_STR = "a"
STR = ""

#消息内容大小
MSG_SIZE = 1024*10240
#线程数
THREAD_NUM = 400
#测试次数
SEND_CNT = 100
#间隔时间
INTERVAL = 1
#显示单次连接
SHOW = False
#连接分组标记
GROUP = 0

ADDR = "183.61.109.173"
PORT = 6000

#总耗时
SUM_TIME = 0
sum_time_mutex= threading.Lock()

#最长耗时
BIG_TIME = 0
big_time_mutex= threading.Lock()

#不同延时区间
SUM_100 = 0
mutex_100 = threading.Lock()
SUM_200 = 0
mutex_200 = threading.Lock()
SUM_200 = 0
mutex_200 = threading.Lock()
SUM_300 = 0
mutex_300 = threading.Lock()
SUM_400 = 0
mutex_400 = threading.Lock()
SUM_500 = 0
mutex_500 = threading.Lock()
SUM_BAD = 0
mutex_bad = threading.Lock()

SUM_SEND_ALL = 0
mutex_send_all = threading.Lock()


is_exit = False

def timer(host, port):
    global SUM_TIME, sum_time_mutex
    global BIG_TIME, big_time_mutex
    global SUM_100, mutex_100
    global SUM_200, mutex_200
    global SUM_300, mutex_300
    global SUM_400, mutex_400
    global SUM_500, mutex_500
    global SUM_BAD, mutex_bad
    global SUM_SEND_ALL, mutex_send_all

    #和server建立連線
    addr = (host, port)
    client_sock = socket(AF_INET, SOCK_STREAM)
    client_sock.connect(addr)
    RecvData=""
    echo_req = echo_pb2.Echo.Req()
    echo_req.code = 1
    echo_req.str = STR

    echo_res = echo_pb2.Echo.Res()
    echo_res.code = 0
    echo_res.str = ""
    send_cnt = 0
    bad_cnt = 0
    time_sum = 0

    global is_exit 
    while (not is_exit) and (send_cnt < SEND_CNT):
        # 发送数据
        echo_buf = echo_req.SerializeToString()
        SendData = struct.pack("iiii"+str(len(echo_buf))+"s", 20003, client_sock.fileno() + GROUP, 0, len(echo_buf), echo_buf)
        #print "----------------"
        #print "将发送的数据    : [" + SendData + "]"

        #print "Send data ..." + str(len(SendData))
        send_byte = 0
        start_time = time.time()
        while len(SendData) > 0:
            self_byte = client_sock.send(SendData)
            #print "发送了字节数: " + str(self_byte)
            if send_byte > 0:
                #print "-------------------------------------"
                send_out = SendData[:send_byte]
                SendData = SendData[send_byte:]
                #print "发送内容为    : " + send_out
                #print "SendData长度为: " + str(len(SendData))
                #print "-------------------------------------"
            else:
                SendData = ""
                #print self_byte
                #print "send all!!"

        while len(RecvData) < echo_req.ByteSize():
            tmp = client_sock.recv(MAX_RECV)
            RecvData += tmp
            #print u"接收消息长度   : [" + str(len(tmp)) + "]"
        if len(RecvData) >= echo_req.ByteSize():
            msg_type, msg_cid, msg_cid2, msg_len, recv_buf = struct.unpack("iiii" + str(len(RecvData) - 16) + "s", RecvData)
            time_span = time.time() - start_time

            time_span = int(time_span*1000)
            time_sum = time_sum + time_span

            big_time_mutex.acquire()
            if time_span > BIG_TIME:
                BIG_TIME = time_span
            big_time_mutex.release()

            mutex_send_all.acquire()
            SUM_SEND_ALL = SUM_SEND_ALL + 1
            mutex_send_all.release()

            echo_res.ParseFromString(recv_buf)
            
            if (time_span > 0 and time_span <= 100):
            #    print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(time_span)+"ms" 
                mutex_100.acquire()
                SUM_100 = SUM_100 + 1
                mutex_100.release()
            elif (time_span > 100 and time_span <= 200):
           #     print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(time_span)+"ms" 
                mutex_200.acquire()
                SUM_200 = SUM_200 + 1
                mutex_200.release()
            elif (time_span > 200 and time_span <= 300):
           #     print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(time_span)+"ms" 
                mutex_300.acquire()
                SUM_300 = SUM_300 + 1
                mutex_300.release()
            elif (time_span > 300 and time_span <= 400):
           #     print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(time_span)+"ms" 
                mutex_400.acquire()
                SUM_400 = SUM_400 + 1
                mutex_400.release()
            elif (time_span > 400 and time_span <= 500):
           #     print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(time_span)+"ms" 
                mutex_500.acquire()
                SUM_500 = SUM_500 + 1
                mutex_500.release()
            elif (time_span > 500):
                print "连接id: " + str(client_sock.fileno()) + "单次消耗时间: " + str(float(time_span)/1000)+" s" + " code: " + str(echo_res.code) + time.strftime(" %Y-%m-%d %H:%M:%S",time.localtime(time.time()))
                mutex_bad.acquire()
                SUM_BAD = SUM_BAD + 1
                mutex_bad.release()
            #print u"接收消息类型   : [" + str(msg_type) + "]"
            #print u"接收消息长度   : [" + str(msg_len) + "]"
            #print u"接收消息内容   : [" + recv_buf + "]"
            #print u"接收code       : [" + str(echo_res.code) + "]"
            #print u"接收str        : [" + str(echo_res.str) + "]"
            if (echo_res.str != echo_req.str or echo_res.code != echo_req.code):
                print "校验失败"
                break
            echo_req.code = echo_res.code + 1
            echo_req.str = STR + str(echo_res.code+1)
            RecvData=""

        send_cnt = send_cnt + 1
        time.sleep(INTERVAL)

    #print u"连接关闭:" + str(client_sock.fileno()) + u" 平均消耗时间: " + str((int)(time_sum/send_cnt)) + "ms"
    if sum_time_mutex.acquire():
        SUM_TIME += time_sum/send_cnt
        sum_time_mutex.release()

def handler(signum, frame):
    global is_exit
    is_exit = True

class MyThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        timer(ADDR, PORT)

def print_result():
    print "并发连接数: " + str(THREAD_NUM)
    print "数据包大小: " + str(MSG_SIZE) + "Byte"
    print "发包个数: " + str(SEND_CNT)
    print "发包频率: " + str(INTERVAL) + "s"

    sum_time_mutex.acquire()
    print "总平均耗时:" + str(int(SUM_TIME/THREAD_NUM)) + "ms"
    sum_time_mutex.release()

    big_time_mutex.acquire()
    print "最长耗时:" + str(BIG_TIME) + " ms"
    big_time_mutex.release()

    print "耗时分布:"
    mutex_send_all.acquire()
    mutex_100.acquire()
    print "0   ~ 100 ms: " + (format(float(SUM_100)/SUM_SEND_ALL, '.2%'))
    mutex_100.release()

    mutex_200.acquire()
    print "100 ~ 200 ms: " + (format(float(SUM_200)/SUM_SEND_ALL, '.2%'))
    mutex_200.release()

    mutex_300.acquire()
    print "200 ~ 300 ms: " + (format(float(SUM_300)/SUM_SEND_ALL, '.2%'))
    mutex_300.release()

    mutex_400.acquire()
    print "300 ~ 400 ms: " + (format(float(SUM_400)/SUM_SEND_ALL, '.2%'))
    mutex_400.release()

    mutex_500.acquire()
    print "400 ~ 500 ms: " + (format(float(SUM_500)/SUM_SEND_ALL, '.2%'))
    mutex_500.release()

    mutex_bad.acquire()
    print "500 ~ ~~~ ms: " + (format(float(SUM_BAD)/SUM_SEND_ALL, '.2%'))
    mutex_bad.release()



if __name__ == "__main__":
    opts, args = getopt.getopt(sys.argv[1:], "n:c:a:p:i:sg:m:")
    for op, value in opts:
        if op == "-n":
            THREAD_NUM = int(value)
        if op == "-c":
            SEND_CNT = int(value)
        if op == "-a":
            ADDR = value
        if op == "-p":
            PORT = int(value)
        if op == "-i":
            INTERVAL = float(value)
        if op == "-s":
            SHOW = True
        if op == "-g":
            GROUP = int(value)
        if op == "-m":
            MSG_SIZE = int(value)

    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)
    threads = []
    for j in range(0, MSG_SIZE):
        STR += SUB_STR
    for i in range(0, THREAD_NUM):
        my_thread = MyThread()
        my_thread.setDaemon(True)
        threads.append(my_thread)
        my_thread.start()
    while 1:
        alive = False
        for i in range(0, THREAD_NUM):
            alive = alive or threads[i].isAlive()
        if not alive:
            break
    print_result()

