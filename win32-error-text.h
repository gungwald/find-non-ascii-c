/*
 * win32errortext.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Bill
 */

#ifndef WIN32_ERROR_TEXT_H
#define WIN32_ERROR_TEXT_H

#include <Windows.h>	/* DWORD */
#include <string.h>	/* wchar_t, size_t */

extern wchar_t *getErrorText(DWORD eCode, wchar_t *text, size_t textCapacity);
extern wchar_t *getMessage(DWORD errorCode);

#endif
