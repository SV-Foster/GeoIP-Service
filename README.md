# GeoIP Service

Are you looking for a reliable and efficient way to manage IP address data and perform critical database operations, like updates, without affecting your service availability and performance? If so, you might be interested in GeoIP Service that offers a comprehensive solution for your needs!

GeoIP Service is a background Windows Service that runs on your Windows Server and manages the GeoIP database, a global database that provides information about the geographic location, country, region, city, ASN and other details of any IPv4 or IPv6 address. By using GeoIP Service, you can easily and fast retrieve and update this information for your own purposes, such as analytics, security or marketing.

But that's not all. GeoIP Service also allows you to perform very important database files changes, such as for backup, restore, migration, or optimization, without requiring a restart or causing any client disconnects. This means that you can maintain 100% uptime and ensure the continuity and quality of your service, even when you need to make major changes to your database.

Moreover, GeoIP Service supports multiple protocols and standards, such as IPv4, IPv6 and Named PIPE, and provides Windows domain authentication and security support, so that you can communicate with your clients and servers in a secure and flexible way. Whether you need to handle IPv4 or IPv6 addresses or use Named PIPE for inter-process communication, GeoIP Service can handle it all.

GeoIP Service is designed to be easy to install, configure and use, and comes with a user-friendly interface and documentation. You can also customize GeoIP Service to suit your specific needs and preferences.

Don't miss this opportunity to take your IP address management and database operations to the next level with GeoIP Service!


## Usage

To install GeoIP Service on Windows, you need to follow these steps:

1. Create the folders needed for the service. For example, you can create a folder named "C:\Program Files\SV Foster's GeoIP Service\" and copy the executable files of the service and database files into it.
2. Run maint.exe utility with the path to the copied service executable as an argument. This will register the service in the Windows registry and create a service name for it. For example, you can run the following command in a command prompt:

```maint.exe install "C:\Program Files\SV Foster's GeoIP Service\GeoipSVC.exe"```

3. Check options in the registry key "HKEY_LOCAL_MACHINE\SOFTWARE\SV Foster\GeoIP Service", change values if needed.
4. Start the service with the "sc start GeoIPSVC" command. This will launch the service and make it run in the background. You can also use the Services Manager to start the service from the graphical user interface.
5. Check that the service is running with the clgeoip.exe command. This is a command-line tool that allows you to communicate with the service and get IP address information. For example, you can run the following command to get the information of the IP address 199.83.131.167:

```clgeoip /IP 199.83.131.167```

The command will return the country, region, city, latitude, longitude, and other details of the IP address, if available.
6. Change the database files on the GeoIP server without restart or client disconnects with the clgeoip command. The service uses two database files: geo.mmdb and asn.mmdb, which contain the IP address information and the autonomous system number information, respectively. You can download the latest versions of these files from the GeoIP database website. To change the database files, you need to use the "/Mode Hotplug" option and specify the paths of the new files with the /FileGeo and /FileASN options. For example, you can run the following command to change the database files to geo-new.mmdb and asn-new.mmdb:

```clgeoip /Mode Hotplug /FileGeo geo-new.mmdb /FileASN asn-new.mmdb```

The command will replace the old files with the new ones with the GeoIP Service without affecting the service operation or causing any client disconnects!

You have successfully installed and configured the GeoIP Service on Windows. You can now use the service to get IP address information and perform database files changes with ease and efficiency!


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

```maint.exe /?```


## Authors

This program was written and is maintained by SV Foster.


## License
-------

This program is available under EULA, see [EULA file](EULA.txt) for the complete text of the license. This program is free for personal, educational and/or non-profit usage.

libmaxminddb Copyright 2013-2022 [https://github.com/maxmind/libmaxminddb](MaxMind, Inc). Founded in 2002 and creator of GeoIP®, MaxMind is an industry leader in IP geolocation, proxy detection, and online fraud prevention solutions.
