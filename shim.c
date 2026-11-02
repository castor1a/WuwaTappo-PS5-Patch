#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <hidsdi.h>
#include <stdio.h>
#include "report.h"
static SRWLOCK lock=SRWLOCK_INIT;
static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static XINPUT_STATE latest;
static BOOL connected;
static DWORD (WINAPI *real_state)(DWORD,XINPUT_STATE*);
static HMODULE self;
static HANDLE open_pad(BOOL *bluetooth){
 UINT n=0;if(GetRawInputDeviceList(NULL,&n,sizeof(RAWINPUTDEVICELIST))==(UINT)-1||!n)return INVALID_HANDLE_VALUE;
 RAWINPUTDEVICELIST *list=HeapAlloc(GetProcessHeap(),0,n*sizeof(*list));
 if(!list)return INVALID_HANDLE_VALUE;
 HANDLE result=INVALID_HANDLE_VALUE;
 if(GetRawInputDeviceList(list,&n,sizeof(*list))!=(UINT)-1)for(UINT i=0;i<n;i++){
  if(list[i].dwType!=RIM_TYPEHID)continue;
  WCHAR path[1024];UINT size=1024;
  if(GetRawInputDeviceInfoW(list[i].hDevice,RIDI_DEVICENAME,path,&size)==(UINT)-1)continue;
  HANDLE h=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if(h==INVALID_HANDLE_VALUE)continue;
  HIDD_ATTRIBUTES a={0};a.Size=sizeof(a);
  if(HidD_GetAttributes(h,&a)&&a.VendorID==0x054c&&(a.ProductID==0x0ce6||a.ProductID==0x0df2)){
   *bluetooth=(wcsstr(path,L"00001124")!=NULL);result=h;break;
  }
  CloseHandle(h);
 }
 HeapFree(GetProcessHeap(),0,list);return result;
}
static DWORD WINAPI reader(void *unused){
 (void)unused;HANDLE h=INVALID_HANDLE_VALUE;BOOL bt=FALSE;
 for(;;){
  if(h==INVALID_HANDLE_VALUE){h=open_pad(&bt);if(h==INVALID_HANDLE_VALUE){Sleep(1500);continue;}}
  unsigned char data[78]={0};data[0]=bt?0x31:1;unsigned len=bt?78:64;
  XINPUT_GAMEPAD g;
  if(HidD_GetInputReport(h,data,len)&&parse_report(data,len,&g)){
   AcquireSRWLockExclusive(&lock);
   if(!connected||memcmp(&g,&latest.Gamepad,sizeof(g))!=0){latest.Gamepad=g;latest.dwPacketNumber++;}
   connected=TRUE;ReleaseSRWLockExclusive(&lock);Sleep(8);
  }else{
   CloseHandle(h);h=INVALID_HANDLE_VALUE;
   AcquireSRWLockExclusive(&lock);connected=FALSE;ZeroMemory(&latest.Gamepad,sizeof(latest.Gamepad));ReleaseSRWLockExclusive(&lock);Sleep(500);
  }
 }
 return 0;
}
static BOOL CALLBACK initialize(PINIT_ONCE o,PVOID arg,PVOID *ctx){
 (void)o;(void)arg;(void)ctx;
 WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,MAX_PATH);
 if(n&&n<MAX_PATH-16){wcscat(path,L"\\xinput1_4.dll");HMODULE real=LoadLibraryW(path);if(real)real_state=(void*)GetProcAddress(real,"XInputGetState");}
 /* Pin the module: the worker must never execute in an unloaded DLL. */
 HMODULE pinned;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)&initialize,&pinned);
 HANDLE t=CreateThread(NULL,0,reader,NULL,0,NULL);if(t)CloseHandle(t);
 return TRUE;
}
__declspec(dllexport) DWORD WINAPI XInputGetState(DWORD index,XINPUT_STATE *state){
 if(!state)return ERROR_BAD_ARGUMENTS;
 if(index>3)return ERROR_DEVICE_NOT_CONNECTED;
 InitOnceExecuteOnce(&once,initialize,NULL,NULL);
 if(real_state){DWORD r=real_state(index,state);if(r==ERROR_SUCCESS)return r;}
 if(index!=0)return ERROR_DEVICE_NOT_CONNECTED;
 AcquireSRWLockShared(&lock);BOOL ok=connected;if(ok)*state=latest;ReleaseSRWLockShared(&lock);
 return ok?ERROR_SUCCESS:ERROR_DEVICE_NOT_CONNECTED;
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH){self=h;DisableThreadLibraryCalls(h);}return TRUE;}
#ifdef PROBE
int main(void){for(int i=0;i<20;i++){XINPUT_STATE s={0};DWORD r=XInputGetState(0,&s);printf("result=%lu packet=%lu buttons=%04x LT=%u RT=%u LX=%d LY=%d\n",r,s.dwPacketNumber,s.Gamepad.wButtons,s.Gamepad.bLeftTrigger,s.Gamepad.bRightTrigger,s.Gamepad.sThumbLX,s.Gamepad.sThumbLY);Sleep(100);}return 0;}
#endif
