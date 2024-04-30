all: vcube

vcube: vcube.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic vcube.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c) -g smpl.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

vcube.o: vcube.c smpl.h
	$(COMPILE.c) -g vcube.c

clean:
	$(RM) *.o vcube
