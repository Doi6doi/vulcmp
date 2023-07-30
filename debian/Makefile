include ../Makefile.defs

DDEB=dpkg-deb
MD=mkdir -p
CP=cp
RM=rm -rf
EH=echo

DDIR=$(NAME)_$(VER)_$(LARCH)
DEB=$(DDIR).deb
CNT=$(DDIR)/DEBIAN/control

build: ../lib$(NAME).so ../vulcmp.h $(CNT)
	$(MD) $(DDIR)/usr/lib
	$(CP) ../libvulcmp.so $(DDIR)/usr/lib/
	$(MD) $(DDIR)/usr/include
	$(CP) ../vulcmp.h $(DDIR)/usr/include/
	$(DDEB) --build --root-owner-group $(DDIR)

clean:
	$(RM) $(DDIR) $(DDIR).deb

$(CNT): Makefile
	$(MD) $(DDIR)/DEBIAN/
	$(EH) 'Package: $(NAME)' > $(CNT)
	$(EH) 'Version: $(VER)' >> $(CNT)
	$(EH) 'Architecture: $(LARCH)' >> $(CNT)
	$(EH) 'Maintainer: $(MNAME) <$(MEMAIL)>' >> $(CNT)
	$(EH) 'Depends: libvulkan1' >> $(CNT)
	$(EH) 'Description: $(DESC)' >> $(CNT)
