#pragma once
#include "stdafx.h"

bool ThreadTLSConstructor();
inline void ThreadTLSDestructor();
void RegisterThreadId();
inline void UnregisterThreadId();
inline bool IsMyThread(DWORD threadID);
VOID NTAPI tls_callback(PVOID DllHandle, DWORD Reason, PVOID Reserved);