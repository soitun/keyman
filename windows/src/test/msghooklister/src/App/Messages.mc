MessageIdTypedef=DWORD

SeverityNames=(
	Success=0x0:MESSAGE_SEVERITY_SUCCESS
	Informational=0x1:MESSAGE_SEVERITY_INFORMATION
	Warning=0x2:MESSAGE_SEVERITY_WARNING
	Error=0x3:MESSAGE_SEVERITY_ERROR
)

FacilityNames=(Info=0x0:FACILITY_INFO
    Runtime=0x2:FACILITY_RUNTIME
    Install=0x3:FACILITY_INSTALL
    Io=0x4:FACILITY_IO_ERROR_CODE
)

LanguageNames=(English=0x409:MSG00409)

;#include "SimpleTraits.h"
;
;// retrieves the messageId string from hMod, or the system if hMod is NULL
;template<class Args>
;BOOL GetModuleMessage(HMODULE hMod, DWORD messageId, std::wstring& errorText, Args* args)
;{
;	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER;
;	if(!args)
;	{
;		flags |= FORMAT_MESSAGE_IGNORE_INSERTS;
;	}
;	else if(is_same<remove_pointer<Args>::type, DWORD_PTR>::value)
;	{
;		flags |= FORMAT_MESSAGE_ARGUMENT_ARRAY;
;	}
;	flags |= hMod ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM;
;	WCHAR* error;
;	if(FormatMessageW(
;			flags,
;			hMod,
;			messageId,
;			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
;			reinterpret_cast<WCHAR*>(&error),
;			0, reinterpret_cast<va_list*>(args)
;		)
;	)
;	{
;		errorText = error;
;		LocalFree(error);
;	}
;	return !!error;
;}
;
;inline BOOL GetModuleMessage(HMODULE hMod, DWORD messageId, std::wstring& errorText)
;{
;	return GetModuleMessage(hMod, messageId, errorText, (va_list*)NULL);
;}
;
;// Prototype for function that will display the messages in the console
;void __cdecl DisplayMessage(DWORD message, ...);
;
;// get a string from the message table
;std::wstring GetModuleString(DWORD message);

MessageId=0x1
Severity=Informational
Facility=Runtime
SymbolicName=MSG_USAGE
Language=English
Lists the messages pending in a window's message queue or installed hooks.

MsgListerApp [/I | /U | /L | /H]
MsgListerApp HWND

/I    Installs the application, requires administrator permissions
/U    Uninstalls the application, also requires admin permissions
/L    Displays a list of active windows HWNDs and titles
/H    List all currently active hooks across all sessions

HWND  Lists pending messages and other info for the window 
	  represented by HWND
.

MessageId=0x2
Severity=Error
Facility=Runtime
SymbolicName=MSG_NEEDS_USER_ADMIN
Language=English
You must be a member of the Administrators group to install and
uninstall this software. Rerun the program with elevated permissions.
.

MessageId=0x3
Severity=Error
Facility=Runtime
SymbolicName=MSG_INVALID_WINDOW
Language=English
%1!s! is an invalid window value.
.

MessageId=0x4
Severity=Error
Facility=Runtime
SymbolicName=MSG_NEEDS_NATIVE_VERSION
Language=English
64-bit version of the package is required on 64-bit computers.
The 32-bit version will not work correctly.
.

MessageId=0x5
Severity=Error
Facility=Install
SymbolicName=MSG_GENERIC_INSTALL_FAILURE
Language=English
Installation failed: %1!s!
.

MessageId=0x6
Severity=Error
Facility=Runtime
SymbolicName=MSG_CANT_OPEN_DRIVER
Language=English
Couldn't open driver handle because of error:
%1!s!
.

MessageId=0x7
Severity=Error
Facility=Runtime
SymbolicName=MSG_CANT_COPY_FILE
Language=English
Couldn't copy %1!s! to %2!s! because of error:
%3!s!
.

MessageId=0x8
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_CREATE_PARAM_KEY
Language=English
Failed to create registry key HKLM\%1!s! because of error:
%2!s!%.
.

MessageId=0x9
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_DELETE_SERVICE
Language=English
Failed to delete %1!s! service because of error %2!s!
The driver file at %3!s! may not have been deleted.
.

MessageId=0xa
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_LOAD_DBGHELP_FUNCS
Language=English
Couldn't find required functions in dbghelp.dll. Ensure the 
file is in the same directory as this program and its version
is above 6.0.0.0.
.

MessageId=0xb
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_LOAD_SYMBOL_INFO
Language=English
Unable to retrieve required information from the symbol file.
Error returned by %1!s! was %2!s!
.

MessageId=0xc
Severity=Warning
Facility=Runtime
SymbolicName=MSG_WIN32K_VERSION_CHANGE
Language=English
The win32k.sys file has been changed since you installed
the program. Please re-install this application to
continue using it.
.

MessageId=0xd
Severity=Success
Facility=Runtime
SymbolicName=MSG_OPERATION_SUCCESS
Language=English
Operation was successfully completed.
.

MessageId=0xe
Severity=Error
Facility=Runtime
SymbolicName=MSG_CANT_CHECK_WIN32K
Language=English
Can't open win32k.sys or registry key %1!s!
.

MessageId=0xf
Severity=Error
Facility=Runtime
SymbolicName=MSG_DRIVER_COMMUNICATION_FAILURE
Language=English
Driver command %1!s! failed with error %2!s!
.

MessageId=0x10
Severity=Error
Facility=Runtime
SymbolicName=MSG_GENERIC_OPERATION_FAILURE
Language=English
Operation failed: %1!s!
.

MessageId=0x11
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_CREATE_SERVICE
Language=English
Failed to create %1!s! service. Error %2!s!
.

MessageId=0x12
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_OPEN_SCM
Language=English
Failed to open the Service Control Manager. Error %1!s!
.

MessageId=0x13
Severity=Error
Facility=Install
SymbolicName=MSG_CANT_DELETE_FILE
Language=English
Couldn't delete the driver file at %1!s!. Error %2!s!
.

MessageId=0x14
Severity=Error
Facility=Install
SymbolicName=MSG_PLATFORM_NOT_SUPPORTED
Language=English
Only AMD64 and X86 platforms are supported.
.

MessageId=0x15
Severity=Error
Facility=Runtime
SymbolicName=MSG_CANT_LOAD_DLL
Language=English
Unable to load %1!s!.
.

MessageId=0x16
Severity=Informational
Facility=Info
SymbolicName=MSG_GENERAL_MESSAGE_INFO
Language=English
Window: %1!s!
Message: 0x%2!x! (%3!s!), wParam = 0x%4!Ix!, lParam = 0x%5!Ix!
Time sent: 0x%6!x!
Mouse x: %7!ld!
Mouse y: %8!ld!
Posted: %9!d!
.

; // The blank line before the terminating dot is intentional
MessageId=0x17
Severity=Informational
Facility=Info
SymbolicName=MSG_POSTED_MESSAGE_INFO
Language=English
Extra message info: 0x%1!Ix!
Real mouse position: x = %2!ld!, y = %3!ld!

.

MessageId=0x18
Severity=Informational
Facility=Info
SymbolicName=MSG_SENT_MESSAGE_INFO_THREAD
Language=English
Sent by thread: %1!lu!%2!s!
.

MessageId=0x19
Severity=Informational
Facility=Info
SymbolicName=MSG_SENT_MESSAGE_INFO_FUNCTION
Language=English
Function details:
%1!s!
.

MessageId=0x1a
Severity=Informational
Facility=Info
SymbolicName=MSG_THREAD_STATE
Language=English
Information for thread Id: %1!lu!

Thread flags: %2!s!GetMessageExtraInfo(): 0x%3!Ix!
Pending operations: %4!s!
%5!lu! message(s) in the queue:
.

MessageId=0x1b
Severity=Error
Facility=Runtime
SymbolicName=MSG_WINDOW_DOESNT_HAVE_A_THREAD
Language=English
Window 0x%1!Ix! doesn't seem to have a thread association
.

MessageId=0x1c
Severity=Informational
Facility=Info
SymbolicName=MSG_ACTIVE_WINDOW_LIST
Language=English
Active window list:
.

MessageId=0x1d
Severity=Informational
Facility=Info
SymbolicName=MSG_HOOK_DISPLAY_HEADER
Language=English
Displaying %1!lu! hook(s):
.

MessageId=0x1e
Severity=Informational
Facility=Info
SymbolicName=MSG_HOOK_DISPLAY_INFO
Language=English
Hook Number %1!lu!
HHOOK: 0x%2!Ix!
Session: %3!lu!
WindowStation: %4!s!
Type: %5!s!
Dll Name: %6!s!
RVA to hook function: 0x%7!Ix!
SetWindowsHookEx thread: %8!lu! (%9!S!)
Hooked thread: %10!lu! (%11!S!)
Hook timeout: %12!lu!
Last time hung: %13!d!
Flags: %14!s!

.

MessageId=0x1e
Severity=Error
Facility=Runtime
SymbolicName=MSG_FAILED_HOOK_QUERY
Language=English
Failed to query hook information
.

MessageId=0x1f
Severity=Informational
Facility=Info
SymbolicName=MSG_NO_HOOKS_RETURNED
Language=English
No hook info was returned
.
