all: release

valgrind: MAKETARGET = valgrind
valgrind: makework

debug: MAKETARGET = debug
debug: makework

release: MAKETARGET = release
release: makework

nosound: MAKETARGET = release_nosound
nosound: makework

debug_nosound: MAKETARGET = debug_nosound
debug_nosound: makework

editor: MAKETARGET = editor
editor: makework

editor_nosound: MAKETARGET = editor_nosound
editor_nosound: makework

makework : 
	$(MAKE) -f lua.Makefile release
	$(MAKE) -f iichantra.Makefile $(MAKETARGET)

clean:
	$(MAKE) -f lua.Makefile clean
	$(MAKE) -f iichantra.Makefile clean

.PHONY: clean all debug release
