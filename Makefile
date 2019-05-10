CC=gcc
CFLAGES= -Wall -O2  
OBJSDIR=obj
TARGET=camera
OBJS=camera.o
SRC=$(OBJS:o=c)
RM=rm -f

$(TARGET):$(OBJSDIR) $(OBJS)
	$(CC) $(CFLAGES) -L ./jpeg-ubuntu/lib -o $(TARGET) $(OBJSDIR)/*.o -ljpeg 

$(OBJS):$(SRC)
	$(CC) $(CFLAGES) -c $(SRC) -o $(OBJSDIR)/$@ 

clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJSDIR)/*.o
