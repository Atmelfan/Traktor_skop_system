#!/usr/bin/env python
import minimalmodbus
import argparse
import time
import threading
import textwrap

instruments = []
updaterate = 5
register = 8
nregisters = 1
tfunctioncode = 4

HOLDING_REGISTER_DEVICE_INFO = 0
HOLDING_REGISTER_ANGLE_CONFIG = 3

INPUT_ANGLE_X = 6
INPUT_ANGLE_Y = 7
INPUT_ANGLE_Z = 8

def device_info(reg0):
	_id = reg0 >> 8
	_model = reg0 & 0x00FF
	return 'ID=%i, model %i, (0x%04X)'%(_id,_model,reg0)

def print_values():
	thread = threading.Timer(1.0/updaterate, print_values)
	thread.daemon = True
	thread.start()

	values = []
	for instr in instruments:
		if nregisters == 1 and INPUT_ANGLE_X <= register <= INPUT_ANGLE_Z and tfunctioncode == 4:
			a = instr.read_register(register,2,functioncode=4,signed=True);
			values.append(a)
		elif nregisters == 1:
			a = instr.read_register(register,functioncode=tfunctioncode);
			values.append('0x%04X'%a)
		else:
			a = (instr.read_registers(register,nregisters,functioncode=tfunctioncode));
			a = ['0x%04X'%t for t in a]
			values.append(a)
	print('%s'%str(values))

def main(port, ids):
	for dev_id in ids:
		t = minimalmodbus.Instrument(port, dev_id)
		try:
			print("Device #%i info:  %s"%(dev_id, device_info(t.read_register(HOLDING_REGISTER_DEVICE_INFO))))
			t.write_register(HOLDING_REGISTER_ANGLE_CONFIG,0x0FFF)#Enable angle calculations
			instruments.append(t)
		except Exception, e:
			print("Error communicating with device #%i"%(dev_id))

	print_values()

	while True:
		pass
	

if __name__ == '__main__':
	parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
	description=textwrap.dedent('''.
Input registers:
	0\tGYRO_X
	1\tGYRO_Y
	2\tGYRO_Z
	3\tACCEL_X
	4\tACCEL_Y
	5\tACCEL_Z
	6\tANGLE_X
	7\tANGLE_Y
	8\tANGLE_Z
Holding registers(--holding):
	0\tDEVICE_INFO
	1\tGYRO_CONF
	2\tACCEL_CONF
	3\tANGLE_CONF'''))
	parser.add_argument('port', help='Port to connect to, e.g. "COM3" or "/dev/ttyUSB4"')
	parser.add_argument('id', nargs='+', type=int, help='IDs of sensors to read')
	parser.add_argument('--rate', default='5', type=int, help='Update rate, defaults to 5Hz')
	parser.add_argument('--register', default='8', type=int, help='Register to read, defaults to 8(ANGLE_Z)')
	parser.add_argument('--n', default='1', type=int, help='Number of registers to read starting from 8 or whatever --register specified, defaults to 1')
	parser.add_argument('--angles', action='store_true', help='Read all angle registers(6-8), overrides --register(=6) and --n(=3)')
	parser.add_argument('--holding', action='store_true', help='Read a holding register instead of a input register')


	args = parser.parse_args();
	updaterate = args.rate
	register = args.register

	if args.n > 0:
		nregisters = args.n
	elif args.n == 0:
		print('Yeah, not gonna tell the devices "Hey! Give me 0 registers!"... (--n=0)')
		exit()
	else:
		print('How am I supposed to read %i registers? (--n %i)'%(args.n,args.n) )
		exit()

	reg_type='input'
	if args.holding and not args.angles:
		tfunctioncode = 3
		reg_type='holding'

	if args.angles:
		register = 6
		nregisters = 3

	if nregisters > 1:
		print('Reading %s register %i to register %i!'%(reg_type, register, register+nregisters-1))
	else:
		print('Reading %s register %i!'%(reg_type, register))

	

	main(args.port, args.id)