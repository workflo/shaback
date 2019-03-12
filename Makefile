docker-image:
	docker build . -t flockerdub/shaback -t shaback

push-docker-image:
	docker push flockerdub/shaback:latest

lastbuild-check: .lastbuild

.lastbuild: .git/refs/heads/release
	@cp -av $< $@

.PHONY:	docker-image push-docker-image
