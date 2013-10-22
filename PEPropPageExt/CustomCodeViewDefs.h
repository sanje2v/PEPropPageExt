#pragma once
#include <Windows.h>

///////////////////////////////////////////////////////////////////////////////
// CodeView debug information structures 
//

#define CV_SIGNATURE_NB10   '01BN'
#define CV_SIGNATURE_RSDS   'SDSR'

// CodeView header 
struct CV_HEADER 
{
	DWORD CvSignature; // NBxx
	LONG  Offset;      // Always 0 for NB10
};

// CodeView NB10 debug information 
// (used when debug information is stored in a PDB 2.00 file) 
struct CV_INFO_PDB20 
{
	CV_HEADER  Header; 
	DWORD      Signature;       // seconds since 01.01.1970
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
};

// CodeView RSDS debug information 
// (used when debug information is stored in a PDB 7.00 file) 
struct CV_INFO_PDB70 
{
	DWORD      CvSignature; 
	GUID       Signature;       // unique identifier 
	DWORD      Age;             // an always-incrementing value 
	BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file 
};