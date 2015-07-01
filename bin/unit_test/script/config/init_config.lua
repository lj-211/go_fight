function test()
	print("do lua file from c++")
end

test()

print (s_test_lua.server_id_)
print (s_test_lua.server_name_)

-- 设置属性
s_test_lua.server_id_ = 7890
s_test_lua.server_name_ = "战斗吧小宇宙"
print ("修改后:")
print (s_test_lua.server_id_)
print (s_test_lua.server_name_)

function return_to_cpp()
	return "这是从lua返回的值"
end

function test_parameter(tbl)
	print ("----------------------------")
	print (type(tbl))
	for k, v in pairs(tbl) do
		print (k)
		print (v)
	end
	print ("----------------------------")
	return 1
end

function test_error_function(a, b)
	return a + b
end

function test_return_str(a)
	return a .. "1234"
end
