#include <stdio.h>
#include <io.h>
#include <fcntl.h>

int main()
{
	_setmode(_fileno(stdout), _O_U8TEXT);
	wprintf(L"%c", 0x00de);
	return 0;
}
