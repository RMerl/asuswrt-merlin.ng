/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM IDNSSDService.idl
 */

#ifndef __gen_IDNSSDService_h__
#define __gen_IDNSSDService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class IDNSSDService; /* forward declaration */


/* starting interface:    IDNSSDBrowseListener */
#define IDNSSDBROWSELISTENER_IID_STR "27346495-a1ed-458a-a5bc-587df9a26b4f"

#define IDNSSDBROWSELISTENER_IID \
  {0x27346495, 0xa1ed, 0x458a, \
    { 0xa5, 0xbc, 0x58, 0x7d, 0xf9, 0xa2, 0x6b, 0x4f }}

class NS_NO_VTABLE NS_SCRIPTABLE IDNSSDBrowseListener : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(IDNSSDBROWSELISTENER_IID)

  /* void onBrowse (in IDNSSDService service, in boolean add, in long interfaceIndex, in long error, in AString serviceName, in AString regtype, in AString domain); */
  NS_SCRIPTABLE NS_IMETHOD OnBrowse(IDNSSDService *service, PRBool add, PRInt32 interfaceIndex, PRInt32 error, const nsAString & serviceName, const nsAString & regtype, const nsAString & domain) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IDNSSDBrowseListener, IDNSSDBROWSELISTENER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IDNSSDBROWSELISTENER \
  NS_SCRIPTABLE NS_IMETHOD OnBrowse(IDNSSDService *service, PRBool add, PRInt32 interfaceIndex, PRInt32 error, const nsAString & serviceName, const nsAString & regtype, const nsAString & domain); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IDNSSDBROWSELISTENER(_to) \
  NS_SCRIPTABLE NS_IMETHOD OnBrowse(IDNSSDService *service, PRBool add, PRInt32 interfaceIndex, PRInt32 error, const nsAString & serviceName, const nsAString & regtype, const nsAString & domain) { return _to OnBrowse(service, add, interfaceIndex, error, serviceName, regtype, domain); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IDNSSDBROWSELISTENER(_to) \
  NS_SCRIPTABLE NS_IMETHOD OnBrowse(IDNSSDService *service, PRBool add, PRInt32 interfaceIndex, PRInt32 error, const nsAString & serviceName, const nsAString & regtype, const nsAString & domain) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnBrowse(service, add, interfaceIndex, error, serviceName, regtype, domain); } 



/* starting interface:    IDNSSDResolveListener */
#define IDNSSDRESOLVELISTENER_IID_STR "6620e18f-47f3-47c6-941f-126a5fd4fcf7"

#define IDNSSDRESOLVELISTENER_IID \
  {0x6620e18f, 0x47f3, 0x47c6, \
    { 0x94, 0x1f, 0x12, 0x6a, 0x5f, 0xd4, 0xfc, 0xf7 }}

class NS_NO_VTABLE NS_SCRIPTABLE IDNSSDResolveListener : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(IDNSSDRESOLVELISTENER_IID)

  /* void onResolve (in IDNSSDService service, in long interfaceIndex, in long error, in AString fullname, in AString host, in short port, in AString path); */
  NS_SCRIPTABLE NS_IMETHOD OnResolve(IDNSSDService *service, PRInt32 interfaceIndex, PRInt32 error, const nsAString & fullname, const nsAString & host, PRInt16 port, const nsAString & path) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IDNSSDResolveListener, IDNSSDRESOLVELISTENER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IDNSSDRESOLVELISTENER \
  NS_SCRIPTABLE NS_IMETHOD OnResolve(IDNSSDService *service, PRInt32 interfaceIndex, PRInt32 error, const nsAString & fullname, const nsAString & host, PRInt16 port, const nsAString & path); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IDNSSDRESOLVELISTENER(_to) \
  NS_SCRIPTABLE NS_IMETHOD OnResolve(IDNSSDService *service, PRInt32 interfaceIndex, PRInt32 error, const nsAString & fullname, const nsAString & host, PRInt16 port, const nsAString & path) { return _to OnResolve(service, interfaceIndex, error, fullname, host, port, path); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IDNSSDRESOLVELISTENER(_to) \
  NS_SCRIPTABLE NS_IMETHOD OnResolve(IDNSSDService *service, PRInt32 interfaceIndex, PRInt32 error, const nsAString & fullname, const nsAString & host, PRInt16 port, const nsAString & path) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnResolve(service, interfaceIndex, error, fullname, host, port, path); } 



/* starting interface:    IDNSSDService */
#define IDNSSDSERVICE_IID_STR "3a3539ff-f8d8-40b4-8d02-5ea73c51fa12"

#define IDNSSDSERVICE_IID \
  {0x3a3539ff, 0xf8d8, 0x40b4, \
    { 0x8d, 0x02, 0x5e, 0xa7, 0x3c, 0x51, 0xfa, 0x12 }}

class NS_NO_VTABLE NS_SCRIPTABLE IDNSSDService : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(IDNSSDSERVICE_IID)

  /* IDNSSDService browse (in long interfaceIndex, in AString regtype, in AString domain, in IDNSSDBrowseListener listener); */
  NS_SCRIPTABLE NS_IMETHOD Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IDNSSDBrowseListener *listener, IDNSSDService **_retval NS_OUTPARAM) = 0;

  /* IDNSSDService resolve (in long interfaceIndex, in AString name, in AString regtype, in AString domain, in IDNSSDResolveListener listener); */
  NS_SCRIPTABLE NS_IMETHOD Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, IDNSSDResolveListener *listener, IDNSSDService **_retval NS_OUTPARAM) = 0;

  /* void stop (); */
  NS_SCRIPTABLE NS_IMETHOD Stop(void) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(IDNSSDService, IDNSSDSERVICE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IDNSSDSERVICE \
  NS_SCRIPTABLE NS_IMETHOD Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IDNSSDBrowseListener *listener, IDNSSDService **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, IDNSSDResolveListener *listener, IDNSSDService **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD Stop(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IDNSSDSERVICE(_to) \
  NS_SCRIPTABLE NS_IMETHOD Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IDNSSDBrowseListener *listener, IDNSSDService **_retval NS_OUTPARAM) { return _to Browse(interfaceIndex, regtype, domain, listener, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, IDNSSDResolveListener *listener, IDNSSDService **_retval NS_OUTPARAM) { return _to Resolve(interfaceIndex, name, regtype, domain, listener, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Stop(void) { return _to Stop(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IDNSSDSERVICE(_to) \
  NS_SCRIPTABLE NS_IMETHOD Browse(PRInt32 interfaceIndex, const nsAString & regtype, const nsAString & domain, IDNSSDBrowseListener *listener, IDNSSDService **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->Browse(interfaceIndex, regtype, domain, listener, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Resolve(PRInt32 interfaceIndex, const nsAString & name, const nsAString & regtype, const nsAString & domain, IDNSSDResolveListener *listener, IDNSSDService **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->Resolve(interfaceIndex, name, regtype, domain, listener, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Stop(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Stop(); } 



#endif /* __gen_IDNSSDService_h__ */
