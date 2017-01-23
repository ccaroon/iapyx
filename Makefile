iapyx.bin: main.ino lib/*
	mkdir dist
	cp project.properties main.ino lib/* dist/
	particle compile photon dist/ --saveTo iapyx.bin
	rm -rf dist/

flash: iapyx.bin
	particle flash Iapyx iapyx.bin

clean:
	rm -rf iapyx.bin dist/
