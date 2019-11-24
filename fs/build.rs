fn main() {
    cc::Build::new()
        .file("../src/disk_emu.c")
        .file("../src/sfs_api.c")
        .file("../src/tests.c")
        .file("../src/sfs_test1.c")
        .file("../src/sfs_test2.c")
        .compile("fs-c");
}
