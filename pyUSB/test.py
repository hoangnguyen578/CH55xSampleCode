import usb.core
import usb.util

# find our device
dev = usb.core.find(idVendor=0x1234, idProduct=0x5678)

# was it found?
if dev is None:
    raise ValueError('Device not found')

# set the active configuration. With no arguments, the first
# configuration will be the active one
dev.set_configuration()

# get an endpoint instance
cfg = dev.get_active_configuration()
intf = cfg[(0,0)]

data = b"\x03"

dev.write(0x01, data)

data = dev.read(0x82,64)

print(hex(data[0]), hex(data[1]), hex(data[2]), hex(data[3]), hex(data[4]), hex(data[5]), hex(data[6]), hex(data[7]))
