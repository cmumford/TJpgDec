
.PHONY: format
format:
	clang-format -i test/simple/*.c

.PHONY: test
test:
	build/simple test/images
