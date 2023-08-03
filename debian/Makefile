include ../Makefile.defs

DDEB=dpkg-deb
MD=mkdir -p
CP=cp
RM=rm -rf
EH=echo

LNAME=lib$(NAME)
DDIR=$(LNAME)_$(VER)_$(LARCH)
DEB=$(DDIR).deb
CNT=$(DDIR)/DEBIAN/control

build: ../$(LNAME).so ../vulcmp.h $(CNT)
	$(MD) $(DDIR)/usr/lib
	$(CP) ../libvulcmp.so $(DDIR)/usr/lib/
	$(MD) $(DDIR)/usr/include
	$(CP) ../vulcmp.h $(DDIR)/usr/include/
	$(DDEB) --build --root-owner-group $(DDIR)

clean:
	$(RM) $(DDIR) $(DDIR).deb

$(CNT): Makefile
	$(MD) $(DDIR)/DEBIAN/
	$(EH) 'Package: $(LNAME)' > $(CNT)
	$(EH) 'Version: $(VER)' >> $(CNT)
	$(EH) 'Architecture: $(LARCH)' >> $(CNT)
	$(EH) 'Maintainer: $(MNAME) <$(MEMAIL)>' >> $(CNT)
	$(EH) 'Depends: libvulkan1' >> $(CNT)
	$(EH) 'Description: $(DESC)' >> $(CNT)
