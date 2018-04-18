# CTGP-7 Launcher
Launcher app used with CTGP-7 to launch the mod. This repository also has a modification of the plugin loader used with Luma3DS plugin loader fork but specific for MK7 title IDs. If you came here only for the plugin loader I recommend checking the repository for that version instead.

# Building
Building CTGP-7 Launcher is handled through buildtools. You have to have the following installed:
- [devkitPro with devkitARM](https://sourceforge.net/projects/devkitpro/files/Automated%20Installer/)
- ctrulib (installed automatically with the devkitARM script)
- [citro3d](https://github.com/fincs/citro3d)
- [portlibs](https://github.com/devkitPro/3ds_portlibs)

Once you have installed all the dependencies simply run `make` in the BootNTR directory and if you set it all up correctly it should build.


# Credits
- @Nanquitas: BootNTRSelector, from which this app is forked.
- @44670: Initial BootNTR and NTR CFW developer.
