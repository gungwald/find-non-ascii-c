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

// The _WIN32 macro is set for both 32-bit and 64-bit Windows targets.
// It is the accepted way to detect Windows.
#ifndef _WIN32
#include <langinfo.h>	/* nl_langinfo */
#define FOPEN_READ_MODE "r"
#endif

#ifdef _WIN32
#include <windows.h>	/* SetConsoleOutputCP, CP_UTF8 */
#include <io.h>		/* _setmode */
#include <fcntl.h>	/* _O_U8TEXT */
#include "win32-error-text.h"
#define FOPEN_READ_MODE "rt, ccs=UTF-8"
#endif

#define HEX_STR_SIZE 256
#define INPUT_LINE_SIZE 512
#define MULTI_BYTE_CHAR_SIZE 32

// Determine what printf conversion sequence to use for wint_t.
// This type is "wide int", not "Win NT".
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

static bool findNonAscii(FILE *f, char *name);
static char *bytesToHexString(char *bytes);
static char *wideCharToMultiByte(wchar_t c);
static void checkLocale();
static const char *findCharEncNameInLocale(const char *locale);
static bool isUTF8CharEncName(const char *charEncodingName);

int main(int argc, char *argv[]) {
#ifdef _WIN32
    // Turn on wchar_t to UTF-8 translation for stdout (wprintf).
    // This requires all output to be wide character. Printf will fail.
    _setmode(_fileno(stdout), _O_U8TEXT);
#endif

    //checkLocale();

#ifdef DEBUG
    wprintf(L"size_t is %lu bytes\n", sizeof(size_t));
    wprintf(L"wint_t is %lu bytes\n", sizeof(wint_t));
    wprintf(L"WINVER is %lu\n", WINVER);
    wprintf(L"WINT_MAX is %lu\n", WINT_MAX);
    wprintf(L"UINT_MAX is %lu\n", UINT_MAX);
    wprintf(L"FOPEN_READ_MODE is %s\n", FOPEN_READ_MODE);
    fflush(stdout);
#endif

    int exitCode = EXIT_SUCCESS;

    if (argc > 1) {
        if (argc == 2 && strcmp(argv[1], "-") == 0) {
            if (! findNonAscii(stdin, "stdin")) {
                exitCode = EXIT_FAILURE;
            }
        }
        else {
            for (int i = 1; i < argc && exitCode == EXIT_SUCCESS; i++) {
                char *fileName = argv[i];
                FILE *in = fopen(fileName, FOPEN_READ_MODE);
                if (in == NULL) {
                    perror(fileName);
                    exitCode = EXIT_FAILURE;
                }
                else {
                    if (! findNonAscii(in, fileName)) {
                        exitCode = EXIT_FAILURE;
                    }
                    if (fclose(in) == EOF) {
                        fwprintf(stderr, L"%S: Failed to close file: %S\n", fileName,
                                 strerror(errno));
                        exitCode = EXIT_FAILURE;
                    }
                }
            }
        }
    }
    else {
        fwprintf(stderr, L"%S: Exiting because no input files were provided\n", basename(argv[0]));
    }
    return exitCode;
}

bool findNonAscii(FILE *f, char *name) {
    wint_t c;
    uint32_t lineNum = 1;
    uint32_t columnNum = 0;
    bool success = true;
    while ((c = fgetwc(f)) != WEOF) {
        columnNum++;
        if (c > 127) {
            // Windows requires wprintf rather than printf to get the
            // wchar_t to UTF-8 automatic translation to work.
            // Linux works with printf as long as the conversion char
            // sequence is %lc.
            wprintf(L"%hs:%" PRIu32 ",%" PRIu32 ": char='%lc' code=%" PRINT_WINT_T
                    " bytes=[%hs]\n",
                    name, lineNum, columnNum, c, c, bytesToHexString(wideCharToMultiByte(c)));
        }
        else if (c == L'\n') {
            lineNum++;
            columnNum = 0;
        }
    }
    if (ferror(f)) {
        fwprintf(stderr, L"%S: Read failed at line %" PRIu32 ": %S\n", name, lineNum,
                 strerror(errno));
        success = false;
    }
    return success;
}

char *bytesToHexString(char *bytes) {
    static char hexString[HEX_STR_SIZE];
    int byteCount = strlen(bytes);
    if (byteCount > 0) {
        uint8_t byteValue = (uint8_t) bytes[0];
        char charHexValue[6];
        snprintf(charHexValue, 6, "%" PRIx8, byteValue);
        strncpy(hexString, charHexValue, 5);
        for (int i = 1; i < byteCount; i++) {
            snprintf(charHexValue, 6, " %" PRIx8, (uint8_t) bytes[i]);
            strncat(hexString, charHexValue, 5);
        }
    }
    return hexString;
}

char *wideCharToMultiByte(wchar_t c) {
    static char multiByteChar[MULTI_BYTE_CHAR_SIZE];
#ifdef _WIN32
    int byteCount = WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        &c,
                        1,
                        multiByteChar,
                        MULTI_BYTE_CHAR_SIZE,
                        NULL,
                        NULL
                    );
    if (byteCount == 0) {
        wchar_t *msg = getMessage(GetLastError());
        fwprintf(stderr, L"WideCharToMultiByte failed: %s\n", msg);
        exit(EXIT_FAILURE);
    }
#else
    int byteCount = wctomb(multiByteChar, c);
    if (byteCount == -1) {
        fprintf(stderr, "wctomb failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
#endif
    multiByteChar[byteCount] = '\0';
    return multiByteChar;
}

void checkLocale() {
#ifdef DEBUG
    // Get the current locale
    char *locale = setlocale(LC_ALL, NULL);
    const char *encName = findCharEncNameInLocale(locale);
    wprintf(L"Locale: %S\n", locale);
    wprintf(L"Encdng: %S\n", encName);
#endif

    /*
        if (! isUTF8CharEncName(encName)) {
    	fprintf(stderr, "\nWARNING: Possible character encoding mismatch\n\n"
    		"To properly decode any non-ASCII characters stored in the input\n"
    		"files, you must match the character encoding of the current\n"
    	        "session with the one that was previously used to save the file.\n"
    		"The current session is configured to use the character encoding:\n\n"
    		"	%s\n\n"
    		"If this does not match the encoding that was previously used to\n"
    		"save the file, non-ASCII characters will still be detected. But\n"
    	        "they won't be identified correctly.\n\n"
    		"If your input files have been saved in the UTF-8 encoding, which\n"
    		"is most likely the case, use the following commands to set the\n"
    	        "character encoding of your session to UTF-8:\n\n"
    		"	Windows:   CHCP 65001\n"
    		"	Linux/Mac: export LANG=en_US.UTF-8\n\n", encName);
    	fflush(stdout);
        }
    */

// The _WIN32 macro is set for both 32-bit and 64-bit Windows targets.
// It is the accepted way to detect Windows.
#ifdef _WIN32
    // This is not necessary as it only affects output from ASCII functions,
    // not the wide character functions used in this program.
    //if (GetConsoleOutputCP() != CP_UTF8) {
    //fwprintf(stderr, L"WARNING: Console code page is %d. Setting to %d for UTF-8 output.\n", GetConsoleOutputCP(), CP_UTF8);
    //SetConsoleOutputCP(CP_UTF8);
    //}
    // The locale has to be set to UTF-8 for wctomb to generate UTF-8 chars on both Windows and Linux.
    // Setting the code page does not set the locale.
    // Windows considers it an error to try to set the locale to UTF-8, so we can't do that. Catch-22.
    // Windows will not accept a UTF-8 parameter with these functions:
    // setlocale
    // _create_locale
    // _wctomb_l
    // So the only option left is to use the Win32 API: WideCharToMultiByte.
#else
    // Initialize locale settings from LANG environment variable.
    setlocale(LC_ALL, "");
    char *codeset = nl_langinfo(CODESET);
    if (strcmp(codeset, "UTF-8") != 0) {
        fprintf(stderr, "WARNING: Character encoding is not set to UTF-8: %s\n", codeset);
    }
#endif

}

/**
 * Returns a pointer into the <code>locale</code> variable.
 */
const char *findCharEncNameInLocale(const char *locale) {
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

bool isUTF8CharEncName(const char *charEncodingName) {
    bool foundMatch = false;
    if (charEncodingName != NULL) {
        char *lowerName = toLowerCase(charEncodingName);
        char *utf8Names[] = {"utf8", "utf-8", NULL};
        for (int i = 0; utf8Names[i] != NULL && !foundMatch; i++) {
            if (strcmp(lowerName, utf8Names[i]) == 0) {
                foundMatch = true;
            }
        }
        free(lowerName);
    }
    return foundMatch;
}
