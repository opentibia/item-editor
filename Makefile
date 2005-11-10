# Makefile by Primer

CXX = g++
CXXFLAGS = -O2 -march=i486 -Wall
TARGET = otie
OBJS = main.o gui.o fileloader.o item_loader.o item_sprite.o item_type.o md5.o gui_linux.o
INCS = `pkg-config --cflags gtk+-2.0`
LIBS = `pkg-config --libs gtk+-2.0`

.PHONY: clean all install

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@
	@ echo ">> Compilation finished successfully."

# Common rules
main.o: client/main.cpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/main.cpp
	
gui.o : client/gui.cpp client/gui.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/gui.cpp
	
fileloader.o : client/fileloader.cpp client/fileloader.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/fileloader.cpp
	
item_loader.o: client/item_loader.cpp client/item_loader.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/item_loader.cpp

item_sprite.o: client/item_sprite.cpp client/item_sprite.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/item_sprite.cpp
	
item_type.o: client/item_type.cpp client/item_type.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c client/item_type.cpp
	
md5.o: client/md5.cpp client/md5.h
	$(CXX) $(CXXFLAGS) $(INCS) -c client/md5.cpp

# Linux rules
gui_linux.o: linux/gui_linux.cpp linux/gui_linux.hpp
	$(CXX) $(CXXFLAGS) $(INCS) -c linux/gui_linux.cpp

clean:
	rm *.o
	rm $(TARGET)
	
