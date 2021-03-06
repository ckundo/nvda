/*
This file is a part of the NVDA project.
URL: http://www.nvda-project.org/
Copyright 2006-2010 NVDA contributers.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2.0, as published by
    the Free Software Foundation.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
This license can be found at:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <string>
#include <map>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <ole2.h>
#include <rpc.h>
#include "displayModelRemote.h"
#include "gdiHooks.h"

using namespace std;

BOOL CALLBACK EnumChildWindowsProc(HWND hwnd, LPARAM lParam) {
	((deque<HWND>*)lParam)->push_back(hwnd);
	return TRUE;
}

error_status_t displayModelRemote_getWindowTextInRect(handle_t bindingHandle, const long windowHandle, const int left, const int top, const int right, const int bottom, const int minHorizontalWhitespace, const int minVerticalWhitespace, boolean useXML, BSTR* textBuf, BSTR* characterLocationsBuf) {
	HWND hwnd=(HWND)windowHandle;
	deque<HWND> windowDeque;
	EnumChildWindows(hwnd,EnumChildWindowsProc,(LPARAM)&windowDeque);
	windowDeque.push_back(hwnd);
	const bool hasDescendantWindows=(windowDeque.size()>1);
	RECT textRect={left,top,right,bottom};
	displayModel_t* tempModel=NULL;
	if(hasDescendantWindows) {
		tempModel=new displayModel_t;
		for(deque<HWND>::reverse_iterator i=windowDeque.rbegin();i!=windowDeque.rend();++i) {
			if(!IsWindowVisible(*i)) continue;
			displayModelsByWindow.acquire();
			displayModelsMap_t<HWND>::iterator j=displayModelsByWindow.find(*i);
			if(j!=displayModelsByWindow.end()) {
				j->second->acquire();
				j->second->copyRectangle(textRect,FALSE,FALSE,false,textRect,NULL,tempModel);
				j->second->release();
			}
			displayModelsByWindow.release();
		}
	} else { //hasDescendantWindows is False
		displayModelsByWindow.acquire();
		displayModelsMap_t<HWND>::iterator i=displayModelsByWindow.find(hwnd);
		if(i!=displayModelsByWindow.end()) {
			i->second->acquire();
			tempModel=i->second;
		}
		displayModelsByWindow.release();
	}
	if(tempModel) {
		wstring text;
		deque<charLocation_t> characterLocations;
		tempModel->renderText(textRect,minHorizontalWhitespace,minVerticalWhitespace,hasDescendantWindows,useXML!=false,text,characterLocations);
		if(hasDescendantWindows) {
			tempModel->requestDelete();
		} else {
			tempModel->release();
		}
		*textBuf=SysAllocStringLen(text.c_str(),static_cast<UINT>(text.size()));
		size_t cpBufSize=characterLocations.size()*5;
		// Hackishly use a BSTR to contain points.
		wchar_t* cpTempBuf=(wchar_t*)malloc(cpBufSize*sizeof(wchar_t));
		wchar_t* cpTempBufIt=cpTempBuf;
		for(deque<charLocation_t>::const_iterator cpIt=characterLocations.begin();cpIt!=characterLocations.end();++cpIt) {
			*(cpTempBufIt++)=(wchar_t)cpIt->left;
			*(cpTempBufIt++)=(wchar_t)cpIt->top;
			*(cpTempBufIt++)=(wchar_t)cpIt->right;
			*(cpTempBufIt++)=(wchar_t)cpIt->bottom;
			*(cpTempBufIt++)=(wchar_t)cpIt->baseline;
		}
		*characterLocationsBuf=SysAllocStringLen(cpTempBuf,static_cast<UINT>(cpBufSize));
		free(cpTempBuf);
	}
	return 0;
}

error_status_t displayModelRemote_requestTextChangeNotificationsForWindow(handle_t bindingHandle, const long windowHandle, const BOOL enable) {
	if(enable) windowsForTextChangeNotifications[(HWND)windowHandle]+=1; else windowsForTextChangeNotifications[(HWND)windowHandle]-=1;
	return 0;
}
