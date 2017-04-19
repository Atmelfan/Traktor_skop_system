#!/usr/bin/env python
import minimalmodbus
import argparse

class ArmSegment(object):
	"""docstring for ArmSegment"""
	def __init__(self, nid, length, port):
		super(ArmSegment, self).__init__()
		self.length = length
		self.id = nid
		self.instr = minimalmodbus.Instrument(port, nid)

	def get_angle(self):
		return self.instr.read_register(0,numberOfDecimals=2,signed=True, functioncode=4)*math.pi/180

	def __str__(self):
		return "Segment, %fm long, sensor id %d"%(self.length, self.nid)
		

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='.')
	parser.add_argument('-x', default='0', type=float, help='Offset X')
	parser.add_argument('-y', default='0', type=float, help='Offset Y')
	parser.add_argument('port', help='Port to connect to, t.ex "COM3" or "/dev/ttyUSB4"')
	parser.add_argument('seg', nargs='+', help='Arm segments, format as <id>:<length>')

	args=parser.parse_args()
	arm = []
	for segn in args.seg:
		seginfo = segn.split(':')
		if len(seginfo) == 2:
			arm.append(ArmSegment(int(seginfo[0]), float(seginfo[1]), args.port))
		else:
			print('Segments must be specified as <id>:<len>!')
			exit()
	print('Arm: ' + str(arm))
	while True:
		rawsens=[]
		h = args.y
		d = args.x
		for s in arm:
			angle = s.get_angle()
			rawsens.append(angle)
			h += s.length * math.sin(angle)
			d += s.length * math.cos(angle)
		print("%f %d %s")

	