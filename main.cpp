#include <iostream>
#include <windows.h>
#include <thread>

unsigned int width, height;
HANDLE console;

unsigned char** ReadBMP(const char *filename) {
    FILE* bitmap = fopen(filename, "rb");

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, bitmap); // read the 54-byte header

    // extract image height and width from header
    width = header[18];
    height = header[22];

    int row_padded = (width*3 + 3) & (~3);
    unsigned char** data = new unsigned char*[height];

    for(int i = height - 1; i >= 0; --i){
        data[i] = new unsigned char[row_padded];
        fread(data[i], sizeof(unsigned char), row_padded, bitmap);
    }

    fclose(bitmap);

    return data;
}

char** BMPDataToMazeMatrix(unsigned char **data) {
    char** maze = new char*[height];

    for(unsigned int i(0); i < height; ++i){
        maze[i] = new char[width];

        for(unsigned int j(0); j < width; ++j)
            maze[i][j] = ((int)data[i][3*j] == 255 &&
                          (int)data[i][3*j + 1] == 255 &&
                          (int)data[i][3*j + 2] == 255) ? 'f' : 'w';
    }

    return maze;
}

void DrawMaze(char **maze){
    for(unsigned int y = 0; y < height; ++y){
        for(unsigned int x = 0; x < width; ++x){
            if(maze[y][x] == 'l' || maze[y][x] == 'd' || maze[y][x] == 'r' || maze[y][x] == 'u' || maze[y][x] == 's'){
                SetConsoleTextAttribute(console, (0 << 4) + 12);
                if(maze[y][x] == 'l'){
                    if(maze[y-1][x] == 'd' || maze[y-1][x] == 's') putchar(188);
                    else if(maze[y][x+1] == 'l') putchar(205);
                    else putchar(187);
                }
                else if(maze[y][x] == 'd'){
                    if(maze[y-1][x] == 'd' || maze[y-1][x] == 's') putchar(186);
                    else if(maze[y][x-1] == 'r') putchar(187);
                    else putchar(201);
                }
                else if(maze[y][x] == 'r'){
                    if(maze[y-1][x] == 'd' || maze[y-1][x] == 's') putchar(200);
                    else if(maze[y][x-1] == 'r') putchar(205);
                    else putchar(201);
                }
                else if(maze[y][x] == 'u'){
                    if(maze[y+1][x] == 'u') putchar(186);
                    else if(maze[y][x+1] == 'l') putchar(200);
                    else putchar(188);
                } else {
                    putchar('o');
                }
                SetConsoleTextAttribute(console, (0 << 4) + 7);
            }else if(maze[y][x] == 'w') putchar(219);
            else if(maze[y][x] == 'v') putchar('.');
            else putchar(' ');
        }
        puts("");
    }
}

bool SolveMazeDFS(char **maze){
    unsigned int x, y = 1;
    for(x = 1; x < width-1; ++x){
        if(maze[0][x] == 'f'){
            maze[0][x] = 's';
            break;
        }
    }

    while(y != height-1 && y != 0){
        if(maze[y][x-1] == 'f') maze[y][x--] = 'l'; // go left
        else if(maze[y+1][x] == 'f') maze[y++][x] = 'd'; // go down
        else if(maze[y][x+1] == 'f') maze[y][x++] = 'r'; // go right
        else if(maze[y-1][x] == 'f') maze[y--][x] = 'u'; // go up
        else if(maze[y][x-1] == 'r') maze[y][x--] = 'v'; // backtrack left
        else if(maze[y+1][x] == 'u') maze[y++][x] = 'v'; // backtrack down
        else if(maze[y][x+1] == 'l') maze[y][x++] = 'v'; // backtrack right
        else if(maze[y-1][x] == 'd') maze[y--][x] = 'v'; // backtrack up
        else return false; // stuck
    }

    if(y == 0) return false; // No solution, returned to start
    maze[y][x] = 's';

    return true;
}

template <typename Tip>
void DeleteArray(unsigned int n, Tip **arr){
    for(unsigned int i=0; i<n; ++i) delete[] arr[i];
    delete[] arr;
}

int main(int argc, char **argv){
    if(argc != 2) return 1;
    console = GetStdHandle(STD_OUTPUT_HANDLE);

    unsigned char** data = ReadBMP(argv[1]);
    char** maze = BMPDataToMazeMatrix(data);

    std::thread t1(DeleteArray<unsigned char>, height, data);

    if(!SolveMazeDFS(maze)) puts("No solution");
    else DrawMaze(maze);

    std::thread t2(DeleteArray<char>, height, maze);

    printf("\n\nPress key to continue...");
    std::cin.get();

    t1.join();
    t2.join();

    return 0;
}
