#include <stdio.h>	/* printf, fprintf, snprintf, FILE, fopen, fgetwc */
#include <stdlib.h> 	/* EXIT_SUCCESS */
#include <stdbool.h>	/* bool, true, false */
#include <wchar.h>	/* wchar_t */
#include <string.h>	/* strncat, strdup */
#include <errno.h>	/* errno */
#include <locale.h>	/* setlocale */
#include <libgen.h>	/* basename */
#include <inttypes.h>   /* PRIx8, PRIu32 */
#include <limits.h>	/* UINT_MAX */

#include "java-string-compat.h"

#define HEX_STR_SIZE 256
#define INPUT_LINE_SIZE 512

// Determine what printf conversion sequence to use for wint_t.
#if WINT_MAX == INT_MAX
#define PRINT_WINT_T "d"
#elif WINT_MAX == LONG_MAX
#define PRINT_WINT_T "ld"
#elif WINT_MAX == UINT_MAX
#define PRINT_WINT_T "u"
#elif WINT_MAX == SHRT_MAX
#define PRINT_WINT_T "hd"
#elif WINT_MAX == USHRT_MAX
#define PRINT_WINT_T "hu"
#endif

bool findNonAscii(FILE *f, char *name);
char *wideCharToMultiByteHex(wchar_t c);
void checkLocale();
const char *findCharEncNameInLocale(const char *locale);
bool isUTF8CharEncName(const char *charEncodingName);

int main(int argc, char *argv[])
{

#ifdef DEBUG
    printf("wint_t is %d bytes\n", (int) sizeof(wint_t));
    printf("WINT_MAX is %u\n", WINT_MAX);
    printf("UINT_MAX is %u\n", UINT_MAX);
#endif

    // Initialize locale settings from LANG environment variable.
    setlocale(LC_ALL, "");
    checkLocale();

    int exitCode = EXIT_SUCCESS;

    for (int i = 1; i < argc && exitCode == EXIT_SUCCESS; i++) {
        char *fileName = argv[i];
        FILE *in = fopen(fileName, "r");
        if (in == NULL) {
            perror(fileName);
	    exitCode = EXIT_FAILURE;
        }
        else {
	    if (! findNonAscii(in, fileName)) {
		exitCode = EXIT_FAILURE;
	    }
            if (fclose(in) == EOF) {
                fprintf(stderr, "%s: Failed to close file: %s\n", fileName, 
			strerror(errno));
		exitCode = EXIT_FAILURE;
            }
        }
    }
    return exitCode;
}

bool findNonAscii(FILE *f, char *name)
{
    wint_t c;
    uint32_t lineNum = 1;
    uint32_t columnNum = 0;
    bool success = true;
    while ((c = fgetwc(f)) != WEOF) {
	columnNum++;
        if (c > 127) {
            printf("%s:%" PRIu32 ",%" PRIu32 ": char='%lc' code=%" PRINT_WINT_T 
    		" bytes=[%s]\n",
	    	name, lineNum, columnNum, c, c, wideCharToMultiByteHex(c));
        }
        else if (c == L'\n') {
            lineNum++;
	    columnNum = 0;
        }
    }
    if (ferror(f)) {
        fprintf(stderr, "%s: Read failed at line %" PRIu32 ": %s\n", name, lineNum, 
		strerror(errno));
	success = false;
    }
    return success;
}

char *wideCharToMultiByteHex(wchar_t c)
{
    static char hexString[HEX_STR_SIZE];
    hexString[0] = '\0';
    char bytes[MB_CUR_MAX + 1];
    int byteCount = wctomb(bytes, c);
    if (byteCount > 0) {
        uint8_t byteValue = (uint8_t) bytes[0];
        snprintf(hexString, 6, "%" PRIx8, byteValue);
        for (int i = 1; i < byteCount; i++) {
            char charHexValue[6];
            snprintf(charHexValue, 6, " %" PRIx8, (uint8_t) bytes[i]);
	    strncat(hexString, charHexValue, 5);
        }
    }
    return hexString;
}

void checkLocale()
{
    // Get the current locale
    char *locale = setlocale(LC_ALL, NULL);
    char *encName = findCharEncNameInLocale(locale);

    if (! isUTF8CharEncName(encName)) {
	printf("WARNING: "
		"To properly decode any non-ASCII characters stored in the file,\n"
		"you must match the character encoding used to read the file\n"
	        "with the one that was used to save the file.\n"
		"This session is currently configured to read the file using\n"
		"the character encoding: %s. If this does not match the encoding\n"
		"used to save the file, non-ASCII characters can still be detected\n"
		"but they won't be identified correctly.\n", encName);
	fflush(stdout);
    }
}

/**
 * Returns a pointer into the <code>locale</code> variable.
 */
const char *findCharEncNameInLocale(const char *locale)
{
	const char *charEncName = NULL;
	const char *dotPtr = strrchr(locale, '.');
	if (dotPtr != NULL) {
		charEncName = dotPtr + 1;
		if (charEncName[0] == '\0') {
			charEncName = NULL;
		}
	}
	return charEncName;
}

bool isUTF8CharEncName(const char *charEncodingName)
{
	bool foundMatch = false;
	if (charEncodingName != NULL) {
		char *lowerName = toLowerCase(charEncodingName);
		char *utf8Names[] = {"utf8", "utf-8"};
		for (int i = 0; i < sizeof(utf8Names) && !foundMatch; i++) {
			if (strcmp(lowerName, utf8Names[i]) == 0) {
				foundMatch = true;
			}
		}
		free(lowerName);
	}
	return foundMatch;
}

