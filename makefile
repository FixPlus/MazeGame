
windows:
	make -f makeWindows all

linux:
	make -f makeLinux all


clean:
	make -f makeWindows clean

release: windows
	make -f makeWindows release