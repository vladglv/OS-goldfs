build_dir=build
demu=disk_emu/Cargo.toml

all:
	mkdir -p $(build_dir)
	meson $(build_dir)
	cargo build --release --manifest-path $(demu) --target-dir $(build_dir)
	ninja -C $(build_dir)

clean:
	rm -rf $(build_dir)
