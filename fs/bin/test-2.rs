extern crate fs;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

fn main() {
    unsafe {
        difficult_test();
    }
}
