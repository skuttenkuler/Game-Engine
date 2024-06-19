#include <windows.h>


LRESULT CALLBACK 
MainWindowCallback(HWND Window,
              UINT Message,
              WPARAM wParam,
              LPARAM lParam)
{
LRESULT Result = 0;

switch (Message) {
  case WM_SIZE:
    {
      OutputDebugStringA("WM_SIZE\n");
    }
    break;

  case WM_DESTROY:
    {

      OutputDebugStringA("WM_DESTROY\n");
    }
    break;

  case WM_CLOSE:
    {
      PostQuitMessage(0);
      OutputDebugStringA("WM_CLOSE\n");
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
      PatBlt(DeviceContext,X,Y,Width,Height,WHITENESS);
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
  WindowClass.lpfnWndProc = MainWindowCallback;
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
      for(;;)
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
