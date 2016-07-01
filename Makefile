
CXXFLAGS += -O2 -g -Wall
LDFLAGS += -lpcap -lncurses -lpthread -lrt

SRC=$(shell echo src/*.cc)
OBJ=$(SRC:%.cc=obj/%.o)
DEPENDS=$(OBJ:%.o=%.d)
OUTPUT=./pcaptop

$(OUTPUT):$(OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

obj/%.o:%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@ 

-include $(OBJ:%.o=%.d)

.PHONY:clean
clean:
	@rm -rf $(OUTPUT) $(OBJ) $(DEPENDS)
