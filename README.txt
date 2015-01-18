If you want to compile SuperMarioBuda (aka smb), you need to install this pakages:

scons
libconfig8-dev
libsdl1.2-dev
libsdl-mixer1.2-dev
libsdl-image1.2-dev
libsdl-ttf2.0-dev
libwiimote-devel (OPTIONAL FOR WIIMOTE CONTROLLER)
bluez-libs-devel (OPTIONAL FOR WIIMOTE CONTROLLER)

So, for example on ubuntu type:
#sudo apt-get install scons
... and so on.

Now, from your terminal move to the smb directory:
#cd smb-0.X.Y 

and type 'scons' command:
#scons

to start the program, just type ./smb:
#./smb

KEY:
ESC 		Quit
F12 		Toggle Fullscreen/window
F11 		Take a screenshot
ARROW 	Move Mario
LEFTCONTROL Jump

See smb.cfg for customization and options.


