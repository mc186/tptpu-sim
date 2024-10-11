# Makefile to take care of both Makefile.tptpu1 and Makefile.tptpu2
all: tptpu1 tptpu2 new
#TPTPU1 version 1
tptpu1:
	make tptpu1 -f Makefile.tptpu1

sim1:
	make tptpu1-sim -f Makefile.tptpu1

mmu1:
	make tptpu1-mmutest -f Makefile.tptpu1

tile1:
	make tptpu1-tiletest -f Makefile.tptpu1

# TPTPU version 2
tptpu2:
	make tptpu2 -f Makefile.tptpu2

sim2:
	make tptpu2-sim -f Makefile.tptpu2



# New arch
new:
	make new -f Makefile.new

sim_new:
	make new-sim -f Makefile.new

clean:
	-make clean -f Makefile.tptpu1
	-make clean -f Makefile.tptpu2
	-make clean -f Makefile.new