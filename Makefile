#
# Makefile
#

all: app_nanos.tgz app_nanosp.tgz app_nanox.tgz
debug: app_nanos_dbg.tgz app_nanosp_dbg.tgz app_nanox_dbg.tgz

.PHONY: clean all debug integration_tests unit_tests \
	integration_tests_basic integration_tests_basic_%

DOCKER			= docker
DOCKER_RUN		= $(DOCKER) run --rm -i -v "$(realpath .):/app"
DOCKER_RUN_APP_BUILDER	= $(DOCKER_RUN) ledger-app-builder:latest

#
# Fetch the docker images:

LEDGERHQ=ghcr.io/ledgerhq

docker_speculos:
	$(DOCKER) pull $(LEDGERHQ)/speculos
	$(DOCKER) image tag $(LEDGERHQ)/speculos speculos

docker_ledger_app_builder:
	$(DOCKER) pull $(LEDGERHQ)/ledger-app-builder/ledger-app-builder:latest
	$(DOCKER) image tag $(LEDGERHQ)/ledger-app-builder/ledger-app-builder \
			ledger-app-builder

docker_images:	docker_speculos			\
		docker_ledger_app_builder

run-test: nanos.out
	grep AFTER  nanos.out | sed s/.*AFTER\]//  > /tmp/aftah
	grep BEFORE nanos.out | sed s/.*BEFORE\]// > /tmp/b4
	diff -u /tmp/b4 /tmp/aftah

CMD = /speculos/speculos.py --display headless /app/app/bin/app.elf
nanos.out: app_nanos.tgz
	> nanos.out 2>&1 \
	$(DOCKER_RUN) --entrypoint=/bin/sh speculos -c "$(CMD)" || true

app_%.tgz:	app/src/*.[ch] app/Makefile
	$(DOCKER_RUN_APP_BUILDER) bash -c \
          'BOLOS_SDK=$$$(shell echo $(patsubst app_%.tgz,%,$@) | tr '[:lower:]' '[:upper:]')_SDK make -C app DEBUG=true'
	$(DOCKER_RUN_APP_BUILDER) bash -c "cd app/bin/ && tar cz ." > $@

clean:
	rm -rf bin app_*.tgz
	rm test.out
