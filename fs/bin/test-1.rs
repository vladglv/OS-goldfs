extern crate fs;

extern "C" {
    fn simple_test() -> i32;
}

fn main() {
    unsafe {
        simple_test();
    }
}
