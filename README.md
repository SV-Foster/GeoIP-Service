# GeoIP Service

Are you looking for a reliable and efficient way to manage IP address data and perform critical database operations, like updates, without affecting your service availability and performance? If so, you might be interested in GeoIP Service that offers a comprehensive solution for your needs!

![Screenshot](Documents/screenshot%20001.png)
![Screenshot](Documents/screenshot%20002.png)

GeoIP Service is a background Windows Service that runs on your Windows Server or workstation and manages the GeoIP database, a global database that provides information about the geographic location, country, region, city, ASN and other details of any IPv4 or IPv6 address. By using GeoIP Service, you can easily and fast retrieve and update this information for your own purposes, such as analytics, security or marketing.

But that's not all. GeoIP Service also allows you to perform very important database files changes, such as for backup, restore, migration, or optimization, without requiring a restart or causing any client disconnects. This means that you can maintain 100% uptime and ensure the continuity and quality of your service, even when you need to make major changes to your database.

Moreover, GeoIP Service supports multiple protocols and standards, such as IPv4, IPv6 and Named PIPE, and provides Windows domain authentication and security support, so that you can communicate with your clients and servers in a secure and flexible way. Whether you need to connect through IPv4, IPv6 or use Named PIPE for inter-process communication, GeoIP Service can handle it all.

GeoIP Service is designed to be easy to install, configure and use, and comes with a user-friendly interface and documentation. You can also customize GeoIP Service to suit your specific needs and preferences.

Don't miss this opportunity to take your IP address management and database operations to the next level with GeoIP Service!


## Installation

To install GeoIP Service on Windows, you need to follow these steps:

1. Create the folders needed for the service. For example, a folder named `C:\Program Files\SV Foster's GeoIP Service\`, and copy the files of the service into it. Also create `C:\Program Files\SV Foster's GeoIP Service\Database\` folder and copy .mmdb database files into, the fresh new copy of them you can download from the MaxMind web site for free. The GeoIP Service uses two database files: `GeoLite2-City.mmdb` and `GeoLite2-ASN.mmdb`, which contain the IP address information and the autonomous system number information, respectively.
2. Open Command prompt window with administrator privileges. Navigate to the GeoIP Service folder with the `cd` command.
3. Run `maint.exe` utility with the `install` command and path to the copied service executable as an argument. This will register the service in the Windows registry and create a service name for it. For example, you can run the following command in a command prompt:

```
maint install "C:\Program Files\SV Foster's GeoIP Service\GeoipSVC.exe"
```
4. Check options in the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\SV Foster\GeoIP Service`, change values if needed. All options are  described below in this document.
5. Start the service with the `sc start GeoIPSVC` command. This will launch the service and make it run in the background. You can also use the Services Manager to start the service from the graphical user interface. GeoIP Service is configured to start automatically on every boot, no need to start it manually every time, only the first time, to allow you to tweak some options.
6. Check that the service is running with the `clgeoip.exe` command. This is a command-line tool that allows you to communicate with the service and get IP address information. For example, you can run the following command to get the information of the IP address 199.83.131.167:

```
clgeoip /IP 199.83.131.167
```

The command will return the country, region, city, latitude, longitude, and other details of the IP address, if available.

You have successfully installed and configured the GeoIP Service on Windows. You can now use the service to get IP address information and perform database files changes with ease and efficiency!


## Uninstallation

To completely remove GeoIP Service, you shuld stop the service and run `maint.exe` utility with the proper command:

1. Open Command prompt window with administrator privileges. Navigate to the GeoIP Service folder with the `cd` command.
2. Stop the service with the SC:
```
sc stop GeoipSVC
```
3. Run `maint.exe` utility with the uninstall command. This will deregister the service in the Windows registry and remove a service name from it. For example, you can run the following command in a command prompt:

```
maint uninstall
```
4. Delete all files and folders created earlier.


## Usage

### Hotplugging updated .mmdb files

You can update the database files on the GeoIP server without restart or client disconnects with the `clgeoip.exe` tool. The service uses two database files: `GeoLite2-City.mmdb` and `GeoLite2-ASN.mmdb`, which contain the IP address information and the autonomous system number information, respectively. The latest versions of these files are available on the MaxMind website for free.

To change the database files, first, copy new files into the \Database\ folder of your GeoIP Service, next, you need to use the `/Mode Hotplug` option and specify the names of the new files with the `/FileGeo` and `/FileASN` options. For example, run the following command to change the database files to `geo-20231201.mmdb` and `asn-20231201.mmdb`:

```
clgeoip /Mode Hotplug /FileGeo geo-20231201.mmdb /FileASN asn-20231201.mmdb
```

The command will update the database files with the GeoIP Service without affecting the service operation or causing any client disconnects!


### Command Line Client

Command Line Client retrieves an IP address information from the GeoIP Service and preforms basic maintenance

Usage: 
```
clgeoip /<switch 1> /<switch N> <switch parameter> /IP <X.X.X.X or Y:Y::Y>
```

Switches:
```
  /NoLogo       Don't print copyright logo
  /Mode         Operating mode, IP lookup request (default), server ping or database files hotplug
                Possible values: Request, RequestASN, Ping, Hotplug
  /IP           An IP address for lookup (version 4 or 6), this switch is required, if mode
                is set to Request or RequestASN
  /Transport    Communication mode, via TCPv4, TCPv6 or named PIPE. PIPE mode is set by default
                Possible values: TCP4, TCP6, PIPE
  /PipeName     Name of the PIPE, if changed from the default one
  /PIPETimeoutIO
                Sets PIPE send and receive timeout in seconds
  /NetServer    Name or IP address of the TCP server. localhost is default
  /NetPort      Port number of the TCP server. 28780 is default
  /NetTimeoutConnect
                Sets TCP/IP conncetion timeout in seconds
  /NetTimeoutIO Sets TCP/IP send and receive timeout in seconds
  /FileGeo      File name with Geo database for hotplug command
  /FileASN      File name with ASN database for hotplug command
```

Usage examples:
Retrieves an IP address information from the GeoIP Server located on the local host through the default named PIPE. Can be used to check the server is up and running:
```
  clgeoip /IP 199.83.131.167
```

Retrieves an IP address information from the GeoIP Server located on network server IP-SRV.lan, port 1999 through the TCP/IP version 6 protocol:
```
  clgeoip /Transport TCP6 /NetServer IP-SRV.lan /NetPort 1999 /IP 199.83.131.167
```  

Changes database files on the GeoIP server without restart or client disconnects. If succeed, new file names are registered in the registry. If failed, previous files will be continued to use. This command can't change the folder, where files are stored:
```
  clgeoip /Mode Hotplug /FileGeo geo-new.mmdb /FileASN asn-new.mmdb
``` 


### Event Log

GeoIP Service logs various events to the Windows Event Log, which can be viewed and managed using the Event Viewer. The events include information, warnings, and errors related to the service operation, such as the startup, shutdown, request, response, and exception events. The events are stored under the Application log in the Windows Logs menu.

To access the Event Viewer, you can use one of the following methods:
* Press the Windows key + R on your keyboard to open the run window, type in `eventvwr`, and click OK or hit ENTER
* Click on Start or press the Windows key on your keyboard, search for `Event Viewer`, and click on the top result or press ENTER
* Open a command prompt or a terminal, navigate to the folder where the platform tools package is located, and run the command `eventvwr`

To view the events for GeoIP Service, you can expand the `Windows Logs` menu, and then click on `Application`. The results pane will list individual events for the service. If you want to see more details about a specific event, you can click on the event in the results pane.

To sort the events by a specific criterion, such as the level, date, or source, you can click on the column header in the results pane. To filter the events by a specific criterion, such as the level, date, or keyword, you can right-click on the Application log, select Filter Current Log, and specify the filter options in the dialog box.


### Performing requests from your code/app

To use the GeoIP Service from your app, you need to send requests to the service using one of the supported transport protocols: TCP IPv4, TCP IPv6 or named PIPE. The requests and responses are binaries that follow a specific structure and rules. The structure and rules of the binaries are defined by the `Protocol.h` header file, which is included in the source code of the GeoIP Service and the clgeoip.exe program.

If you are developing in `C` or `C++` programming language, you can use the clgeoip.exe program as a source for how to perform requests from your code/app. The clgeoip.exe program is a command-line client that can communicate with the GeoIP Service using any of the supported protocols. The source code of the clgeoip.exe program is available in the `client-src` folder of the GeoIP Service source code.

If you are using any other programming language, you need to implement the protocol yourself, based on the Protocol.h file and the clgeoip code. You need to create and read binaries according to the protocol definition, and send and receive them using the appropriate protocol. You also need to handle any errors or exceptions that may occur during the communication. You can refer to the Protocol.h file and the clgeoip code for examples and explanations of the protocol elements and functions.


## Options

### Registry options

This section explains the registry options for the GeoIP Service. The GeoIP Service uses two database files, GeoLite2-ASN.mmdb and GeoLite2-City.mmdb, which are updated monthly by MaxMind. The GeoIP Service can be accessed by clients using TCP IPv4, TCP IPv6 or named PIPE protocols.

The registry options for the GeoIP Service are stored under the following key:

```
HKEY_LOCAL_MACHINE\SOFTWARE\SV Foster\GeoIP Service
```

The key contains the following values:

* InstalledPath: A REG_EXPAND_SZ value that specifies the path of the folder that contains the GeoIP Service executable file. The default value is `C:\Program Files\SV Foster's GeoIP Service\`.

* DatabasePath: A REG_EXPAND_SZ value that specifies the path of the folder that contains the database files. The default value is `C:\Program Files\SV Foster's GeoIP Service\Database\`.

* DatabaseFileNameASN: A REG_EXPAND_SZ value that specifies the name of the database file that contains the autonomous system information. The default value is `GeoLite2-ASN.mmdb`.

* DatabaseFileNameGeo: A REG_EXPAND_SZ value that specifies the name of the database file that contains the geolocation information. The default value is `GeoLite2-City.mmdb`.

The key also contains a subkey named `Server`, which contains one or more subkeys that represent the server instances for each protocol. Each subkey has a name that is a hexadecimal number from 00000000 to FFFFFFFF, and contains the following values:

* Type: A REG_DWORD value that specifies the type of the server instance, where 0 means named PIPE, 1 means TCP IPv4, and 2 means TCP IPv6.

* Enabled: A REG_DWORD value that specifies whether the server instance is enabled or not, where 1 means enabled, and 0 means disabled.

* PipeName: A REG_SZ value that specifies the name of the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The default value is `\\.\PIPE\GeoIPSVCv1`.

* SecurityAttributesPolicy: A REG_DWORD value that specifies the security attributes policy for the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The possible values are 0, and 1, where 0 means default security attributes and 1 means custom security attributes. The default value is `1`.

* SecurityAttributesCustom: A REG_SZ value that specifies the custom security attributes for the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type and when the SecurityAttributesPolicy value is 1. The value is a string that represents a security descriptor in SDDL format. The default value is `D:(D;OICI;GA;;;BG)(D;OICI;GA;;;AN)(A;OICI;GRGWGX;;;AU)(A;OICI;GA;;;BA)`, which grants full access to administrators and read-write-execute access to authenticated users, and denies access to built-in guests and anonymous users.

* TimeoutIOMS: A REG_DWORD value that specifies the timeout in milliseconds for the input and output operations of the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The default value is `30000`, which means 30 seconds.

* Address: A REG_SZ value that specifies the IP address that is used by the server instance. This value is only applicable for the TCP IPv4 and TCP IPv6 types. The default value is `127.0.0.1` for the TCP IPv4 type, and `::1` for the TCP IPv6 type, which are the loopback addresses for each protocol.

* Port: A REG_DWORD value that specifies the port number that is used by the server instance. This value is only applicable for the TCP IPv4 and TCP IPv6 types. The default value is `28780`, which is the decimal representation of 706C in hexadecimal.

* TimeoutRecieveMS: A REG_DWORD value that specifies the timeout in milliseconds for receiving data from the client. This value is applicable for all types. The default value is `30000`, which means 30 seconds.

* TimeoutSendMS: A REG_DWORD value that specifies the timeout in milliseconds for sending data to the client. This value is applicable for all types. The default value is `30000`, which means 30 seconds.


### SDDL security descriptor format

A security descriptor is a data structure that contains information about the security attributes of an object, such as a named pipe. A security descriptor consists of four components: an owner, a group, a discretionary access control list (DACL), and a system access control list (SACL). The owner and the group identify the user or group that owns and controls the object. The DACL and the SACL specify the access rights and audit settings for the object.

The security descriptor definition language (SDDL) is a string format that can be used to represent a security descriptor as a text. The SDDL format is useful for storing and transferring security descriptors between different systems or applications. The SDDL format is also used by the GeoIP Service registry options to specify the custom security attributes for the named pipe.

The SDDL format for a security descriptor consists of the following elements:
1. A revision number that indicates the version of the SDDL format. The current revision number is 1.
2. A colon (:) that separates the revision number from the rest of the string.
3. An owner identifier that starts with O: and is followed by a security identifier (SID) that represents the owner of the object. The SID can be either a numeric value or a predefined abbreviation, such as BA for built-in administrators or AU for authenticated users. For example, O:BA means that the owner is the built-in administrators group.
4. A group identifier that starts with G: and is followed by a SID that represents the group of the object. The SID can be either a numeric value or a predefined abbreviation, as in the owner identifier. For example, G:AU means that the group is the authenticated users group.
5. A DACL identifier that starts with D: and is followed by a list of access control entries (ACEs) that specify the access rights for the object. Each ACE is enclosed in parentheses and consists of four parts: an access type, an access mask, a flag, and a trustee. The access type can be A for allow or D for deny. The access mask can be a hexadecimal value or a combination of predefined abbreviations, such as GA for generic all or GR for generic read. The flag can be a hexadecimal value or a combination of predefined abbreviations, such as CI for container inherit or OI for object inherit. The trustee can be a SID or a predefined abbreviation, as in the owner and group identifiers. For example, (A;;GA;;;BA) means that the ACE allows generic all access to the built-in administrators group.
6. A SACL identifier that starts with S: and is followed by a list of ACEs that specify the audit settings for the object. Each ACE is enclosed in parentheses and consists of four parts: an audit type, an access mask, a flag, and a trustee. The audit type can be AU for audit success or AL for audit failure. The access mask, the flag, and the trustee are the same as in the DACL identifier. For example, (AU;;GR;;;AU) means that the ACE audits successful generic read access by the authenticated users group.


The following is an example of a security descriptor in SDDL format for a named pipe:
```
O:BA G:AU D:(D;OICI;GA;;;BG)(D;OICI;GA;;;AN)(A;OICI;GRGWGX;;;AU)(A;OICI;GA;;;BA) S:(AU;OICI;GR;;;AU)
```
This security descriptor means that the owner is the built-in administrators group, the group is the authenticated users group, the DACL denies generic all access to the built-in guests and anonymous users groups, allows generic read, write, and execute access to the authenticated users group, and allows generic all access to the built-in administrators group, and the SACL audits successful generic read access by the authenticated users group.


## Building

GeoIP Service uses the Microsoft Visual Studio 2022 for its builds.

To build GeoIP Service from source files with Microsoft Visual Studio, you can use either the graphical mode or the command-line mode. Here are the instructions for both methods:

### Graphical mode
1. Open Microsoft Visual Studio and select Open a project or solution from the start page or the File menu.
2. Browse to the folder where the .sln file is located and select it. This will load the project in Microsoft Visual Studio.
3. Select the configuration and platform for the project by using the drop-down menus on the toolbar. For example, you can choose Debug or Release for the configuration, and x86 or x64 for the platform.
4. Build the project by clicking on the Build menu and selecting Build Solution. You can also use the keyboard shortcut Ctrl+Shift+B.
5. Run the project by clicking on the Debug menu and selecting Start Debugging. You can also use the keyboard shortcut F5

### Command-line mode
1. Open a Developer Command Prompt for Microsoft Visual Studio. You can find it in the Start menu under Microsoft Visual Studio Tools.
2. Navigate to the folder where the .sln file is located by using the cd command.
3. Invoke the MSBuild tool to build the project. You can specify various options and flags for the tool. For example, the following command builds the project with the Release configuration and the x64 platform:
```
msbuild GeoIP-Service.sln /p:Configuration=Release /p:Platform=x64
```
4. Run your executable by typing its name in the command prompt. For example:
```
Maintenance-exe-x86-64\maint /?
```

## Authors

This program was written and is maintained by SV Foster.


## License

This program is available under EULA, see [EULA text file](EULA.txt) for the complete text of the license. This program is free for personal, educational and/or non-profit usage.

libmaxminddb Copyright [MaxMind, Inc](https://github.com/maxmind/libmaxminddb), 2013-2022. Founded in 2002 and creator of GeoIP®, MaxMind is an industry leader in IP geolocation, proxy detection, and online fraud prevention solutions.
