TARGET=fsk
OBJS=./obj/cini.o ./obj/tools.o ./obj/axmscurses.o ./obj/XMS_String.o ./obj/XMS_Event.o \
			./obj/XMS_Sub.o ./obj/FSK_Sub.o ./obj/FSK_Data.o ./obj/WrLog.o ./obj/adapter.o ./obj/Connect.o
INC=../../inc
LIBPATH=../../dll

./obj/%.o:./src/%.cpp
	g++ -c -g -I$(INC) -I./include $< -o $@ 

$(TARGET):$(OBJS) 
	g++ $(OBJS) -L$(LIBPATH) -lDJITPCom -lDJAcsAPI -lcurses -lpthread -o $(TARGET)

clean:
	rm ./obj/*.o
	rm $(TARGET)
	
