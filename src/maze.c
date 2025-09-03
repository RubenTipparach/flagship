#include "maze.h"
#include "raylib.h"
#include <stdio.h>

// Function to load maze from ASCII file
Maze LoadMazeFromFile(const char* filename) {
    Maze maze = {0};
    FILE* file = fopen(filename, "r");
    
    if (file == NULL) {
        TraceLog(LOG_ERROR, "Failed to open maze file: %s", filename);
        return maze;
    }
    
    char line[MAX_MAZE_WIDTH + 2]; // +2 for newline and null terminator
    int row = 0;
    
    while (fgets(line, sizeof(line), file) && row < MAX_MAZE_HEIGHT) {
        int col = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n' && col < MAX_MAZE_WIDTH; i++) {
            maze.data[row][col] = line[i];
            col++;
        }
        
        if (col > maze.width) maze.width = col;
        row++;
    }
    
    maze.height = row;
    fclose(file);
    
    TraceLog(LOG_INFO, "Loaded maze: %dx%d", maze.width, maze.height);
    return maze;
}