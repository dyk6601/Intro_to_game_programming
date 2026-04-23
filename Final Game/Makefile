CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I.
TARGET   := game

SOURCES := \
	main.cpp \
	CS3113/cs3113.cpp \
	CS3113/ShaderProgram.cpp \
	CS3113/Effects.cpp \
	CS3113/Entity.cpp \
	CS3113/LevelA.cpp \
	CS3113/SkeletonEnemy.cpp \
	CS3113/VampireEnemy.cpp \
	CS3113/GorgonEnemy.cpp \
	CS3113/LevelB.cpp \
	CS3113/LevelC.cpp \
	CS3113/LevelD.cpp \
	CS3113/LevelCToDScene.cpp \
	CS3113/MainMenuScene.cpp \
	CS3113/OpeningScene.cpp \
	CS3113/LoseScene.cpp \
	CS3113/WinScene.cpp \
	CS3113/Map.cpp \
	CS3113/Scene.cpp

HEADERS := $(shell find CS3113 -name '*.h')

UNAME_S := $(shell uname -s)

# Prefer pkg-config when available.
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_LIBS)),)
	ifeq ($(UNAME_S),Darwin)
		RAYLIB_LIBS := -L/opt/homebrew/lib -lraylib -framework IOKit -framework Cocoa -framework OpenGL -framework CoreVideo
	else
		RAYLIB_LIBS := -lraylib -lm -lpthread -ldl -lrt -lX11
	endif
endif

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) $(SOURCES) -o $@ $(RAYLIB_LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
