CXXFLAGS+= -std=c++11 -O2 -g -Wall -Wextra
CXX=clang++
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -c

my_test:my_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f my_test my_test.o
