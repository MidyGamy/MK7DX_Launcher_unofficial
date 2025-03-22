# MK7DX Launcher
Launcher app used with MK7DX to launch the mod. This repository also has a modification of the plugin loader used in the Luma3DS with plugin loading support fork but specific for MK7 title IDs. If you came here only for the plugin loader I recommend checking that repository instead.

# Building
This has been uploaded for archive and reference purposes. The latest commit may need some modifications so it can be compiled properly.

Building MK7DX Launcher is handled through buildtools. You have to have the following installed:
- [devkitPro with devkitARM](https://sourceforge.net/projects/devkitpro/files/Automated%20Installer/)
- ctrulib (installed automatically with the devkitARM script)
- [citro3d](https://github.com/fincs/citro3d)
- [portlibs](https://github.com/devkitPro/3ds_portlibs)

Once you have installed all the dependencies simply run `make` in the BootNTR directory and if you set it all up correctly it should build.


# Credits
- [@Nanquitas](https://github.com/Nanquitas) : BootNTRSelector, from which this app is forked.
- [@44670](https://github.com/44670) : Initial BootNTR and NTR CFW developer.
