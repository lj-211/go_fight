#! /usr/bin/python
# -*- coding: UTF-8 -*-

def colorama_unit_test():
	from colorama import Fore, Back, Style
	print(Fore.RED + 'some red text')
	print(Back.GREEN + 'and with a green background')
	print(Style.DIM + 'and in dim text')
	print(Fore.RESET + Back.RESET + Style.RESET_ALL)
	print('back to normal now')

if __name__ == "__main__":
	colorama_unit_test()
