CC ?= gcc
DESTDIR ?= /

BIN=tdx-cfgblock

$(BIN): tdx-cfgblock.c tdx-cfgdata.h
	@$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean install

clean:
	rm -f $(BIN)

install:
	mkdir -p $(DESTDIR)/usr/sbin
	install -m 0755 $(BIN) $(DESTDIR)/usr/sbin
