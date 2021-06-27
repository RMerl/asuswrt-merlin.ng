MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
    )


FacilityNames=(System=0x0:FACILITY_SYSTEM
    Runtime=0x2:FACILITY_RUNTIME
    Stubs=0x3:FACILITY_STUBS
    Io=0x4:FACILITY_IO_ERROR_CODE
)

LanguageNames=(English=0x409:MSG00409)

; // The following are message definitions.

MessageId=0x1
Severity=Error
Facility=Runtime
SymbolicName=SVC_EMERGENCY
Language=English
Emergency: %1.
.

MessageId=0x2
Severity=Error
Facility=Runtime
SymbolicName=SVC_ALERT
Language=English
Alert: %1.
.

MessageId=0x3
Severity=Error
Facility=Runtime
SymbolicName=SVC_CRITICAL
Language=English
Critical: %1.
.

MessageId=0x4
Severity=Error
Facility=Runtime
SymbolicName=SVC_ERROR
Language=English
Error: %1.
.

MessageId=0x5
Severity=Warning
Facility=Runtime
SymbolicName=SVC_WARNING
Language=English
Warning: %1.
.

MessageId=0x6
Severity=Warning
Facility=Runtime
SymbolicName=SVC_NOTICE
Language=English
Notice: %1.
.

MessageId=0x7
Severity=Informational
Facility=Runtime
SymbolicName=SVC_INFO
Language=English
Info: %1.
.

MessageId=0x8
Severity=Informational
Facility=Runtime
SymbolicName=SVC_DEBUG
Language=English
Debug: %1.
.

; // A message file must end with a period on its own line
; // followed by a blank line.
