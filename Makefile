
.PHONY: format
format:
	clang-format -i test/simple/*.c test/fuzz/*.cc

.PHONY: test
test:
	build/simple test/images

.PHONY: run
run:
	build/Debug/tjpgd_vc.exe test/images

.PHONY: fuzzer
fuzzer:
	build/fuzzer
