all:
	mb-as -a=crt-bram.lst -o crt-bram.o crt-bram.s
	mb-as -a=crt-sram.lst -o crt-sram.o crt-sram.s
	mb-as -a=crt-sram-with-int.lst -o crt-sram-with-int.o crt-sram-with-int.s

clean:
	-rm *.o
	-rm *.out
	-rm *.bin
	-rm *.v
	-rm *.map
	-rm *.lst
	-rm *.bak
	-rm *.srec
	-rm *.prom
	-rm *.rom
