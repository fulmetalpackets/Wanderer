Wanderer
========

Warning
========

Do not run Wanderer.exe on any network you so not wish to infect every windows system.  You have been warned.

About
========

Wanderer is a non-malicious penetration testing tool which emulates worm like behavior.  It was designed to test defense measure such as an IDS and an incidents response teams procedures. Wanderer has been tested on Windows XP, 2003, 2008R2, and 7.

Overview
========

This application is written in C++ and is designed to propagate on a windows based network. By design, Wanderer as a standalone application does not preform any tasks besides propagation through the network, coping itself to each machine.  Any other functionality can be given to Wanderer as its payload.  

Upon execution Wanderer obtains the ip address and subnet of the machine it is operating on. It then creates a list of all possible ip address within it subnet up to a complete Class B.  It will not attempt to scan more than 65,000 addresses for efficiency.

Once it has a list of all possible ip address, it performs an arp scan of all address created previously and records which address respond.  The resulting list is then attempted to propagate to uses a very similar technique to PSexec via the Windows ADMIN$ share. Therefore an administrator username and password of the victim machine must be provided to Wanderer.  If Windows firewall is setup to block this share, Wanderer will not spread.  If a successful connection is made to a machine, Wanderer copies itself and the given payload to the victim machine.  After the copy is completed, Wanderer installs itself as a Windows service with the same arguments as it was run with. Wanderer currently does not set itself up to boot on start.   After a successfully installation, the service starts itself, executes the payload provided by the user, and the entire process is restarted.  The service is designed to never stop, but to sleep for a random amount of time between each run.  This will allow Wanderer to find new machines as they appear on the network and also help minimize multiple instances of Wanderer scanning at the same time.  Wanderer logs all of its actions and errors to C:\WandererError.txt. 

Source Files:
main.cpp - holds Windows main and service code.  Parses command line arguments and calls Wanderers main function. 
Wanderer.cpp - Holds the bulk of Wanderer's code and all functions that are specific to this application
Utils.cpp - Windows helper functions
Log.cpp - Thread safe logging class

Build
========

Open 'solution/Wanderer.sln' in Visual Studio C++ and build the solution in Release mode for Win32 to make Wanderer.exe

Usage
========

Wanderer requires an administrator username and password in order to propagate between machines.  The user on the command line should never call the service parameter.  This is for when Wanderer invokes itself as a service on the victim machine.The payload parameter is only needed if it is desired that Wanderer do more than propagate.  If a payload is given, the payload must either be in the same folder, or a fully qualified path must be provided.  The syntax to run Wanderer is as follows:

> Wanderer.exe [-service] -u username -p password [-e payload]

License
========

Licensed under a 3 clause BSD license, please see LICENSE for details.   