#![no_std]
extern crate libc;

use libc::{c_char, c_void, FILE};

#[no_mangle]
pub unsafe extern "C" fn vl_close_disk(file: *mut FILE) -> i64 {
    let mut r = -1;

    if !file.is_null() {
        r = libc::fclose(file);
        assert!(r == 0);
    }

    i64::from(r)
}

#[no_mangle]
pub unsafe extern "C" fn vl_init_fresh_disk(
    file: *mut *mut FILE,
    filename: *const c_char,
    size: i64,
    num: i64,
    block_size: *mut i64,
    block_num: *mut i64,
) -> i64 {
    if size <= 0
        || num <= 0
        || filename.is_null()
        || block_size.is_null()
        || block_num.is_null()
        || file.is_null()
    {
        return -1;
    }

    *block_size = size;
    *block_num = num;

    *file = libc::fopen(filename, "w+be".as_ptr() as *const _);
    if (*file).is_null() {
        return -1;
    }

    let buf = libc::calloc(size as usize, 1);
    if buf.is_null() {
        return -1;
    }

    for _i in 0..num {
        assert!(libc::fwrite(buf, size as usize, 1, *file) == 1);
    }

    libc::free(buf);

    0
}

#[no_mangle]
pub unsafe extern "C" fn vl_init_disk(
    file: *mut *mut FILE,
    filename: *const c_char,
    size: i64,
    num: i64,
    block_size: *mut i64,
    block_num: *mut i64,
) -> i64 {
    if size <= 0
        || num <= 0
        || filename.is_null()
        || block_size.is_null()
        || block_num.is_null()
        || file.is_null()
    {
        return -1;
    }

    *block_size = size;
    *block_num = num;

    *file = libc::fopen(filename, "r+be".as_ptr() as *const _);
    if (*file).is_null() {
        return -1;
    }

    0
}

#[no_mangle]
pub unsafe extern "C" fn vl_write_blocks(
    file: *mut FILE,
    begin: i64,
    num: i64,
    block_size: i64,
    block_num: i64,
    buf: *const c_void,
) -> i64 {
    if file.is_null() || begin < 0 || num <= 0 || buf.is_null() {
        return -1;
    }

    assert!(block_size > 0);
    assert!(block_num > 0);

    if begin + num > block_num {
        return -1;
    }

    let offset = begin * block_size;
    if libc::fseek(file, offset, libc::SEEK_SET) != 0 {
        return -1;
    }

    let block_buf = libc::malloc(block_size as usize);
    if block_buf.is_null() {
        return -1;
    }

    let mut s = 0;
    for i in 0..num {
        let ptr = buf as *const u8;
        let addr = ptr.offset((i * block_size) as isize);
        libc::memcpy(block_buf, addr as *const _, block_size as usize);
        assert!(libc::fwrite(block_buf, block_size as usize, 1, file) == 1);
        assert!(libc::fflush(file) == 0);

        s += 1;
    }

    libc::free(block_buf);

    s
}

#[no_mangle]
pub unsafe extern "C" fn vl_read_blocks(
    file: *mut FILE,
    begin: i64,
    num: i64,
    block_size: i64,
    block_num: i64,
    buf: *mut c_void,
) -> i64 {
    if file.is_null() || begin < 0 || num <= 0 || buf.is_null() {
        return -1;
    }

    assert!(block_size > 0);
    assert!(block_num > 0);

    if begin + num > block_num {
        return -1;
    }

    let offset = begin * block_size;
    if libc::fseek(file, offset, libc::SEEK_SET) != 0 {
        return -1;
    }

    let block_buf = libc::malloc(block_size as usize);
    if block_buf.is_null() {
        return -1;
    }

    let mut s = 0;
    for i in 0..num {
        assert!(libc::fread(block_buf, block_size as usize, 1, file) == 1);

        let ptr = buf as *mut u8;
        let addr = ptr.offset((i * block_size) as isize);
        libc::memcpy(addr as *mut _, block_buf, block_size as usize);

        s += 1;
    }

    libc::free(block_buf);

    s
}
