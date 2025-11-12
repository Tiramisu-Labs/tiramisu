package main

//export alloc
func alloc(size int32) int32 {
	// VERY simple stub: allocate from static area (not production)
	// For real usage, implement proper allocator or rely on TinyGo runtime
	return 1024
}

//export dealloc
func dealloc(ptr int32, size int32) {}

// user implements this in user.go:
// //export handler
// func handler(reqPtr int32, reqLen int32) int64

// entrypoint redirect
//
//export handle_request
func handle_request(reqPtr int32, reqLen int32) int64 {
	return handler(reqPtr, reqLen)
}

func main() {}
