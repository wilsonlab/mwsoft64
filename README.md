Dependencies:
 libx11-dev xfonts-75dpi xfonts-100dpi csh

In arch, those font libraries are called:
 xorg-fonts-75dpi and xorg-fonts-100dpi
 
In RedHat/CentOS, the font libraries are:
 xorg-x11-fonts-75dpi and xorg-x11-fonts-100dpi
 However, it segfaults when initially loading fonts, so perhaps something else is missing?
 
 *May need to ```fc-cache -vf``` (possibly sudo) for system to find new fonts*
