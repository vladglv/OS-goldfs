fn main() {
    cc::Build::new()
        .include("../fs-c/include")
        .file("../fs-c/src/disk_emu.c")
        .file("../fs-c/src/sfs_api.c")
        .file("../fs-c/tests/tests.c")
        .file("../fs-c/tests/sfs_test1.c")
        .file("../fs-c/tests/sfs_test2.c")
        .compile("fs-c");
}
