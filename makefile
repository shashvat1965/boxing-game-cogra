CXX = g++
CXXFLAGS = -std=c++11
LDFLAGS = -lGL -lGLU -lglut -lassimp -lm -lpng
SOURCES = main.cpp modelBoxer.cpp customBoxer.cpp globals.cpp helpers.cpp
TARGET = boxing-game-cogra

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)