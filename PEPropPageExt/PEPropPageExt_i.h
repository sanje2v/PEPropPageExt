

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Oct 25 11:21:00 2013
 */
/* Compiler settings for PEPropPageExt.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __PEPropPageExt_i_h__
#define __PEPropPageExt_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPEPropPageExt_FWD_DEFINED__
#define __IPEPropPageExt_FWD_DEFINED__
typedef interface IPEPropPageExt IPEPropPageExt;
#endif 	/* __IPEPropPageExt_FWD_DEFINED__ */


#ifndef __PEPropPageExt_FWD_DEFINED__
#define __PEPropPageExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class PEPropPageExt PEPropPageExt;
#else
typedef struct PEPropPageExt PEPropPageExt;
#endif /* __cplusplus */

#endif 	/* __PEPropPageExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IPEPropPageExt_INTERFACE_DEFINED__
#define __IPEPropPageExt_INTERFACE_DEFINED__

/* interface IPEPropPageExt */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IPEPropPageExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1BF849B2-6F21-44DD-8376-8EB867E4F83A")
    IPEPropPageExt : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Fx( 
            int iValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPEPropPageExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPEPropPageExt * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPEPropPageExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPEPropPageExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *Fx )( 
            IPEPropPageExt * This,
            int iValue);
        
        END_INTERFACE
    } IPEPropPageExtVtbl;

    interface IPEPropPageExt
    {
        CONST_VTBL struct IPEPropPageExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPEPropPageExt_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPEPropPageExt_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPEPropPageExt_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPEPropPageExt_Fx(This,iValue)	\
    ( (This)->lpVtbl -> Fx(This,iValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPEPropPageExt_INTERFACE_DEFINED__ */



#ifndef __PEPropPageExtLib_LIBRARY_DEFINED__
#define __PEPropPageExtLib_LIBRARY_DEFINED__

/* library PEPropPageExtLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_PEPropPageExtLib;

EXTERN_C const CLSID CLSID_PEPropPageExt;

#ifdef __cplusplus

class DECLSPEC_UUID("9939CDB0-9819-4a29-85D3-6E1897260ED4")
PEPropPageExt;
#endif
#endif /* __PEPropPageExtLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


