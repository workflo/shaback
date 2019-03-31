build:
	docker build . -t flockerdub/shaback -t shaback

push:
	docker push flockerdub/shaback:latest

lastbuild-check: .lastbuild

.lastbuild: .git/refs/heads/release
	@cp -av $< $@

.PHONY:	build push
