Dependencies:
 libx11-dev xfonts-75dpi xfonts-100dpi csh
 
 Can install in ubuntu using 
 
 >$sudo apt-get install libx11-dev

>$ sudo apt-get install xfonts-75dpi

>$ sudo apt-get install xfonts-100dpi


---

In arch, those font libraries are called:
 xorg-fonts-75dpi and xorg-fonts-100dpi
 
In RedHat/CentOS, the font libraries are:
 xorg-x11-fonts-75dpi and xorg-x11-fonts-100dpi
 However, it segfaults when initially loading fonts, so perhaps something else is missing? In either case, restarting the computer seems to fix the problem*.
 
 The makefile in the main directory should compile all the programs in the subdirectories, each of which has its own makefile that you won't need to run then! To make, just type make :)
 
 *May need to ```fc-cache -vf``` (possibly sudo) for system to find new fonts*
