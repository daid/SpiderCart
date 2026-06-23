import serial
import sys
import time

ser = serial.Serial(sys.argv[1], 115200)

def sendpacket(data: bytes):
    ser.write(bytes([0x5A, len(data)]))
    ser.write(data)
    time.sleep(0.010)
    res = ser.read(ser.in_waiting)
    print(res)
    return res


sendpacket(b'\x02')  # put GB in reset

f = open(sys.argv[2], 'rb')
addr = 0
while True:
    chunk = f.read(250)
    if not chunk:
        break
    sendpacket(bytes([0x10, addr & 0xFF, (addr >> 8) & 0xFF, (addr >> 16) & 0xFF]) + chunk)
    addr += len(chunk)
    print(addr)
f.close()

f = open(sys.argv[2], 'rb')
addr = 0
while True:
    chunk = f.read(255)
    if not chunk:
        break
    res = sendpacket(bytes([0x11, addr & 0xFF, (addr >> 8) & 0xFF, (addr >> 16) & 0xFF]))
    assert res[:len(chunk)] == chunk, chunk
    addr += len(chunk)
    print(addr)
f.close()

sendpacket(b'\x01')  # exit reset
