# GeoIP Service

Are you looking for a reliable and efficient way to manage IP address data and perform critical database operations, like updates, without affecting your service availability and performance? If so, you might be interested in GeoIP Service that offers a comprehensive solution for your needs!

![Screenshot](Documents/screenshot%20001.png)
![Screenshot](Documents/screenshot%20002.png)

GeoIP Service is a background Windows Service that runs on your Windows Server and manages the GeoIP database, a global database that provides information about the geographic location, country, region, city, ASN and other details of any IPv4 or IPv6 address. By using GeoIP Service, you can easily and fast retrieve and update this information for your own purposes, such as analytics, security or marketing.

But that's not all. GeoIP Service also allows you to perform very important database files changes, such as for backup, restore, migration, or optimization, without requiring a restart or causing any client disconnects. This means that you can maintain 100% uptime and ensure the continuity and quality of your service, even when you need to make major changes to your database.

Moreover, GeoIP Service supports multiple protocols and standards, such as IPv4, IPv6 and Named PIPE, and provides Windows domain authentication and security support, so that you can communicate with your clients and servers in a secure and flexible way. Whether you need to connect through IPv4, IPv6 or use Named PIPE for inter-process communication, GeoIP Service can handle it all.

GeoIP Service is designed to be easy to install, configure and use, and comes with a user-friendly interface and documentation. You can also customize GeoIP Service to suit your specific needs and preferences.

Don't miss this opportunity to take your IP address management and database operations to the next level with GeoIP Service!


## Usage

To install GeoIP Service on Windows, you need to follow these steps:

1. Create the folders needed for the service. For example, a folder named `C:\Program Files\SV Foster's GeoIP Service\ `, and copy the files of the service into it. Also create `C:\Program Files\SV Foster's GeoIP Service\Database\ ` folder and copy .mmdb database files into, the fresh new copy of them you can download from the MaxMind web site for free. The service uses two database files: GeoLite2-City.mmdb and GeoLite2-ASN.mmdb, which contain the IP address information and the autonomous system number information, respectively.
2. Run `maint.exe` utility with the path to the copied service executable as an argument. This will register the service in the Windows registry and create a service name for it. For example, you can run the following command in a command prompt:

```
maint install "C:\Program Files\SV Foster's GeoIP Service\GeoipSVC.exe"
```

3. Check options in the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\SV Foster\GeoIP Service`, change values if needed. All options are  described below in this document.
4. Start the service with the `sc start GeoIPSVC` command. This will launch the service and make it run in the background. You can also use the Services Manager to start the service from the graphical user interface. GeoIP Service is configured to start automatically on every boot, no need to start it manually every time.
5. Check that the service is running with the `clgeoip.exe` command. This is a command-line tool that allows you to communicate with the service and get IP address information. For example, you can run the following command to get the information of the IP address 199.83.131.167:

```
clgeoip /IP 199.83.131.167
```

The command will return the country, region, city, latitude, longitude, and other details of the IP address, if available.

You have successfully installed and configured the GeoIP Service on Windows. You can now use the service to get IP address information and perform database files changes with ease and efficiency!


### Hotplugging updated .mmdb files

You can update the database files on the GeoIP server without restart or client disconnects with the `clgeoip.exe` tool. The service uses two database files: `GeoLite2-City.mmdb` and `GeoLite2-ASN.mmdb`, which contain the IP address information and the autonomous system number information, respectively. The latest versions of these files are available on the MaxMind website for free. To change the database files, you need to use the `/Mode Hotplug` option and specify the names of the new files with the `/FileGeo` and `/FileASN` options. For example, run the following command to change the database files to geo-new.mmdb and asn-new.mmdb:

```
clgeoip /Mode Hotplug /FileGeo geo-new.mmdb /FileASN asn-new.mmdb
```

The command will update the database files with the GeoIP Service without affecting the service operation or causing any client disconnects!


## Options

This section explains the registry options for the GeoIP Service. The GeoIP Service uses two database files, GeoLite2-ASN.mmdb and GeoLite2-City.mmdb, which are updated monthly by MaxMind. The GeoIP Service can be accessed by clients using TCP IPv4, TCP IPv6, or named PIPE protocols.

The registry options for the GeoIP Service are stored under the following key:

```
HKEY_LOCAL_MACHINE\SOFTWARE\SV Foster\GeoIP Service
```

The key contains the following values:

* DatabaseFileNameASN: A REG_EXPAND_SZ value that specifies the name of the database file that contains the autonomous system information. The default value is GeoLite2-ASN.mmdb.

* DatabaseFileNameGeo: A REG_EXPAND_SZ value that specifies the name of the database file that contains the geolocation information. The default value is GeoLite2-City.mmdb.

* DatabasePath: A REG_EXPAND_SZ value that specifies the path of the folder that contains the database files. The default value is C:\Program Files\SV Foster's GeoIP Service\Database\.

* InstalledPath: A REG_EXPAND_SZ value that specifies the path of the folder that contains the GeoIP Service executable file. The default value is C:\Program Files\SV Foster's GeoIP Service\.

The key also contains a subkey named `Server`, which contains one or more subkeys that represent the server instances for each protocol. Each subkey has a name that is a hexadecimal number from 00000000 to FFFFFFFF, and contains the following values:

* Type: A REG_DWORD value that specifies the type of the server instance, where 0 means named PIPE, 1 means TCP IPv4, and 2 means TCP IPv6.

* Enabled: A REG_DWORD value that specifies whether the server instance is enabled or not, where 1 means enabled, and 0 means disabled.

* PipeName: A REG_SZ value that specifies the name of the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The default value is \\.\PIPE\GeoIPSVCv1.

* SecurityAttributesPolicy: A REG_DWORD value that specifies the security attributes policy for the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The possible values are 0, and 1, where 0 means default security attributes and 1 means custom security attributes. The default value is 1.

* SecurityAttributesCustom: A REG_SZ value that specifies the custom security attributes for the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type and when the SecurityAttributesPolicy value is 2. The value is a string that represents a security descriptor in SDDL format. The default value is D:(D;OICI;GA;;;BG)(D;OICI;GA;;;AN)(A;OICI;GRGWGX;;;AU)(A;OICI;GA;;;BA), which grants full access to administrators and read-write-execute access to authenticated users, and denies access to built-in guests and anonymous users.

* TimeoutIOMS: A REG_DWORD value that specifies the timeout in milliseconds for the input and output operations of the named PIPE that is used by the server instance. This value is only applicable for the named PIPE type. The default value is 30000, which means 30 seconds.

* Port: A REG_DWORD value that specifies the port number that is used by the server instance. This value is only applicable for the TCP IPv4 and TCP IPv6 types. The default value is 28780, which is the decimal representation of 706C in hexadecimal.

* Address: A REG_SZ value that specifies the IP address that is used by the server instance. This value is only applicable for the TCP IPv4 and TCP IPv6 types. The default value is 127.0.0.1 for the TCP IPv4 type, and ::1 for the TCP IPv6 type, which are the loopback addresses for each protocol.

* TimeoutRecieveMS: A REG_DWORD value that specifies the timeout in milliseconds for receiving data from the client. This value is applicable for all types. The default value is 30000, which means 30 seconds.

* TimeoutSendMS: A REG_DWORD value that specifies the timeout in milliseconds for sending data to the client. This value is applicable for all types. The default value is 30000, which means 30 seconds.


## Building

GeoIP Service uses the Microsoft Visual Studio 2022 for its builds.

To build GeoIP Service from source files with Microsoft Visual Studio, you can use either the graphical mode or the command-line mode. Here are the instructions for both methods:

Graphical mode:
1. Open Microsoft Visual Studio and select Open a project or solution from the start page or the File menu.
2. Browse to the folder where the .sln file is located and select it. This will load the project in Microsoft Visual Studio.
3. Select the configuration and platform for the project by using the drop-down menus on the toolbar. For example, you can choose Debug or Release for the configuration, and x86 or x64 for the platform.
4. Build the project by clicking on the Build menu and selecting Build Solution. You can also use the keyboard shortcut Ctrl+Shift+B.
5. Run the project by clicking on the Debug menu and selecting Start Debugging. You can also use the keyboard shortcut F5

Command-line mode:
1. Open a Developer Command Prompt for Microsoft Visual Studio. You can find it in the Start menu under Microsoft Visual Studio Tools.
2. Navigate to the folder where the .sln file is located by using the cd command.
3. Invoke the MSBuild tool to build the project. You can specify various options and flags for the tool. For example, the following command builds the project with the Release configuration and the x64 platform:

```msbuild GeoIP-Service.sln /p:Configuration=Release /p:Platform=x64```


Run your executable by typing its name in the command prompt. For example:

```maint /?```


## Authors

This program was written and is maintained by SV Foster.


## License

This program is available under EULA, see [EULA text file](EULA.txt) for the complete text of the license. This program is free for personal, educational and/or non-profit usage.

libmaxminddb Copyright 2013-2022 [MaxMind, Inc](https://github.com/maxmind/libmaxminddb). Founded in 2002 and creator of GeoIP®, MaxMind is an industry leader in IP geolocation, proxy detection, and online fraud prevention solutions.
