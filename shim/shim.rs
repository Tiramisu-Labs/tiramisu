// shim_rust.rs
#![no_std]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_: &PanicInfo) -> ! { loop {} }

extern crate core;

static mut HEAP_PTR: i32 = 1024;

#[no_mangle]
pub extern "C" fn alloc(size: i32) -> i32 {
    unsafe {
        let cur = HEAP_PTR;
        HEAP_PTR += (size + 7) & !7;
        cur
    }
}

#[no_mangle]
pub extern "C" fn handle_request(req_ptr: i32, req_len: i32) -> i64 {
    let req = unsafe { core::slice::from_raw_parts(req_ptr as *const u8, req_len as usize) };
    let req_str = core::str::from_utf8_unchecked(req);
    let resp = handler(req_str); // normal Rust call, no extern
    let bytes = resp.as_bytes();
    let ptr = alloc(bytes.len() as i32);
    unsafe {
        core::ptr::copy_nonoverlapping(bytes.as_ptr(), ptr as *mut u8, bytes.len());
    }
    ((ptr as i64) << 32) | bytes.len() as i64
}

// The user will define this in their Rust file.
extern "Rust" {
    fn handler(req: &str) -> String;
}
