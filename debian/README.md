#Building the engine on any Linux
On Debian/Ubuntu, building a package (see below) is recommended.
On other distributions, install development files of the following
libraries. (In brackets are versions that are known to work, but other
versions should work, too.)

-   Allegro 4 (> 4.2.2)
-   libaldmb (0.9.3)
-   libdumb (0.9.3)
-   libfreetype (2.4.9)
-   libogg (1.2.2-1.3.0)
-   libtheora (1.1.1-1.2.0)
-   libvorbis (1.3.2)

Download the sources with git and change into the **ags** directory:

    git clone git://github.com/adventuregamestudio/ags.git
    cd ags

Compile the engine:

    make --directory=Engine --file=Makefile.linux

The **ags** executable can now be found in the **Engine** folder and
can be installed with

    sudo make --directory=Engine --file=Makefile.linux install

Please take note of the usage instructions at the end of this document.


#Building a Debian/Ubuntu package of AGS
Building a package is the preferred way to install software on
Debian/Ubuntu. This is how it's done.

Install the build dependencies:

    sudo apt-get install git debhelper pkg-config liballegro4.2-dev libaldmb1-dev libfreetype6-dev libtheora-dev libvorbis-dev libogg-dev

Download the sources with git and change into the **ags** directory:

    git clone git://github.com/adventuregamestudio/ags.git
    cd ags

Build the package and install it with gdebi:

    fakeroot debian/rules binary
    sudo gdebi ../ags_3.21.1115~JJS-1_*.deb

#Workaround: 32 bit AGS on 64 bit system
The 64 bit version of AGS causes problems on some systems, namely frequent
random crashes. Until this is resolved, a workaround is to use a 32 bit version
of AGS on a 64 bit system.
The development versions of Debian and Ubuntu support parallel
installation of both 32 and 64 bit versions of all required libraries
(multiarch), so you can build a 32 bit AGS to use on your 64 bit system.
This part works only on Debian sid and wheezy and Ubuntu quantal.
If you observe this problem and are able to help resolve it, that would be great.

Download the sources with git and change into the **ags** directory:

    git clone git://github.com/adventuregamestudio/ags.git
    cd ags

##Matching working directory and orig tarball
To build the package, it is required that there is an "orig tarball"
that has the same content as the working directory. This tarball is generated
from the git content with

    debian/rules get-orig-source

The working directory must have the same content as git, i.e. be "clean".
To ensure this, check if the working directory is clean with

    git status

If there are changes, run 

    debian/rules clean 

and/or

    git reset --hard HEAD

If there are still untracked files, delete them manually.

Run `debian/rules get-orig-source` every time the sources change. If
you want to change the sources yourself, you have to commit the
changes to git, then run `debian/rules get-orig-source`, then
build the package.


##Building the package

Enable multiarch:

    sudo dpkg --add-architecture i386
    sudo apt-get update

Install and prepare pbuilder (use the same distribution you are using,
i.e. `sid`, `wheezy` or `quantal`):

    sudo apt-get install pbuilder
    sudo pbuilder create --distribution sid --architecture i386

This creates an i386 chroot which will be used to build the i386 package
on an amd64 system. pbuilder automatically manages the build dependencies.
The pbuilder base can later be updated with

    sudo pbuilder update

Build the package with pbuilder and install it and its dependencies with gdebi:

    cd ags
    pdebuild
    sudo gdebi /var/cache/pbuilder/result/ags_3.21.1115~JJS-1_i386.deb


#Using the engine

To start an AGS game, just run ags with the game directory or the game
file as parameter, e.g.

    ags /path/to/game/

or

    ags game.exe

The configuration file **acsetup.cfg** in the game directory will be used
if present. Sometimes a configuration file coming with a game can cause problems,
so if a game doesn't start, try deleting **acsetup.cfg** first.

For midi music playback, you have to download GUS patches. We recommend
"Richard Sanders's GUS patches" from this address:

http://alleg.sourceforge.net/digmid.html

A direct link is here:

http://www.eglebbk.dds.nl/program/download/digmid.dat

Rename that file to **patches.dat** and place it directly into your home folder.
