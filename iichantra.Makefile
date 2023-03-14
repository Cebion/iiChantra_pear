# Файл сборки самой иичантры

CXX = g++
LD = $(CXX)
RM = rm -f
CFLAGS = 
CXXFLAGS = $(CFLAGS)
LDFLAGS = -Llib
DFLAGS = -MD
INCLUDES = -Isrc
# TODO: Найти, может ли g++ как то сам создавать папки для объектных файлов
OFOLDER = obj obj/game obj/game/objects obj/game/phys obj/game/phys/sap obj/gui obj/render obj/script obj/sound
# Список объектных файлов, по ним определяется, какие *.cpp-файлы крмпилировать
OFILES = 	obj/StdAfx.o \
			obj/cfile.o \
			obj/config.o \
			obj/crc32.o \
			obj/input_mgr.o \
			obj/main.o \
			obj/misc.o \
			obj/resource_mgr.o \
			obj/scene.o \
			obj/game/animation.o \
			obj/game/camera.o \
			obj/game/game.o \
			obj/game/highscores.o \
			obj/game/http_client.o \
			obj/game/editor.o \
			obj/game/net.o \
			obj/game/object_manager.o \
			obj/game/particle_system.o \
			obj/game/player.o \
			obj/game/proto.o \
			obj/game/ribbon.o \
			obj/game/sprite.o \
			obj/game/objects/object.o \
			obj/game/objects/object_bullet.o \
			obj/game/objects/object_character.o \
			obj/game/objects/object_dynamic.o \
			obj/game/objects/object_effect.o \
			obj/game/objects/object_enemy.o \
			obj/game/objects/object_environment.o \
			obj/game/objects/object_item.o \
			obj/game/objects/object_particle_system.o \
			obj/game/objects/object_player.o \
			obj/game/objects/object_ray.o \
			obj/game/objects/object_ribbon.o \
			obj/game/objects/object_spawner.o \
			obj/game/objects/object_sprite.o \
			obj/game/objects/object_waypoint.o \
			obj/game/objects/weapon.o \
			obj/game/phys/phys_collisionsolver.o \
			obj/game/phys/sap/ASAP_PairManager.o \
			obj/game/phys/sap/IceAllocator.o \
			obj/game/phys/sap/IceBitArray.o \
			obj/game/phys/sap/IceContainer.o \
			obj/game/phys/sap/IceRevisitedRadix.o \
			obj/game/phys/sap/OPC_ArraySAP.o \
			obj/gui/gui.o \
			obj/gui/gui_button.o \
			obj/gui/gui_label.o \
			obj/gui/gui_picture.o \
			obj/gui/gui_textfield.o \
			obj/gui/gui_widget.o \
			obj/gui/text_typer.o \
			obj/render/draw.o \
			obj/render/font.o \
			obj/render/image.o \
			obj/render/renderer.o \
			obj/render/texture.o \
			obj/script/api.o \
			obj/script/gui_api.o \
			obj/script/luathread.o \
			obj/script/object_api.o \
			obj/script/script.o \
			obj/script/timerevent.o \
			obj/sound/snd.o
# Определение необходмиой версии библиотеки bass
BASSLIB = 
LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
	BASSLIB += -lbass_x64
else
	BASSLIB += -lbass
endif
# Используемые библиотеки
LDLIBS = -llua -lGL -lIL -lSDL -lstdc++ -lSDL_net
# Имя целевого исполняемого файла
TARGET = 
RELEASE_TARGET = bin/iiChantra.Release
DEBUG_TARGET = bin/iiChantra.Debug
EDITOR_TARGET = bin/iiChantra.Editor

# Правило сборки целевого исполняемого файла
link : $(OFILES)
	$(LD) -o $(TARGET) $^ $(LDFLAGS) $(LDLIBS)

# Правило сборки и подключения файлов зависимостей
-include $(OFILES:.o=.d)

# Правило сборки объектных файлов
obj/%.o : src/%.cpp
	$(CXX) -c $(DFLAGS) $(CXXFLAGS) $(INCLUDES) -o $@ $< 

# Правило создания папок под объектные файлы
obj:
	mkdir -p $(OFOLDER)

# Правило сборки Release-версии
release: obj
release: TARGET += $(RELEASE_TARGET)
release: CFLAGS += -O2 -Wall -DLINUX -DNDEBUG
release: LDLIBS += $(BASSLIB)
release: link

# Правило сборки Release-версии без звука
release_nosound: obj
release_nosound: TARGET += $(RELEASE_TARGET)
release_nosound: CFLAGS += -O2 -Wall -DLINUX -DNDEBUG -DNOSOUND_BASS
release_nosound: link

# Правило сборки Debug-версии
debug: obj
debug: TARGET += $(DEBUG_TARGET)
debug: CFLAGS += -Wall -g -O0 -D_DEBUG -DLINUX
debug: LDLIBS += $(BASSLIB)
debug: link

debug_nosound: obj
debug_nosound: TARGET += $(DEBUG_TARGET)
debug_nosound: CFLAGS += -Wall -g -O0 -D_DEBUG -DLINUX -DNOSOUND_BASS
debug_nosound: link


# Правило сборки редактора
editor: obj
editor: TARGET += $(EDITOR_TARGET)
editor: CFLAGS += -O2 -Wall -DLINUX -DNDEBUG -DMAP_EDITOR
editor: LDLIBS += $(BASSLIB)
editor: link

# Правило сборки редактора без звука
editor_nosound: obj
editor_nosound: TARGET += $(EDITOR_TARGET)
editor_nosound: CFLAGS += -O2 -Wall -DLINUX -DNDEBUG -DMAP_EDITOR -DNOSOUND_BASS
editor_nosound: link

valgrind: obj
valgrind: TARGET += $(DEBUG_TARGET)
valgrind: CFLAGS += -W -Wall -g -O -DNDEBUG -DLINUX -fno-inline -DNOSOUND_BASS
valgrind: link

#
clean:
	rm -f $(OBJECTS)
	#rm -f $(OFILES:.o=.d)
	rm -rf obj
	rm -f $(RELEASE_TARGET) $(DEBUG_TARGET)
	
.PHONY: clean debug release obj link
