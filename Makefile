iapyx.bin: src/* project.properties lib/*
	mkdir build
	cp project.properties src/* lib/* build/
	particle compile photon build/ --saveTo iapyx.bin
	rm -rf build/

flash: iapyx.bin
	particle flash Iapyx iapyx.bin

clean:
	rm -rf iapyx.bin build/
