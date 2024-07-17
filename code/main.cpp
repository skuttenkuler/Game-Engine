#include <windows.h>
#include <stdint.h>
#include <xinput.h>

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
};

struct win32_window_dimension
{
  int Width;
  int Height;
};

global_variable bool running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(xInputGetStateStub)
{
  return -0;
}
global_variable x_input_get_state *XInputGetState_ = xInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(xInputSetStateStub)
{
  return 0;
}
global_variable x_input_set_state *XInputSetState_ = xInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void)
{
  HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
  if(XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary,"XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary,"XInputSetState");
  }
}


internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension result;
  RECT ClientRect;
  GetClientRect(Window,&ClientRect);
  result.Width = ClientRect.right - ClientRect.left;
  result.Height = ClientRect.bottom - ClientRect.top;

  return(result);

}

internal void RenderGradient(win32_offscreen_buffer Buffer, int XOffset,int YOffset)
{
  
  uint8 *Row =(uint8 *)Buffer.Memory;
  
  for(int Y = 0; Y < Buffer.Height; ++Y){
    uint32 *Pixel = (uint32 *)Row;
    for(int X = 0; X < Buffer.Width; ++X){
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);

      *Pixel++ = ((Green << 8) | Blue);
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
  int BytesPerPixel = 4;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;//top down
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  
  int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
  
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Pitch = Width*BytesPerPixel;
}

internal void Win32DisplayBufferWindow(HDC DeviceContext, int WindowWidth,int WindowHeight, win32_offscreen_buffer Buffer)
{
  StretchDIBits(
      DeviceContext,
      0, 0, WindowWidth, WindowHeight, 
      0, 0, Buffer.Width, Buffer.Height, 
      Buffer.Memory,
      &Buffer.Info,
      DIB_RGB_COLORS,
      SRCCOPY
      );
}

internal LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
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
      win32_window_dimension Dimension = Win32GetWindowDimension(Window);
      Win32DisplayBufferWindow(DeviceContext, Dimension.Width, Dimension.Height,GlobalBackBuffer);
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
  Win32LoadXInput();
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
      HDC DeviceContext = GetDC(Window);
      int XOffset = 0;
      int YOffset = 0;
      running = true;
      while(running)
      {
        MSG Message;
        while (PeekMessage(&Message,0,0,0, PM_REMOVE)) {
            if(Message.message == WM_QUIT){
              running = false;
            }
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        //gamepad input devices
        for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
        {
          XINPUT_STATE controllerState;
          if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
          {
            //controller exists
            XINPUT_GAMEPAD *Pad = &controllerState.Gamepad;

            bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool BACK = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;
          
            if(AButton)
            {
              YOffset += 2;
            }
          }
          else
          {
            //controller does not exist
          }
        }
        RenderGradient(GlobalBackBuffer,XOffset,YOffset);
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer);

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
