LIMINE_DIR  ?= ./limine
SPLEEN_URL  ?= https://github.com/fcambus/spleen/releases/download/2.2.0/spleen-2.2.0.tar.gz
SPLEEN_TAR  ?= spleen-2.2.0.tar.gz
SPLEEN_DIR  ?= spleen

limine:
	@if [ ! -d "$(LIMINE_DIR)" ]; then \
		git clone --branch=v10.6.6-binary --depth=1 \
			https://github.com/limine-bootloader/limine.git $(LIMINE_DIR); \
	fi

spleen:
	mkdir -p $(SPLEEN_DIR)
	@if command -v curl >/dev/null 2>&1; then \
		curl -L -o $(SPLEEN_TAR) "$(SPLEEN_URL)"; \
	elif command -v wget >/dev/null 2>&1; then \
		wget -O $(SPLEEN_TAR) "$(SPLEEN_URL)"; \
	else \
		echo "ERROR: neither curl nor wget found"; exit 1; \
	fi
	tar -xzf $(SPLEEN_TAR) --strip-components=1 \
		-C $(SPLEEN_DIR) --wildcards '*.psfu' || true
	rm -f $(SPLEEN_TAR)