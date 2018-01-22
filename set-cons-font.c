#include <windows.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** args)
{ 
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 20;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    printf("A quick brown fox jumps over the lazy dog\n");

    printf("Setting to Lucida Console: press <Enter> ");
    getchar();
    wcscpy(cfi.FaceName, L"Lucida Console");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

    printf("Setting to Consolas: press <Enter> ");
    getchar();
    wcscpy(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

    printf("Press <Enter> to exit");
    getchar();
    return 0;
}
