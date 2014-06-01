#ifndef __CPUPERF_SERVICE_HEADER__
#define __CPUPERF_SERVICE_HEADER__
BOOL InstallService();
BOOL UnInstallService();
void WINAPI ServiceCtrlHandler(DWORD Opcode);
#endif
