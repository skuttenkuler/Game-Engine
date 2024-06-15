#include <windows.h>
#include <iostream> 

typedef struct tagWNDCLASS {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCSTR    lpszMenuName;
  LPCSTR    lpszClassName;
} WNDCLASS, *PWNDCLASS;

int CALLBACK
WinMain(HINSTANCE hInstance, 
        HINSTANCE hPrevInstance, 
	      LPSTR lpCmdLine,  
        int nCmdShow)
{
  MessageBox(0,"This is my engine","Kidd Engine", MB_OK|MB_ICONINFORMATION);
  return (0);
}
