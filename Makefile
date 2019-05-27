CC=gcc
CFLAGES= -Wall -Wno-comment -O2  
OBJDIR=obj
TARGET=camera
VPATH=./
#SRC = $(wildcard *.c)
#OBJ = $(patsubst %.c,${OBJDIR}/%.o,$(notdir ${SRC})) 
SRC = v4l2_ctr.c bmp.c jpeg.c camera.c httpd.c
OBJ = $(SRC:.c=.o) 

RM=rm -f
JPEGLIB=./jpeg-ubuntu/lib
$(info SRC:$(SRC))
$(info OBJ:$(OBJ))
$(TARGET):$(OBJ)
	$(CC) $(CFLAGES) -L $(JPEGLIB)  -o $(TARGET)  $(OBJDIR)/*.o -ljpeg -lpthread

$(OBJ):%.o:%.c
	$(CC) $(CFLAGES) -c $< -o $(OBJDIR)/$@ 

clean:
	-$(RM) $(TARGET)
	-$(RM) *.jpg
	-$(RM) $(OBJDIR)/*.o
	-$(RM) *.o

