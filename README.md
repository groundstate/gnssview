Installation
------------

You will need to install Qt4 development packages.

In the `gnssview` source directory:

	qmake-qt4 gnssview.pro
	make

Communicating with a GNSS receiver
----------------------------------
You will need to write a program that communicates with your GNSS receiver and then broadcasts the required information via UDP multicast. There are two sample Perl scripts included with the distribution that demonstrate this.

The format of the UDP packet is successive lines in the format:

	time_stamp,constellation_id,satellite_id,azimuth,elevation,signal_level

where the constellation identifiers are:

	0 Beidou
	1 GPS
	2 Galileo
	3 GLONASS
	4 QZSS
	5 SBAS

Azimuth and elevation must be in decimal degrees. The signal level can be whatever the receiver reports: it is scaled via an entry in the configuration file. The satellite identifier needs to be a unique number for each satellite.

Power management
----------------

The `tvservice` tool is used on the Raspberry Pi. This has worked fine for me on an LCD monitor. The display and the backlight go off.

On other Linuxen+x386, YMMV. I tried `dpms` and `vbetool` but there were problems. With `xset`, the backlight would go off briefly and then come back on. With `vbetool`, there were occasional freezes of up to 30s before the monitor turned off. Unfortunately there is no standard way of controlling the monitor in Linux so power management may not work for you.

On Debian systems, `vbetool` needs to run via `sudo` so to disable the password for just `vbetool` you need to edit /etc/sudoers:

	user_name ALL=(ALL) NOPASSWD: /usr/sbin/vbetool
	
Configuration file
------------------

`gnssview` uses a configuration file `gnssview.xml`. The comments in the sample file should be enough to get you going.
The search path for this is `./:~/gnssview:~/.gnssview:/usr/local/etc:/etc`
All other paths are explicit.

Known bugs/quirks
-----------------

The power on/power off logic assumes on < off.

Website
-------

There's a bit more information and screenshots at [ninepointtwoghz.org](http://ninepointtwoghz.org/gnssview.php)