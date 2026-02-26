CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2
CFLAGS += -D_POSIX_C_SOURCE=200809L
INCLUDES := -Iinclude
LDFLAGS := -Llib
LDLIBS := -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

BUILD_DIR := build
SRC := pixel-editor.c
TARGET := $(BUILD_DIR)/pixel
REPO ?= $(CURDIR)

TEST_SRC := tests/test_pixel-editor.c
TEST_TARGET := $(BUILD_DIR)/test_pixel-editor

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
DESKTOPDIR ?= $(DATADIR)/applications
ICONDIR ?= $(DATADIR)/pixmaps
ICON_SRC ?= docs/v0.2.png
APP_ID ?= pixel
APP_SHAREDIR ?= $(DATADIR)/pixel
FONTDIR ?= $(APP_SHAREDIR)/fonts
PALETTEDIR ?= $(APP_SHAREDIR)/palettes
FONTS := fonts/PressStart2P-Regular.ttf
PALETTES := palettes/*.txt

.PHONY: all run test install uninstall uninstall-all purge-user-data install-desktop uninstall-desktop clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p "$@"

$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	cd "$(REPO)" && ./$(TARGET)

$(TEST_TARGET): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) $(LDLIBS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: $(TARGET) install-desktop
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 "$(TARGET)" "$(DESTDIR)$(BINDIR)/pixel"

uninstall: uninstall-desktop
	rm -f "$(DESTDIR)$(BINDIR)/pixel"

uninstall-all: uninstall purge-user-data

purge-user-data:
	rm -rf "$${HOME}/.local/share/pixel"

install-desktop:
	install -d "$(DESTDIR)$(FONTDIR)"
	install -m 644 $(FONTS) "$(DESTDIR)$(FONTDIR)"
	install -d "$(DESTDIR)$(PALETTEDIR)"
	install -m 644 $(PALETTES) "$(DESTDIR)$(PALETTEDIR)"
	install -d "$(DESTDIR)$(ICONDIR)"
	install -m 644 "$(ICON_SRC)" "$(DESTDIR)$(ICONDIR)/$(APP_ID).png"
	install -d "$(DESTDIR)$(DESKTOPDIR)"
	printf '%s\n' \
		'[Desktop Entry]' \
		'Type=Application' \
		'Name=pixel' \
		'Comment=Local Pixel Editor' \
		'Exec=$(BINDIR)/pixel' \
		'Icon=$(ICONDIR)/$(APP_ID).png' \
		'Path=$(APP_SHAREDIR)' \
		'Terminal=false' \
		'Categories=Development;Utility;' \
	> "$(DESTDIR)$(DESKTOPDIR)/$(APP_ID).desktop"

uninstall-desktop:
	rm -f "$(DESTDIR)$(DESKTOPDIR)/$(APP_ID).desktop"
	rm -f "$(DESTDIR)$(ICONDIR)/$(APP_ID).png"
	rm -f "$(DESTDIR)$(ICONDIR)/$(APP_ID).jpg"
	rm -rf "$(DESTDIR)$(APP_SHAREDIR)"

clean:
	rm -f "$(TARGET)" "$(TEST_TARGET)"
