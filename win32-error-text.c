#include <stdio.h>
#include <tchar.h>
#include "win32-error-text.h"
#include <lm.h>

#define ERR_MSG_SIZE 256

/* This function was taken from Microsoft's Knowledge Base Article 149409
   and modified to fix the formatting. */
wchar_t *getErrorText(DWORD dwLastError, wchar_t *text, size_t textCapacity)
{
    HMODULE hModule = NULL; /* default to system source */
    wchar_t *messageBuffer = L"";
    DWORD dwMessageLength;

    /* If dwLastError is in the network range, load the message source */
    if (dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
        hModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    }

    /* Call FormatMessage() to allow for message text to be acquired
       from the system or the supplied module handle */
    dwMessageLength = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_FROM_SYSTEM | /* always consider system table */
            ((hModule != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
            hModule, /* Module to get message from (NULL == system) */
            dwLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
            (LPWSTR) &messageBuffer,
            0,
            (va_list*) NULL);
        
    if (dwMessageLength) {
		int finalLength;

		if (dwMessageLength < textCapacity) {
			finalLength = dwMessageLength;
		}
		else {
			finalLength = textCapacity - 1;
		}

		wcsncpy_s(text, textCapacity, messageBuffer, finalLength);
		text[finalLength] = L'\0';

        /* Free the buffer allocated by the system */
        LocalFree(messageBuffer);
    }
    else {
    	text = L"(No message found)";
    }

    /* If you loaded a message source, unload it */
    if (hModule != NULL) {
        FreeLibrary(hModule);
    }
	return text;
}

wchar_t *getMessage(DWORD errorCode)
{
	static wchar_t text[ERR_MSG_SIZE];
	return getErrorText(errorCode, text, ERR_MSG_SIZE);
}


