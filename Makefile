iapyx.bin: main.ino lib/*/*.h lib/*/*.cpp
	particle compile photon . --saveTo iapyx.bin

flash: iapyx.bin
	particle flash Iapyx iapyx.bin

clean:
	rm -f iapyx.bin
