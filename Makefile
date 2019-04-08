lastbuild-check: .lastbuild

build:
	docker build . -t flockerdub/shaback -t shaback

push:
	docker push flockerdub/shaback:latest

.lastbuild: .git/refs/heads/release
	@cp -av $< $@

.PHONY:	build push
