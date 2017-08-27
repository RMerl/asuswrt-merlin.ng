Building the Bonjour Firefox Extension on Windows

There is a Visual Studio 2005 project file that will build the extension correctly as long as the Visual Studio environment is setup correctly.  This code was built against the 1.9 version of the XULRunner SDK.  The Visual Studio environment should be modified to add the include and lib paths of the XULRunner SDK.  Specifically, the following include paths should be added to VC++ include directories:

…\xulrunner-sdk\include\xpcom
…\xulrunner-sdk\include\nspr
…\xulrunner-sdk\include\string
…\xulrunner-sdk\include\pref
…\xulrunner-sdk\sdk\include

The following path should be added to VC++ lib directories:

…\xulrunner-sdk\lib

After the code has been built, it can be installed like any other Firefox extension.  Please consult Firefox extension documentation for more information on how to package and install Firefox extensions.
