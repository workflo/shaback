docker-image:
	docker build . -t shaback

lastbuild-check: .lastbuild

.lastbuild: .git/refs/heads/release
	@cp -av $< $@

.PHONY:	docker-image
