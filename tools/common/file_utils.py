#! /usr/bin/python
# -*- coding: UTF-8 -*-
import os
import fnmatch
import com_utils
import urllib2

DEBUG_FILE_UTILS = False

# return file list
def get_file_with_word(dir, pattern):
	cwd = os.getcwd()
	if dir:
		os.chdir(dir)

	result = []

	for root, dis, files in os.walk(dir):
		for f in files:
			file_path = os.path.join(root, f)
			if fnmatch.fnmatch(f, pattern):
				result.append(file_path)

	os.chdir(cwd)
	return result

# return True or False
def downfile_to_path(url, path):
	try:
		f = urllib2.urlopen(url)
		data = f.read()
		ab_path = os.path.abspath(path)
		if os.path.exists(ab_path) == True:
			os.remove(ab_path)
		
		with open(ab_path, "wb") as code:     
			code.write(data)
	except Exception, e:
		com_utils.print_normal(str(e))
		return False

	return True

if __name__ == "__main__":
	com_utils.print_normal ("test function: " + "get_file_with_word")
	com_utils.print_normal ("result: " + str(get_file_with_word("/home/lj/dev/Server/bin/game", "cfg_limit_ip*")))

	com_utils.print_normal("test download file: " + "downfile_to_path")
	test_file = "./tmp_file"
	com_utils.print_normal("result: " + str(downfile_to_path("http://14.18.236.34/jjqyz/test_auto_update/auto_update_cfg.json", "./tmp_file")))
	if os.path.exists(test_file) == True:
		os.remove(test_file)	
