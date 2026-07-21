CXX      = g++
CXXFLAGS = -O3 -std=c++17 -Wall -pthread -DUNIFIED_BUILD
INCLUDES = -Isrc -Isrc/nnue -Isrc/nnue/features -Isrc/nnue/layers -Isrc/syzygy
TARGET   = engine
SRC      = veloct.cpp
NNUE_SRC = src/nnue/network.cpp src/nnue/nnue_accumulator.cpp src/nnue/nnue_misc.cpp \
           src/nnue/features/full_threats.cpp src/nnue/features/half_ka_v2_hm.cpp \
           src/nnue/features/pp_3wide.cpp
SYZYGY_SRC = src/syzygy/tbprobe.cpp
ALL_SRC  = $(SRC) $(NNUE_SRC) $(SYZYGY_SRC)
HDR      = src/engine.h src/types.h src/position.h src/search.h src/uci.h

all: $(TARGET)

$(TARGET): $(ALL_SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ALL_SRC) -o $(TARGET) -pthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
