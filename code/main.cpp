#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32; 
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

global_variable bool running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void RenderGradient(win32_offscreen_buffer Buffer, int XOffest,int YOffset)
{
  uint8 *Row =(uint8 *)Buffer.Memory;
  
  for(int Y = 0; Y < Buffer.Height; ++Y){
    uint8 *Pixel = (uint8 *)Row;
    for(int X = 0; X < Buffer.Width; ++X){
      *Pixel = (uint8)(X + XOffest);
      ++Pixel;  
      *Pixel = (uint8)(Y + YOffset);
      ++Pixel;  
      *Pixel = 0;
      ++Pixel;  
      *Pixel = 0;
      ++Pixel;  
    }
    Row += Buffer.Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

  if(Buffer->Memory)
  {
    VirtualFree(Buffer->Memory,0,MEM_RELEASE);
  }
  Buffer->Width = Width;
  Buffer->Height = Height;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;//top down
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  
  Buffer->Pitch = Width*Buffer->BytesPerPixel;
}
internal void Win32DisplayBufferWindow(HDC DeviceContext, RECT ClientRect,win32_offscreen_buffer Buffer, int X, int Y, int Width, int Height)
{
  int WindowWidth = ClientRect.right - ClientRect.left;
  int WindowHeight = ClientRect.bottom - ClientRect.top;
  StretchDIBits(
      DeviceContext,
      0, 0, Buffer.Width, Buffer.Height, 
      0, 0, WindowWidth, WindowHeight, 
      Buffer.Memory,
      &Buffer.Info,
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
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(&GlobalBackBuffer,Width,Height);
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
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      Win32DisplayBufferWindow(DeviceContext, ClientRect,GlobalBackBuffer, X, Y, Width, Height);
      EndPaint(Window, &Paint);
    }
    break;

    default:
    {
      //OutputDebugStringA("default\n");
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
    HWND Window =
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
    if(Window)
    {
      int XOffset = 0;
      int YOffset = 0;
      running = true;
      while(running)
      {
        //proccess/flush queue
        MSG Message;
        while (PeekMessage(&Message,0,0,0, PM_REMOVE)) {
            if(Message.message == WM_QUIT){
              running = false;
            }
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    
        RenderGradient(GlobalBackBuffer,XOffset,YOffset);

        HDC DeviceContext = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32DisplayBufferWindow(DeviceContext, ClientRect,GlobalBackBuffer, 0, 0, WindowWidth, WindowHeight);
        ReleaseDC(Window, DeviceContext);

        ++XOffset;
        YOffset += 2;
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
