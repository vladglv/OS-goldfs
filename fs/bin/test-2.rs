extern crate fs;

extern "C" {
    fn difficult_test() -> i32;
}

fn main() {
    unsafe {
        difficult_test();
    }
}
