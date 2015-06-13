#!/usr/bin/python
#--coding:utf-8--
import echo_pb2

echo_msg = echo_pb2.Echo.Req()
echo_msg.code = 1
echo_msg.str = "1234"

encode_str = echo_msg.SerializeToString()

parse_echo = echo_pb2.Echo.Req()
parse_echo.ParseFromString(encode_str)

print "解析后的数据:"
print parse_echo.code
print parse_echo.str


