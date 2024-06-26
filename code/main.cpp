#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Height, int Width)
{
  if(BitmapHandle)
  {
    DeleteObject(BitmapHandle);
  }
  if(!BitmapDeviceContext)
  {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }

  BITMAPINFO BitmapInfo;
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
 
  //todo future memory management

  BitmapHandle = CreateDIBSection(
    BitmapDeviceContext, 
    &BitmapInfo,
    DIB_RGB_COLORS,
    &BitmapMemory,
    0,0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Height, int Width)
{
  StretchDIBits(
      BitmapDeviceContext,
      X, Y, Width, Height, 
      X, Y, Width, Height,
      BitmapMemory,
      &BitmapInfo,
      DIB_RGB_COLORS,
      SRCCOPY
      );
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
LRESULT Result = 0;

switch (Message) {
  case WM_SIZE:
    {
      RECT ClientRect;
      GetClientRect(Window,&ClientRect);
      int Height = ClientRect.top - ClientRect.bottom;
      int Width = ClientRect.left - ClientRect.right;
      Win32ResizeDIBSection(Width,Height);
    }
    break;

  case WM_DESTROY:
    {
      running = false;
    }
    break;

  case WM_CLOSE:
    {
      running = false;
    }
    break;

  case WM_ACTIVATEAPP:
    {

      OutputDebugStringA("WM_ACTIVEAPP\n");
    }
    break;
  case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint( Window, &Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Height = Paint.rcPaint.top - Paint.rcPaint.bottom;
      int Width = Paint.rcPaint.left - Paint.rcPaint.right;
      Win32UpdateWindow(DeviceContext, X, Y, Height, Width);
      EndPaint(Window, &Paint);
    }
    break;
  //case END_PAINT:
   //// {

    //}
    //bGdi32.libreak;
    default:
    {
      //OutputDebugStringA("default\n")
      Result = DefWindowProc(Window, Message, wParam,lParam);
    }
    break;
}
return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance, 
        HINSTANCE PrevInstance, 
	      LPSTR CommandLine,  
        int ShowCode)
{
  WNDCLASS WindowClass ={};

  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = Instance;
  //WindowClass.hIcon;
  WindowClass.lpszClassName = "KiddEngineWindowClass";
  
if(RegisterClass(&WindowClass))
{
    HWND WindowHandle =
      CreateWindowEx(
        0,
        WindowClass.lpszClassName,
        "Kidd Engine",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        Instance,
        0
      );
    if(WindowHandle)
    {
      running = true;
      while(running)
      {
        MSG Message;
        BOOL messageResult = GetMessage(&Message,0,0,0);
        if(messageResult > 0)
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        else
        {
          break;
        }
      }
    }
    else 
    {
     //todo logging 
    }

}
else 
{
//todo -- logging
};

  return (0);
}
