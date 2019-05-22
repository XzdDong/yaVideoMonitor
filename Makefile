CC=gcc
CFLAGES= -Wall -O2  
OBJDIR=obj
TARGET=camera
#SRC = $(wildcard *.c)
#OBJ = $(patsubst %.c,${OBJDIR}/%.o,$(notdir ${SRC})) 
SRC = v4l2_ctr.c bmp.c jpeg.c camera.c httpd.c
OBJ = $(SRC:.c=.o) 

RM=rm -f
JPEGLIB=./jpeg-ubuntu/lib
$(info SRC:$(SRC))
$(TARGET):$(OBJ)
	$(CC) $(CFLAGES) -L $(JPEGLIB)  -o $(TARGET) $(OBJ) -ljpeg -lpthread

%.o:%.cpp
	$(CC) $(CFLAGES) -c $< -o $@ 

clean:
	-$(RM) $(TARGET)
	-$(RM) *.jpg
	-$(RM) $(OBJDIR)/*.o
