lastbuild-check: .lastbuild

.lastbuild: .git/refs/heads/release
	@cp -av $< $@

