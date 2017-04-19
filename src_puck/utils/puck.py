import minimalmodbus
import time

class Puck(minimalmodbus.Instrument):
	"""docstring for Puck"""

	HOLDING_REGISTER_DEVICE_INFO = (0, 3)
	HOLDING_REGISTER_GYRO_CONF   = (1, 3)
	HOLDING_REGISTER_ACCEL_CONF  = (2, 3)
	HOLDING_REGISTER_ANGLE_CONF  = (3, 3)

	INPUT_REGISTER_GYRO_X  = (0, 4)
	INPUT_REGISTER_GYRO_Y  = (1, 4)
	INPUT_REGISTER_GYRO_Z  = (2, 4)
	INPUT_REGISTER_ACCEL_X = (3, 4)
	INPUT_REGISTER_ACCEL_Y = (4, 4)
	INPUT_REGISTER_ACCEL_Z = (5, 4)
	INPUT_REGISTER_ANGLE_X = (6, 4)
	INPUT_REGISTER_ANGLE_Y = (7, 4)
	INPUT_REGISTER_ANGLE_Z = (8, 4)

	device_info = 0x00 #Contains slave address and model number, read from register 0

	def __init__(self, portname, slaveaddress):
		minimalmodbus.Instrument.__init__(self, portname, slaveaddress)
		self.portname = portname
		self.device_info = self.read_register(HOLDING_REGISTER_DEVICE_INFO)
		self.write_register(HOLDING_REGISTER_ANGLE_CONF, 0x0FFF)

	def get_model(self):
		return (self.device_info & 0x00FF)

	def get_info(self):
		_id = reg0 >> 8#High byte = slave ID
		_model = reg0 & 0x00FF#Low byte = model number
		return 'ID=%i, model %i, (%s)'%(_id,_model,'0x%04X'%reg0)

	def get_register(self, register):
		return self.read_register(register[0], functioncode=register[1])

	def set_register(self, register, value):
		return self.read_register(register[0], value)

	def get_angle(self, angle):
		if get_model() == 205:
			angle[0] = angle[0]*2
			return 0#self.read_float(angle,functioncode=4)
		return self.read_register(angle[0], 2, functioncode=angle[1], signed=True)

	def update_firmware(self, file, newid):
		try:
			set_register(HOLDING_REGISTER_DEVICE_INFO, 0x00CC)#Writing 0x00CC(id=0) to the device info register will force a reboot to bootloader
			return False#The device isn't gonna respond and the above line will raise an exception, if not, return fail.
		except Exception, e:
			pass
		time.sleep(0.01)#Wait 10ms
		
		#


		


