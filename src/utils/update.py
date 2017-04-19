#!/usr/bin/env python
import minimalmodbus
import argparse


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='GPA Robotics modbus bootloader utility')
	parser.add_argument('port', help='Port to connect to, t.ex "COM3" or "/dev/ttyUSB4"')
	parser.add_argument('id', nargs='+', help='id of device(s) to perform action on')
	parser.add_argument('--newid', type=int, help='change the device id after a successfull action, will throw an error if multiple id\'s has been specified.')

	subparsers = parser.add_subparsers(title='actions',description='available actions',help='action to perform')

	parser_install = subparsers.add_parser('flash', help='flash firmware to device <id>')
	parser_install.add_argument('hex', help='Hex file containing the firmware')

	parser_info = subparsers.add_parser('info', help='read settings and information about device <id>')

	args=parser.parse_args()

	