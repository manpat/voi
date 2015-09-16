template<class C, class... A>
C* FramePool::New(A... a) {
	if(sizeof(C)+idx >= size) throw "FramePool buffer overflow";

	auto mem = new (&buffers[currentBuffer][idx]) C{a...};
	idx += sizeof(C);
	return mem;
}