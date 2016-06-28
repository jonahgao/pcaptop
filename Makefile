
CXXFLAGS += -O2
LDFLAGS += -lpcap

./pcaptop:src/main.cc src/net_types.h
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@


clean:
	@rm ./pcaptop
