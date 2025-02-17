explore: bataille.h bataille.cc explore.cc
	$(CXX) -std=c++20 -fstrict-aliasing -O3 -DNDEBUG -o explore bataille.cc explore.cc
