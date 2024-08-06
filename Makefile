
CC=g++
MPICC=mpic++

TARGET=sendrecv

SRCS=$(filter-out $(TARGET).cpp,$(wildcard *.cpp))
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

$(TARGET): $(TARGET).cpp $(OBJS)
	$(MPICC) -o $@ $^ -DMYLOG

%.o:%.cpp
	$(CC) -DMYLOG -o $@ -c $<

clean:
	rm -f *.o $(TARGET)
