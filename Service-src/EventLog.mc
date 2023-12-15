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

LanguageNames=(English=0x0409:MSG00409)

; // The following are message definitions.

MessageId=1
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_START_INIT
Language=English
GeoIP Service is starting
.
MessageId=2
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_RUNNING
Language=English
GeoIP Service is up and running
.
MessageId=3
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_SHUTDOWN
Language=English
GeoIP Service is shutting down
.
MessageId=4
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_DOWN
Language=English
GeoIP Service is down
.
MessageId=5
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_SERVICE_CONTROL_STOP
Language=English
Service Control Manager has requested the GeoIP Service to stop
.
MessageId=6
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_NO_SERVICE_CTRL_HANDLER
Language=English
Unable to register the GeoIP Service control handler
.
MessageId=7
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_NO_SERVICE_STOP_EVENT
Language=English
Unable to create GeoIP Service internal synchronization event object
.
MessageId=8
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_DB_FILES_INIT_ERR
Language=English
Unable to open and/or read database files
.
MessageId=9
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_REGISTRY_OPTIONS_MISCONFIG
Language=English
GeoIP Service is misconfigured or configuration data in the registry is damaged. Check the Service settings and restart
.
MessageId=10
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_PIPESRV_INIT_DONE
Language=English
PIPE Server for PIPE %2 has started, thread ID is %1
.
MessageId=11
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_TCPSRV_INIT_DONE
Language=English
TCP Server on binded IP %2 has started, thread ID is %1, shutdown thread ID is %3
.
MessageId=12
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_PIPESRV_INIT_ERROR
Language=English
Error while initializing PIPE Server
.
MessageId=13
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_TCPSRV_INIT_ERROR
Language=English
Error while initializing TCP Server
.
MessageId=14
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_PIPESRV_SHUTDOWN
Language=English
PIPE Server for PIPE %2, thread ID %1, is down
.
MessageId=15
Severity=Informational
Facility=Runtime
SymbolicName=ELMSG_TCPSRV_SHUTDOWN
Language=English
TCP Server on binded IP %2, thread ID %1, is down
.
MessageId=16
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_PIPESRV_SHUTDOWN_ERROR
Language=English
PIPE Server for PIPE %2, thread ID %1, has unexpectedly encountered a problem, error code %3, and has been shut down
.
MessageId=17
Severity=Error
Facility=Runtime
SymbolicName=ELMSG_TCPSRV_SHUTDOWN_ERROR
Language=English
TCP Server on binded IP %2, thread ID %1, has unexpectedly encountered a problem, error code %3, and has been shut down
.
MessageId=18
Severity=Warning
Facility=Runtime
SymbolicName=ELMSG_PIPEWORKER_SHUTDOWN_ERROR
Language=English
An unexpected error has occurred while processing the client request with PIPE Server for PIPE %1, thread ID %2, error code %3
.
MessageId=19
Severity=Warning
Facility=Runtime
SymbolicName=ELMSG_TCPWORKER_SHUTDOWN_ERROR
Language=English
An unexpected error has occurred while processing the client request with TCP Server on binded IP %1, thread ID %2, error code %3
.
MessageId=20
Severity=Warning
Facility=Runtime
SymbolicName=ELMSG_SERVICE_CRITICAL_THREAD_STOPPED
Language=English
Critical thread has failed, GeoIP Service is shutting down
.

; // A message file must end with a period on its own line
; // followed by a blank line.
