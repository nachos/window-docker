#include <nan.h>

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <psapi.h>
#include <string>

#include <sstream>

#include <vector>

#include <iostream>
#include <fstream>

using namespace std;

using v8::Local;
using v8::FunctionTemplate;
using v8::Number;
using v8::Handle;
using v8::Object;
using v8::String;
using v8::Array;

using Nan::GetFunction;
using Nan::New;
using Nan::Set;
using Nan::True;
using Nan::False;
using Nan::Export;

#define WINDOWZ_HANDLE(name) HWND name = (HWND)info[0]->Uint32Value();

void PASCAL AppBarQuerySetPos(PAPPBARDATA pabd, UINT edge, LPRECT lprc)
{
      int iHeight = 0;
      int iWidth = 0;

      pabd->rc = *lprc;
      pabd->uEdge = edge;

      if ((edge == ABE_LEFT) || (edge == ABE_RIGHT))
      {
          iWidth = pabd->rc.right - pabd->rc.left;
          pabd->rc.top = 0;
          pabd->rc.bottom = GetSystemMetrics(SM_CYSCREEN);
      }
      else
      {
          iHeight = pabd->rc.bottom - pabd->rc.top;
          pabd->rc.left = 0;
          pabd->rc.right = GetSystemMetrics(SM_CXSCREEN);
      }

      SHAppBarMessage(ABM_QUERYPOS, pabd);

      switch (edge)
      {
          case ABE_LEFT:
              pabd->rc.right = pabd->rc.left + iWidth;
              break;

          case ABE_RIGHT:
              pabd->rc.left = pabd->rc.right - iWidth;
              break;

          case ABE_TOP:
              pabd->rc.bottom = pabd->rc.top + iHeight;
              break;

          case ABE_BOTTOM:
              pabd->rc.top = pabd->rc.bottom - iHeight;
              break;
      }

      SHAppBarMessage(ABM_SETPOS, pabd);

      MoveWindow(pabd->hWnd,
                     pabd->rc.left,
                     pabd->rc.top,
                     pabd->rc.right - pabd->rc.left,
                     pabd->rc.bottom - pabd->rc.top,
                     TRUE);
}

void PASCAL AppBarPosChanged(PAPPBARDATA pabd)
{
    RECT rc;
    RECT rcWindow;
    int iHeight;
    int iWidth;

    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(pabd->hWnd, &rcWindow);

    iHeight = rcWindow.bottom - rcWindow.top;
    iWidth = rcWindow.right - rcWindow.left;

    int edge = ABE_TOP;

    switch (edge)
    {
        case ABE_TOP:
            rc.bottom = rc.top + iHeight;
            break;

        case ABE_BOTTOM:
            rc.top = rc.bottom - iHeight;
            break;

        case ABE_LEFT:
            rc.right = rc.left + iWidth;
            break;

        case ABE_RIGHT:
            rc.left = rc.right - iWidth;
            break;
    }

    AppBarQuerySetPos(pabd, edge, &rc);
}

void appBarCallback(HWND hwndAccessBar, UINT uNotifyMsg,
    LPARAM lParam)
{
    APPBARDATA abd;
    UINT uState;

    abd.cbSize = sizeof(abd);
    abd.hWnd = hwndAccessBar;

    switch (uNotifyMsg)
    {
        case ABN_POSCHANGED:
            // The taskbar or another appbar has changed its size or position.
            AppBarPosChanged(&abd);
            break;
    }
}

NAN_METHOD(setAppbarPos) {
  WINDOWZ_HANDLE(hwnd)
  UINT edge = ABE_TOP;// info[1]->Int32Value();

  APPBARDATA abd;
  abd.cbSize = sizeof(abd);
  abd.hWnd = hwnd;

  RECT rcWindow;
  GetWindowRect(abd.hWnd, &rcWindow);

  AppBarQuerySetPos(&abd, edge, &rcWindow);
}

NAN_METHOD(registerAsAppBar) {
  WINDOWZ_HANDLE(hwnd)
  APPBARDATA abd;

  abd.cbSize = sizeof(abd);
  abd.hWnd = hwnd;

  abd.uCallbackMessage = (WM_USER + 0x01);

  int error = SHAppBarMessage(ABM_NEW, &abd);

  info.GetReturnValue().Set(New<Number>(error));
}

NAN_MODULE_INIT(init) {
  NAN_EXPORT(target, registerAsAppBar);
  NAN_EXPORT(target, setAppbarPos);
  NAN_EXPORT(target, appBarCallback);
}

NODE_MODULE(docker, init)