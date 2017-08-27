/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <CoreFoundation/CoreFoundation.h>
#include <dns_sd.h>
#include <UserEventAgentInterface.h>
#include <stdio.h>
#include <stdlib.h>


#pragma mark -
#pragma mark Types
#pragma mark -
static const char*			sPluginIdentifier		= "com.apple.bonjour.events";

// PLIST Keys
static const CFStringRef	sServiceNameKey			= CFSTR("ServiceName");
static const CFStringRef	sServiceTypeKey			= CFSTR("ServiceType");
static const CFStringRef	sServiceDomainKey		= CFSTR("ServiceDomain");

static const CFStringRef	sOnServiceAddKey		= CFSTR("OnServiceAdd");
static const CFStringRef	sOnServiceRemoveKey		= CFSTR("OnServiceRemove");
static const CFStringRef    sWhileServiceExistsKey	= CFSTR("WhileServiceExists");

static const CFStringRef	sLaunchdTokenKey		= CFSTR("LaunchdToken");

static const CFStringRef	sPluginTimersKey		= CFSTR("PluginTimers");


/************************************************
 * Launch Event Dictionary (input from launchd)
 * Passed To: ManageEventsCallback
 *-----------------------------------------------
 * Typing in this dictionary is not enforced 
 * above us. So this may not be true. Type check
 * all input before using it.
 *-----------------------------------------------
 * sServiceNameKey		- CFString (Optional) 
 * sServiceTypeKey		- CFString
 * sServiceDomainKey	- CFString
 *
 * One or more of the following.
 *-----------------------------------
 * sOnServiceAddKey			- CFBoolean
 * sOnServiceRemoveKey		- CFBoolean 
 * sWhileServiceExistsKey	- CFBoolean
 ************************************************/

/************************************************
 * Browser Dictionary
 *-----------------------------------------------
 * sServiceDomainKey - CFString
 * sServiceTypeKey   - CFString
 ************************************************/

/************************************************
 * Event Dictionary
 *-----------------------------------------------
 * sServiceNameKey	 - CFString (Optional) 
 * sLaunchdTokenKey	 - CFNumber
 ************************************************/

typedef struct {
    UserEventAgentInterfaceStruct*		_UserEventAgentInterface;
    CFUUIDRef							_factoryID;
    UInt32								_refCount;
	
	void*								_pluginContext;
	
	CFMutableDictionaryRef				_tokenToBrowserMap;		// Maps a token to a browser that can be used to scan the remaining dictionaries.
	CFMutableDictionaryRef				_browsers;				// A Dictionary of "Browser Dictionarys" where the resposible browser is the key.
	CFMutableDictionaryRef				_onAddEvents;			// A Dictionary of "Event Dictionarys" that describe events to trigger on a service appearing.
	CFMutableDictionaryRef				_onRemoveEvents;		// A Dictionary of "Event Dictionarys" that describe events to trigger on a service disappearing.
	CFMutableDictionaryRef				_whileServiceExist;		// A Dictionary of "Event Dictionarys" that describe events to trigger on a service disappearing.

	
	CFMutableArrayRef					_timers;

} BonjourUserEventsPlugin;


typedef struct {
	
	CFIndex	 refCount;
	BonjourUserEventsPlugin* plugin;
	CFNumberRef token;
	
} TimerContextInfo;

typedef struct {
	CFIndex refCount;
	DNSServiceRef browserRef;
} NetBrowserInfo;

#pragma mark -
#pragma mark Prototypes
#pragma mark -
// COM Stuff
static HRESULT	QueryInterface(void *myInstance, REFIID iid, LPVOID *ppv);
static ULONG	AddRef(void* instance);
static ULONG	Release(void* instance);

static BonjourUserEventsPlugin* Alloc(CFUUIDRef factoryID);
static void Dealloc(BonjourUserEventsPlugin* plugin);

void * UserEventAgentFactory(CFAllocatorRef allocator, CFUUIDRef typeID);

// Plugin Management
static void Install(void* instance);
static void ManageEventsCallback(
						  UserEventAgentLaunchdAction action,
						  CFNumberRef                 token,
						  CFTypeRef                   eventMatchDict,
						  void                      * vContext);


// Plugin Guts
void AddEventToPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchdToken, CFDictionaryRef eventParameters);
void RemoveEventFromPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef	launchToken);

NetBrowserInfo* CreateBrowserForTypeAndDomain(BonjourUserEventsPlugin* plugin, CFStringRef type, CFStringRef domain);
NetBrowserInfo* BrowserForSDRef(BonjourUserEventsPlugin* plugin, DNSServiceRef sdRef);
void AddEventDictionary(CFDictionaryRef eventDict, CFMutableDictionaryRef allEventsDictionary, NetBrowserInfo* key);
void RemoveEventFromArray(CFMutableArrayRef array, CFNumberRef launchdToken);

// Net Service Browser Stuff
void ServiceBrowserCallback (DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* serviceName, const char* regtype, const char* replyDomain, void* context);
void HandleTemporaryEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser, CFStringRef serviceName, CFMutableDictionaryRef eventsDictionary);
void HandleStateEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser,  CFStringRef serviceName, Boolean didAppear);
void TemporaryEventTimerCallout ( CFRunLoopTimerRef timer, void *info );

// Convence Stuff
const char* CStringFromCFString(CFStringRef string);


// TimerContextInfo "Object"
TimerContextInfo* TimerContextInfoCreate(BonjourUserEventsPlugin* plugin, CFNumberRef token);
const void* TimerContextInfoRetain(const void* info);
void TimerContextInfoRelease(const void* info);
CFStringRef TimerContextInfoCopyDescription(const void* info);

// NetBrowserInfo "Object"
NetBrowserInfo* NetBrowserInfoCreate(CFStringRef serviceType, CFStringRef domain, void* context);
const void* NetBrowserInfoRetain(CFAllocatorRef allocator, const void* info);
void NetBrowserInfoRelease(CFAllocatorRef allocator, const void* info);
Boolean	NetBrowserInfoEqual(const void *value1, const void *value2);
CFHashCode	NetBrowserInfoHash(const void *value);
CFStringRef	NetBrowserInfoCopyDescription(const void *value);


static const CFDictionaryKeyCallBacks kNetBrowserInfoDictionaryKeyCallbacks = { 
	0, 
	NetBrowserInfoRetain, 
	NetBrowserInfoRelease,
	NetBrowserInfoCopyDescription,
	NetBrowserInfoEqual,
	NetBrowserInfoHash
};

static const CFDictionaryValueCallBacks kNetBrowserInfoDictionaryValueCallbacks = { 
	0, 
	NetBrowserInfoRetain, 
	NetBrowserInfoRelease,
	NetBrowserInfoCopyDescription,
	NetBrowserInfoEqual	
};

// COM type definition goop.
static UserEventAgentInterfaceStruct UserEventAgentInterfaceFtbl = {
	NULL,                   // Required padding for COM
	QueryInterface,			// Query Interface
	AddRef,					// AddRef()
	Release,				// Release()
	Install					// Install 
}; 

#pragma mark -
#pragma mark COM Management
#pragma mark -

/*****************************************************************************
 *****************************************************************************/
static HRESULT QueryInterface(void *myInstance, REFIID iid, LPVOID *ppv) 
{
	CFUUIDRef interfaceID = CFUUIDCreateFromUUIDBytes(NULL, iid);
	
	// Test the requested ID against the valid interfaces.
	if(CFEqual(interfaceID, kUserEventAgentInterfaceID)) 
	{
		((BonjourUserEventsPlugin *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
		*ppv = myInstance;
		CFRelease(interfaceID);
		return S_OK;
	} 
	else if(CFEqual(interfaceID, IUnknownUUID)) 
	{
		((BonjourUserEventsPlugin *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
		*ppv = myInstance;
		CFRelease(interfaceID);
		return S_OK;
    } 
	else //  Requested interface unknown, bail with error.
	{ 
		*ppv = NULL;
		CFRelease(interfaceID);
		return E_NOINTERFACE;
	}
}

/*****************************************************************************
 *****************************************************************************/
static ULONG AddRef(void* instance)
{
	BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;
	return 	++plugin->_refCount;
}

/*****************************************************************************
 *****************************************************************************/
static ULONG Release(void* instance)
{
	BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;

	if (plugin->_refCount != 0)
		--plugin->_refCount;
	
	if (plugin->_refCount == 0)
	{
		Dealloc(instance);
		return 0;
	}
	
	return plugin->_refCount;
} 

/*****************************************************************************
 * Alloc
 * - 
 * Functionas as both +[alloc] and -[init] for the plugin. Add any 
 * initalization of member variables here.
 *****************************************************************************/
static BonjourUserEventsPlugin* Alloc(CFUUIDRef factoryID)
{
	BonjourUserEventsPlugin* plugin = malloc(sizeof(BonjourUserEventsPlugin));
	
	plugin->_UserEventAgentInterface = &UserEventAgentInterfaceFtbl;
	plugin->_pluginContext = NULL;
		
	if (factoryID) 
	{
		plugin->_factoryID = (CFUUIDRef)CFRetain(factoryID);
		CFPlugInAddInstanceForFactory(factoryID);
	}
	
	plugin->_refCount = 1;
	plugin->_tokenToBrowserMap = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kNetBrowserInfoDictionaryValueCallbacks);
	plugin->_browsers = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
	plugin->_onAddEvents = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
	plugin->_onRemoveEvents = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
	plugin->_whileServiceExist = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
	
	plugin->_timers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	return plugin;
}

/*****************************************************************************
 * Dealloc
 * - 
 * Much like Obj-C dealloc this method is responsible for releasing any object
 * this plugin is holding. Unlike ObjC, you call directly free() instead of 
 * [super dalloc].
 *****************************************************************************/
static void Dealloc(BonjourUserEventsPlugin* plugin)
{
	CFUUIDRef factoryID = plugin->_factoryID;
	
	if (factoryID)
	{
		CFPlugInRemoveInstanceForFactory(factoryID);
		CFRelease(factoryID);
	}
	
	if (plugin->_tokenToBrowserMap)
		CFRelease(plugin->_tokenToBrowserMap);
	
	if (plugin->_browsers)
		CFRelease(plugin->_browsers);
	
	if (plugin->_onAddEvents)
		CFRelease(plugin->_onAddEvents);
	
	if (plugin->_onRemoveEvents)
		CFRelease(plugin->_onRemoveEvents);
	
	if (plugin->_whileServiceExist)
		CFRelease(plugin->_whileServiceExist);
	
	if (plugin->_timers)
	{
		CFIndex i;
		CFIndex count = CFArrayGetCount(plugin->_timers);
		CFRunLoopRef crl = CFRunLoopGetCurrent();
		
		for (i = 0; i < count; ++i)
		{
			CFRunLoopTimerRef timer = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(plugin->_timers, i);
			CFRunLoopRemoveTimer(crl, timer, kCFRunLoopCommonModes);
		}
		
		CFRelease(plugin->_timers);
	}
	
	free(plugin);
}

/*******************************************************************************
 *******************************************************************************/
void * UserEventAgentFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
	(void)allocator;
    BonjourUserEventsPlugin * result = NULL;
	
    if (typeID && CFEqual(typeID, kUserEventAgentTypeID)) {
        result = Alloc(kUserEventAgentFactoryID);
    }
	
    return (void *)result;
}

#pragma mark -
#pragma mark Plugin Management
#pragma mark -
/*****************************************************************************
 * Install
 * -
 * This is invoked once when the plugin is loaded to do initial setup and
 * allow us to register with launchd. If UserEventAgent crashes, the plugin 
 * will need to be reloaded, and hence this will get invoked again.
 *****************************************************************************/
static void Install(void *instance)
{
	BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;
	
	plugin->_pluginContext = UserEventAgentRegisterForLaunchEvents(sPluginIdentifier, &ManageEventsCallback, plugin);
	
	if (!plugin->_pluginContext)
	{
		fprintf(stderr, "%s: failed to register for launch events.\n", sPluginIdentifier);
		return;
	}
	
}

/*****************************************************************************
 * ManageEventsCallback
 * - 
 * This is invoked when launchd loads a event dictionary and needs to inform 
 * us what a daemon / agent is looking for.
 *****************************************************************************/
static void ManageEventsCallback(UserEventAgentLaunchdAction action, CFNumberRef token, CFTypeRef eventMatchDict, void* vContext)
{
	
	if (!eventMatchDict || CFGetTypeID(eventMatchDict) != CFDictionaryGetTypeID())
	{
		fprintf(stderr, "%s given non-dictionary for event dictionary\n", sPluginIdentifier);
		return;
	}
	
	if (action == kUserEventAgentLaunchdAdd)
	{
		// Launchd wants us to add a launch event for this token and matching dictionary.
		AddEventToPlugin((BonjourUserEventsPlugin*)vContext, token, (CFDictionaryRef)eventMatchDict);
	}
	else if (action == kUserEventAgentLaunchdRemove)
	{
		// Launchd wants us to remove the event hook we setup for this token / matching dictionary.
		RemoveEventFromPlugin((BonjourUserEventsPlugin*)vContext, token);
	}
	else
	{
		fprintf(stderr, "%s got unknown UserEventAction: %d\n", sPluginIdentifier, action);
	}
}


#pragma mark -
#pragma mark Plugin Guts
#pragma mark -

/*****************************************************************************
 * AddEventToPlugin
 * - 
 * This method is invoked when launchd wishes the plugin to setup a launch 
 * event matching the parameters in the dictionary.
 *****************************************************************************/
void AddEventToPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchdToken, CFDictionaryRef eventParameters)
{
	CFStringRef		domain = CFDictionaryGetValue(eventParameters, sServiceDomainKey);
	CFStringRef		type = CFDictionaryGetValue(eventParameters, sServiceTypeKey);
	CFStringRef		name = CFDictionaryGetValue(eventParameters, sServiceNameKey);
	CFBooleanRef	cfOnAdd = CFDictionaryGetValue(eventParameters, sOnServiceAddKey);
	CFBooleanRef	cfOnRemove = CFDictionaryGetValue(eventParameters, sOnServiceRemoveKey);
	CFBooleanRef    cfWhileSericeExists = CFDictionaryGetValue(eventParameters, sWhileServiceExistsKey);
	
	Boolean			onAdd = false;
	Boolean			onRemove = false;
	Boolean			whileExists = false;
	
	if (cfOnAdd && CFGetTypeID(cfOnRemove) == CFBooleanGetTypeID() && CFBooleanGetValue(cfOnAdd))
		onAdd = true;
	
	if (cfOnRemove && CFGetTypeID(cfOnRemove) == CFBooleanGetTypeID() && CFBooleanGetValue(cfOnRemove))
		onRemove = true;
	
	if (cfWhileSericeExists && CFGetTypeID(cfWhileSericeExists) == CFBooleanGetTypeID() && CFBooleanGetValue(cfWhileSericeExists))
		whileExists = true;
	
	// A type is required. If none is specified, BAIL
	if (!type || CFGetTypeID(type) != CFStringGetTypeID()) 
	{
		fprintf(stderr, "%s, a LaunchEvent is missing a service type.\n", sPluginIdentifier);
		return;
	}
	
	// If we aren't suppose to launch on services appearing or disappearing, this service does nothing. Ignore.
	if ((!onAdd && !onRemove && !whileExists) || (onAdd && onRemove && whileExists))
	{
		fprintf(stderr, "%s, a LaunchEvent is missing both onAdd/onRemove/existance or has both.\n", sPluginIdentifier);
		return;
	}

	// If no domain is specified, assume local.
	if (!domain)
	{
		domain = CFSTR("local"); 
	}
	else if (CFGetTypeID(domain) != CFStringGetTypeID() ) // If the domain is not a string, fai;
	{
		fprintf(stderr, "%s, a LaunchEvent has a domain that is not a string.\n", sPluginIdentifier);
		return;
	}

	
	// If we have a name filter, but it's not a string. This event it broken, bail.
	if (name && CFGetTypeID(name) != CFStringGetTypeID())
	{
		fprintf(stderr, "%s, a LaunchEvent has a domain that is not a string.\n", sPluginIdentifier);
		return;
	}
	
	// Get us a browser
	NetBrowserInfo* browser = CreateBrowserForTypeAndDomain(plugin, type, domain);
	
	if (!browser)
	{
		fprintf(stderr, "%s, a LaunchEvent has a domain that is not a string.\n", sPluginIdentifier);
		return;
	}
	
	// Create Event Dictionary
	CFMutableDictionaryRef eventDictionary = CFDictionaryCreateMutable(NULL, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	
	CFDictionarySetValue(eventDictionary, sLaunchdTokenKey, launchdToken);
	
	if (name)
		CFDictionarySetValue(eventDictionary, sServiceNameKey, name);
	
	// Add to the correct dictionary.
	if (onAdd)
		AddEventDictionary(eventDictionary, plugin->_onAddEvents, browser);
	
	if (onRemove)
		AddEventDictionary(eventDictionary, plugin->_onRemoveEvents, browser);
	
	if (whileExists)
		AddEventDictionary(eventDictionary, plugin->_whileServiceExist, browser);
	
	// Add Token Mapping
	CFDictionarySetValue(plugin->_tokenToBrowserMap, launchdToken, browser);
	
	// Release Memory
	CFRelease(eventDictionary);
	NetBrowserInfoRelease(NULL, browser);
	
}



/*****************************************************************************
 * RemoveEventFromPlugin
 * - 
 * This method is invoked when launchd wishes the plugin to setup a launch 
 * event matching the parameters in the dictionary.
 *****************************************************************************/
void RemoveEventFromPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef	launchdToken)
{
	NetBrowserInfo* browser = (NetBrowserInfo*)CFDictionaryGetValue(plugin->_tokenToBrowserMap, launchdToken);
	Boolean othersUsingBrowser = false;
	
	if (!browser)
	{
		long long value = 0;
		CFNumberGetValue(launchdToken, kCFNumberLongLongType, &value);
		fprintf(stderr, "%s, Launchd asked us to remove a token we did not register!\nToken:%lld\n", sPluginIdentifier, value);
		return;
	}
	
	CFMutableArrayRef onAddEvents = (CFMutableArrayRef)CFDictionaryGetValue(plugin->_onAddEvents, browser);
	CFMutableArrayRef onRemoveEvents = (CFMutableArrayRef)CFDictionaryGetValue(plugin->_onRemoveEvents, browser);
	
	if (onAddEvents)
	{
		RemoveEventFromArray(onAddEvents, launchdToken);
				
		// Is the array now empty, clean up 
		if (CFArrayGetCount(onAddEvents) == 0)
			CFDictionaryRemoveValue(plugin->_onAddEvents, browser);
	}

	if (onRemoveEvents)
	{
		RemoveEventFromArray(onRemoveEvents, launchdToken);
		
		// Is the array now empty, clean up 
		if (CFArrayGetCount(onRemoveEvents) == 0)
			CFDictionaryRemoveValue(plugin->_onRemoveEvents, browser);
	}
	
	// Remove ourselves from the token dictionary.
	CFDictionaryRemoveValue(plugin->_tokenToBrowserMap, launchdToken);
	
	// Check to see if anyone else is using this browser.
	CFIndex i;
	CFIndex count = CFDictionaryGetCount(plugin->_tokenToBrowserMap);
	NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));

	// Fetch the values of the token dictionary
	CFDictionaryGetKeysAndValues(plugin->_tokenToBrowserMap, NULL, (const void**)browsers);

	for (i = 0; i < count; ++i)
	{
		if (NetBrowserInfoEqual(browsers[i], browser))
		{
			othersUsingBrowser = true;
			break;
		}
	}
	
	// If no one else is useing our browser, clean up!
	if (!othersUsingBrowser)
	{
		CFDictionaryRemoveValue(plugin->_tokenToBrowserMap, launchdToken); // This triggers release and dealloc of the browser
	}
	
	free(browsers);
}


/*****************************************************************************
 * CreateBrowserForTypeAndDomain
 * - 
 * This method returns a NetBrowserInfo that is looking for a type of 
 * service in a domain. If no browser exists, it will create one and return it.
 *****************************************************************************/
NetBrowserInfo* CreateBrowserForTypeAndDomain(BonjourUserEventsPlugin* plugin, CFStringRef type, CFStringRef domain)
{
	CFIndex i;
	CFIndex count = CFDictionaryGetCount(plugin->_browsers);
	NetBrowserInfo* browser = NULL;
	CFDictionaryRef* dicts = malloc(count * sizeof(CFDictionaryRef));
	NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));
	
	// Fetch the values of the browser dictionary
	CFDictionaryGetKeysAndValues(plugin->_browsers, (const void**)browsers, (const void**)dicts);
	
	// Loop thru the browsers list and see if we can find a matching one.
	for (i = 0; i < count; ++i)
	{
		CFDictionaryRef browserDict = dicts[i];

		CFStringRef browserType = CFDictionaryGetValue(browserDict, sServiceTypeKey);
		CFStringRef browserDomain = CFDictionaryGetValue(browserDict, sServiceDomainKey);
		
		// If we have a matching browser, break
		if (CFStringCompare(browserType, type, kCFCompareCaseInsensitive) &&
			CFStringCompare(browserDomain, domain, kCFCompareCaseInsensitive))
		{
			browser = browsers[i];
			NetBrowserInfoRetain(NULL, browser);
			break;
		}
	}
	
	// No match found, lets create one!
	if (!browser)
	{
		
		browser = NetBrowserInfoCreate(type, domain, plugin);
		
		if (!browser)
		{			
			fprintf(stderr, "%s, failed to search for %s.%s", sPluginIdentifier, CStringFromCFString(type) , CStringFromCFString(domain));
			free(dicts);
			free(browsers);
			return NULL;
		}
		
		// Service browser created, lets add this to ourselves to the dictionary.
		CFMutableDictionaryRef browserDict = CFDictionaryCreateMutable(NULL, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		
		CFDictionarySetValue(browserDict, sServiceTypeKey, type);
		CFDictionarySetValue(browserDict, sServiceDomainKey, domain);
		
		// Add the dictionary to the browsers dictionary.
		CFDictionarySetValue(plugin->_browsers, browser, browserDict);
		
		// Release Memory
		CFRelease(browserDict);
	}
	
	free(dicts);
	free(browsers);
	
	return browser;
}

/*****************************************************************************
 * BrowserForSDRef
 * - 
 * This method returns a NetBrowserInfo that matches the calling SDRef passed
 * in via the callback.
 *****************************************************************************/
NetBrowserInfo* BrowserForSDRef(BonjourUserEventsPlugin* plugin, DNSServiceRef sdRef)
{
	CFIndex i;
	CFIndex count = CFDictionaryGetCount(plugin->_browsers);
	NetBrowserInfo* browser = NULL;
	NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));
	
	// Fetch the values of the browser dictionary
	CFDictionaryGetKeysAndValues(plugin->_browsers, (const void**)browsers, NULL);
	
	// Loop thru the browsers list and see if we can find a matching one.
	for (i = 0; i < count; ++i)
	{
		NetBrowserInfo* currentBrowser = browsers[i];
		
		if (currentBrowser->browserRef == sdRef)
		{
			browser = currentBrowser;
			break;
		}
	}
	
	
	free(browsers);

	return browser;
}

/*****************************************************************************
 * AddEventDictionary
 * - 
 * Adds a event to a browser's event dictionary
 *****************************************************************************/

void AddEventDictionary(CFDictionaryRef eventDict, CFMutableDictionaryRef allEventsDictionary, NetBrowserInfo* key)
{
	CFMutableArrayRef eventsForBrowser = (CFMutableArrayRef)CFDictionaryGetValue(allEventsDictionary, key);
	
	if (!eventsForBrowser) // We have no events for this browser yet, lets add him.
	{
		eventsForBrowser = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFDictionarySetValue(allEventsDictionary, key, eventsForBrowser); 
	}
	else 
	{
		CFRetain(eventsForBrowser);
	}

	CFArrayAppendValue(eventsForBrowser, eventDict);
	CFRelease(eventsForBrowser);
}

/*****************************************************************************
 * RemoveEventFromArray
 * - 
 * Searches a Array of Event Dictionaries to find one with a matching launchd
 * token and remove it.
 *****************************************************************************/

void RemoveEventFromArray(CFMutableArrayRef array, CFNumberRef launchdToken)
{
	CFIndex i;
	CFIndex count = CFArrayGetCount(array);
	// Loop thru looking for us.
	for (i = 0; i < count; )
	{
		CFDictionaryRef eventDict = CFArrayGetValueAtIndex(array, i);
		CFNumberRef token = CFDictionaryGetValue(eventDict, sLaunchdTokenKey);
		
		if (CFEqual(token, launchdToken)) // This is the same event?
		{
			CFArrayRemoveValueAtIndex(array, i);	// Remove the event,
			break; // The token should only exist once, so it make no sense to continue.
		}
		else
		{
			++i; // If it's not us, advance.
		}
	}	
}

#pragma mark -
#pragma mark Net Service Browser Stuff
#pragma mark -

/*****************************************************************************
 * ServiceBrowserCallback
 * - 
 * This method is the heart of the plugin. It's the runloop callback annoucing 
 * the appearence and disappearance of network services.
 *****************************************************************************/

void ServiceBrowserCallback (DNSServiceRef				sdRef,
							 DNSServiceFlags            flags,
							 uint32_t                   interfaceIndex,
							 DNSServiceErrorType        errorCode,
							 const char*                serviceName,
							 const char*                regtype,
							 const char*                replyDomain,
							 void*                      context )
{
	(void)interfaceIndex;
	(void)regtype;
	(void)replyDomain;
	BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)context;
	NetBrowserInfo* browser = BrowserForSDRef(plugin, sdRef);
	
	if (!browser) // Missing browser?
		return;
	
	if (errorCode != kDNSServiceErr_NoError)
		return;
	
	CFStringRef cfServiceName = CFStringCreateWithCString(NULL, serviceName, kCFStringEncodingUTF8);
	
	if (flags & kDNSServiceFlagsAdd)
	{
		HandleTemporaryEventsForService(plugin, browser, cfServiceName, plugin->_onAddEvents);
		HandleStateEventsForService(plugin, browser, cfServiceName, true);
	}
	else 
	{
		HandleTemporaryEventsForService(plugin, browser, cfServiceName, plugin->_onRemoveEvents);
		HandleStateEventsForService(plugin, browser, cfServiceName, false);
	}

	CFRelease(cfServiceName);
}

/*****************************************************************************
 * HandleTemporaryEventsForService
 * - 
 * This method handles the firing of one shot events. Aka. Events that are 
 * signaled when a service appears / disappears. They have a temporarly 
 * signaled state. 
 *****************************************************************************/
void HandleTemporaryEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser, CFStringRef serviceName, CFMutableDictionaryRef eventsDictionary)
{
	CFArrayRef events = (CFArrayRef)CFDictionaryGetValue(eventsDictionary, browser); // Get events for the browser we passed in.
	CFIndex i;
	CFIndex count;
	
	if (!events)  // Somehow we have a orphan browser...
		return;
	
	count = CFArrayGetCount(events);
	
	// Go thru the events and run filters, notifity if they pass.
	for (i = 0; i < count; ++i)
	{
		CFDictionaryRef eventDict = (CFDictionaryRef)CFArrayGetValueAtIndex(events, i);
		CFStringRef eventServiceName = (CFStringRef)CFDictionaryGetValue(eventDict, sServiceNameKey);
		CFNumberRef token = (CFNumberRef) CFDictionaryGetValue(eventDict, sLaunchdTokenKey);
				
		// Currently we only filter on service name, that makes this as simple as... 
		if (!eventServiceName || CFEqual(serviceName, eventServiceName))
		{
			// Create Context Info
			CFRunLoopTimerContext context; 
			TimerContextInfo* info = TimerContextInfoCreate(plugin, token);

			context.version = 0;
			context.info = info;
			context.retain = TimerContextInfoRetain;
			context.release = TimerContextInfoRelease;
			context.copyDescription = TimerContextInfoCopyDescription;
			
			// Create and add one shot timer to flip the event off after a second
			CFRunLoopTimerRef timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 1.0, 0, 0, 0, TemporaryEventTimerCallout, &context);
			CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes);
			
			// Signal Event
			UserEventAgentSetLaunchEventState(plugin->_pluginContext, token, true);
			
			// Clean Up
			TimerContextInfoRelease(info);
			CFRelease(timer);
			
		}
	}
	
}

/*****************************************************************************
 * HandleStateEventsForService
 * - 
 * This method handles the toggling the state of a while exists event to 
 * reflect the network. 
 *****************************************************************************/
void HandleStateEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser, CFStringRef serviceName, Boolean didAppear)
{
	CFArrayRef events = (CFArrayRef)CFDictionaryGetValue(plugin->_whileServiceExist, browser); // Get the _whileServiceExist events that are interested in this browser.
	CFIndex i;
	CFIndex count;
	
	if (!events)  // Somehow we have a orphan browser...
		return;
	
	count = CFArrayGetCount(events);
	
	// Go thru the events and run filters, notifity if they pass.
	for (i = 0; i < count; ++i)
	{
		CFDictionaryRef eventDict = (CFDictionaryRef)CFArrayGetValueAtIndex(events, i);
		CFStringRef eventServiceName = (CFStringRef)CFDictionaryGetValue(eventDict, sServiceNameKey);
		CFNumberRef token = (CFNumberRef) CFDictionaryGetValue(eventDict, sLaunchdTokenKey);
		
		// Currently we only filter on service name, that makes this as simple as... 
		if (!eventServiceName || CFEqual(serviceName, eventServiceName))
			UserEventAgentSetLaunchEventState(plugin->_pluginContext, token, didAppear);
	}
}

/*****************************************************************************
 * TemporaryEventTimerCallout
 * - 
 * This method is invoked a second after a watched service appears / disappears
 * to toggle the state of the launch event back to false.
 *****************************************************************************/
void TemporaryEventTimerCallout ( CFRunLoopTimerRef timer, void *info )
{
	TimerContextInfo* contextInfo = (TimerContextInfo*)info;

	UserEventAgentSetLaunchEventState(contextInfo->plugin->_pluginContext, contextInfo->token, false);
	
	// Remove from pending timers array.
	CFIndex i;
	CFIndex count = CFArrayGetCount(contextInfo->plugin->_timers);
	
	for (i = 0; i < count; ++i)
	{
		CFRunLoopTimerRef item = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(contextInfo->plugin->_timers, i);
		
		if (item == timer)
			break;
	}
	
	if (i != count)
		CFArrayRemoveValueAtIndex(contextInfo->plugin->_timers, i);
}

#pragma mark -
#pragma mark Convenence
#pragma mark -

/*****************************************************************************
 * CStringFromCFString
 * - 
 * Silly convenence function for dealing with non-critical CFSTR -> cStr
 * conversions.
 *****************************************************************************/

const char* CStringFromCFString(CFStringRef string)
{
	const char* defaultString = "??????";
	const char* cstring;

	if (!string)
		return defaultString;
	
	cstring = CFStringGetCStringPtr(string, kCFStringEncodingUTF8);
	
	return (cstring) ? cstring : defaultString;
	
}

#pragma mark -
#pragma mark TimerContextInfo "Object"
#pragma mark -

/*****************************************************************************
 * TimerContextInfoCreate
 * - 
 * Convenence for creating TimerContextInfo pseudo-objects
 *****************************************************************************/
TimerContextInfo* TimerContextInfoCreate(BonjourUserEventsPlugin* plugin, CFNumberRef token)
{
	TimerContextInfo* info = malloc(sizeof(TimerContextInfo));
	
	info->refCount = 1;
	info->plugin = plugin;
	info->token = (CFNumberRef)CFRetain(token);
	
	return info;
}

/*****************************************************************************
 * TimerContextInfoRetain
 * - 
 * Convenence for retaining TimerContextInfo pseudo-objects
 *****************************************************************************/
const void* TimerContextInfoRetain(const void* info)
{
	TimerContextInfo* context = (TimerContextInfo*)info;
	
	if (!context)
		return NULL;
	
	++context->refCount;
	
	return context;
}

/*****************************************************************************
 * TimerContextInfoRelease
 * - 
 * Convenence for releasing TimerContextInfo pseudo-objects
 *****************************************************************************/
void TimerContextInfoRelease(const void* info)
{
	TimerContextInfo* context = (TimerContextInfo*)info;
	
	if (!context)
		return;
	
	if (context->refCount == 1)
	{
		CFRelease(context->token);
		free(context);
		return;
	}
	else 
	{
		--context->refCount;
	}
}

/*****************************************************************************
 * TimerContextInfoCopyDescription
 * - 
 * This method actually does nothing, but is just a stub so CF is happy.
 *****************************************************************************/
CFStringRef TimerContextInfoCopyDescription(const void* info)
{
	(void)info;
	return CFStringCreateWithCString(NULL, "TimerContextInfo: No useful description", kCFStringEncodingUTF8);
}


#pragma mark -
#pragma mark NetBrowserInfo "Object"
#pragma mark -
/*****************************************************************************
 * NetBrowserInfoCreate
 * - 
 * The method creates a NetBrowserInfo Object and initalizes it.
 *****************************************************************************/
NetBrowserInfo* NetBrowserInfoCreate(CFStringRef serviceType, CFStringRef domain, void* context)
{
	NetBrowserInfo* outObj = NULL;
	DNSServiceRef browserRef = NULL;
	char* cServiceType = NULL;
	char* cDomain = NULL;
	Boolean success = true;
	
	CFIndex serviceSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(serviceType), kCFStringEncodingUTF8);
	cServiceType = calloc(serviceSize, 1);
	success = CFStringGetCString(serviceType, cServiceType, serviceSize, kCFStringEncodingUTF8);
	
	if (domain)
	{
		CFIndex domainSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(domain), kCFStringEncodingUTF8);
		cDomain = calloc(serviceSize, 1);
		success = success && CFStringGetCString(domain, cDomain, domainSize, kCFStringEncodingUTF8);
	}
	
	if (!success)
	{
		fprintf(stderr, "LaunchEvent has badly encoded service type or domain.\n");
		free(cServiceType);

		if (cDomain)
			free(cDomain);
		
		return NULL;
	}
	
	DNSServiceErrorType err = DNSServiceBrowse(&browserRef, 0, 0, cServiceType, cDomain, ServiceBrowserCallback, context);

	if (err != kDNSServiceErr_NoError)
	{
		fprintf(stderr, "Failed to create browser for %s, %s\n", cServiceType, cDomain);
		free(cServiceType);
		
		if (cDomain)
			free(cDomain);
		
		return NULL;
	}

	DNSServiceSetDispatchQueue(browserRef, dispatch_get_main_queue());
	
	
	outObj = malloc(sizeof(NetBrowserInfo));
	
	outObj->refCount = 1;
	outObj->browserRef = browserRef;

	free(cServiceType);
								  
	if (cDomain)
		free(cDomain);
	
	return outObj;
}

/*****************************************************************************
 * NetBrowserInfoRetain
 * - 
 * The method retains a NetBrowserInfo object.
 *****************************************************************************/
const void* NetBrowserInfoRetain(CFAllocatorRef allocator, const void* info)
{
	(void)allocator;
	NetBrowserInfo* obj = (NetBrowserInfo*)info;
	
	if (!obj) 
		return NULL;
	
	++obj->refCount;
	
	return obj;
}

/*****************************************************************************
 * NetBrowserInfoRelease
 * - 
 * The method releases a NetBrowserInfo object.
 *****************************************************************************/
void NetBrowserInfoRelease(CFAllocatorRef allocator, const void* info)
{
	(void)allocator;
	NetBrowserInfo* obj = (NetBrowserInfo*)info;
	
	if (!obj)
		return;
	
	if (obj->refCount == 1)
	{
		DNSServiceRefDeallocate(obj->browserRef);
		free(obj);
	}
	else 
	{
		--obj->refCount;
	}

}

/*****************************************************************************
 * NetBrowserInfoEqual
 * - 
 * The method is used to compare two NetBrowserInfo objects for equality.
 *****************************************************************************/
Boolean	NetBrowserInfoEqual(const void *value1, const void *value2)
{
	NetBrowserInfo* obj1 = (NetBrowserInfo*)value1;
	NetBrowserInfo* obj2 = (NetBrowserInfo*)value2;
	
	if (obj1->browserRef == obj2->browserRef)
		return true;
	
	return false;
}

/*****************************************************************************
 * NetBrowserInfoHash
 * - 
 * The method is used to make a hash for the object. We can cheat and use the 
 * browser pointer.
 *****************************************************************************/
CFHashCode	NetBrowserInfoHash(const void *value)
{
	return (CFHashCode)((NetBrowserInfo*)value)->browserRef;
}


/*****************************************************************************
 * NetBrowserInfoCopyDescription
 * - 
 * Make CF happy.
 *****************************************************************************/
CFStringRef	NetBrowserInfoCopyDescription(const void *value)
{
	(void)value;
	return CFStringCreateWithCString(NULL, "NetBrowserInfo: No useful description", kCFStringEncodingUTF8);
}
