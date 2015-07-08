#! /usr/bin/python
# -*- coding: UTF-8 -*-
from colorama import Fore, Back, Style

PRINT_COLOR=True
PRINT_FLUSH=False
def set_print_color(value):
	global PRINT_COLOR
	PRINT_COLOR=value
def set_print_flush(value):
	global PRINT_FLUSH
	PRINT_FLUSH=value

#Fore: BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET.
#Back: BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET.
#Style: DIM, NORMAL, BRIGHT, RESET_ALL
# ERROR print_red
# NOTICE print_back_green
#
#

def print_red(info):
	if PRINT_COLOR:
		print (Fore.RED + info + Fore.RESET + Back.RESET + Style.RESET_ALL)
	else:
		print (info)

	if PRINT_FLUSH:
		sys.stdout.flush()

def print_error(info):
	if PRINT_COLOR:
		print (Fore.RED + info + Fore.RESET + Back.RESET + Style.RESET_ALL)
	else:
		print (info)

	if PRINT_FLUSH:
		sys.stdout.flush()

def print_notice(info):
	if PRINT_COLOR:
		print (Style.DIM + Back.GREEN + Fore.BLUE+ info + Fore.RESET + Back.RESET + Style.RESET_ALL)
	else:
		print (info)

	if PRINT_FLUSH:
		sys.stdout.flush()

def print_normal(info):
	print (info)

	if PRINT_FLUSH:
		sys.stdout.flush()

if __name__ == "__main__":
	print_red("print_red")
	print_back_green("print_back_green")
