# Файл сборки lua

CXX = g++
CC = gcc
LD = $(CC)
RM = rm -f
CFLAGS = -DLUA_USE_LINUX
CXXFLAGS = $(CFLAGS)
LDFLAGS = 
DFLAGS = -MD
INCLUDES = -Isrc
# TODO: Найти, может ли g++ как то сам создавать папки для объектных файлов
OFOLDER = obj/script/lua
# Список объектных файлов, по ним определяется, какие *.cpp-файлы крмпилировать
OFILES = 	obj/script/lua/lapi.o \
			obj/script/lua/lauxlib.o \
			obj/script/lua/lbaselib.o \
			obj/script/lua/lcode.o \
			obj/script/lua/ldblib.o \
			obj/script/lua/ldebug.o \
			obj/script/lua/ldo.o \
			obj/script/lua/ldump.o \
			obj/script/lua/lfunc.o \
			obj/script/lua/lgc.o \
			obj/script/lua/linit.o \
			obj/script/lua/liolib.o \
			obj/script/lua/llex.o \
			obj/script/lua/lmathlib.o \
			obj/script/lua/lmem.o \
			obj/script/lua/loadlib.o \
			obj/script/lua/lobject.o \
			obj/script/lua/lopcodes.o \
			obj/script/lua/loslib.o \
			obj/script/lua/lparser.o \
			obj/script/lua/lstate.o \
			obj/script/lua/lstring.o \
			obj/script/lua/lstrlib.o \
			obj/script/lua/ltable.o \
			obj/script/lua/ltablib.o \
			obj/script/lua/ltm.o \
			obj/script/lua/lundump.o \
			obj/script/lua/lvm.o \
			obj/script/lua/lzio.o \
			obj/script/lua/print.o

# Используемые библиотеки
LDLIBS = -lstdc++
# Имя целевого исполняемого файла
TARGET = lib/liblua.a
#RELEASE_TARGET = lib/liblua.a
#DEBUG_TARGET = bin/liblua.a

# Правило сборки целевого файла библиотеки
link : $(OFILES)
	ar rcs $(TARGET) $^
	#$(LD) -o $(TARGET) $^ $(LDFLAGS) $(LDLIBS)

# Правило сборки и подключения файлов зависимостей
-include $(OFILES:.o=.d)

# Правило сборки объектных файлов
#obj/%.o : src/%.cpp
#	$(CXX) -c $(DFLAGS) $(CXXFLAGS) $(INCLUDES) -o $@ $< 
obj/%.o : src/%.c
	$(CC) -c $(DFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $< 

# Правило создания папок под объектные файлы
obj:
	mkdir -p $(OFOLDER)

# Правило сборки Release-версии
release: obj
#release: TARGET += $(RELEASE_TARGET)
release: CFLAGS +=  -DNDEBUG -D_LIB
release: link


#
clean:
	#rm -f $(OFILES:.o=.d)
	rm -rf $(OFOLDER)
	rm -f $(TARGET)
	
.PHONY: clean all release obj link
