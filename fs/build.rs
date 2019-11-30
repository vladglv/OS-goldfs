extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    cc::Build::new()
        .include("../fs-c/include")
        .file("../fs-c/src/disk_emu.c")
        .file("../fs-c/src/sfs_api.c")
        .file("../fs-c/tests/tests.c")
        .file("../fs-c/tests/sfs_test1.c")
        .file("../fs-c/tests/sfs_test2.c")
        .compile("fs-c");

    println!("cargo:rerun-if-changed=../fs-c/tests/sfs_test1.h");
    println!("cargo:rerun-if-changed=../fs-c/tests/sfs_test2.h");

    let bindings = bindgen::Builder::default()
        .header("../fs-c/tests/sfs_test1.h")
        .header("../fs-c/tests/sfs_test2.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Could not write bindings");
}
