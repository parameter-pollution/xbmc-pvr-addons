if ADDON_MYTHTV
          ADDITIONAL_SUBDIRS =
endif

SUBDIRS = pvr.a1tv \
          pvr.iptvsimple \
          $(ADDITIONAL_SUBDIRS)

clean:
	-rm -f *.zip
	for d in $(SUBDIRS); do if test -f $$d/Makefile; then $(MAKE) -C $$d clean; fi; done

zip:
	for d in $(SUBDIRS); do if test -f $$d/Makefile; then $(MAKE) -C $$d zip; fi; done

install:
	for d in $(SUBDIRS); do if test -f $$d/Makefile; then $(MAKE) -C $$d install; fi; done

