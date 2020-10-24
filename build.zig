const Builder = @import("std").build.Builder;

pub fn build(b: *Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    // disk_emu
    const libdu = b.addStaticLibrary("du", null);
    libdu.setTarget(target);
    libdu.setBuildMode(mode);
    libdu.addCSourceFile("src/disk_emu.c", &[_][]const u8{"-std=c11"});
    libdu.linkSystemLibrary("c");

    // sfs
    const libsfs = b.addStaticLibrary("sfs", null);
    libsfs.setTarget(target);
    libsfs.setBuildMode(mode);
    libsfs.addCSourceFile("src/sfs_api.c", &[_][]const u8{"-std=c11"});
    libsfs.linkSystemLibrary("c");

    // test
    const libtest = b.addStaticLibrary("test", null);
    libtest.setTarget(target);
    libtest.setBuildMode(mode);
    libtest.addCSourceFile("src/tests.c", &[_][]const u8{"-std=c11"});
    libtest.linkSystemLibrary("c");

    // test1
    const test1 = b.addExecutable("test1", null);
    test1.setTarget(target);
    test1.setBuildMode(mode);
    test1.addCSourceFile("src/sfs_test1.c", &[_][]const u8{"-std=c11"});
    test1.linkLibrary(libdu);
    test1.linkLibrary(libsfs);
    test1.linkLibrary(libtest);
    test1.linkSystemLibrary("c");
    test1.install();

    // test2
    const test2 = b.addExecutable("test2", null);
    test2.setTarget(target);
    test2.setBuildMode(mode);
    test2.addCSourceFile("src/sfs_test2.c", &[_][]const u8{"-std=c11"});
    test2.linkLibrary(libdu);
    test2.linkLibrary(libsfs);
    test2.linkLibrary(libtest);
    test2.linkSystemLibrary("c");
    test2.install();
}
