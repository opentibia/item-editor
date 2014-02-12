# What is OTItemEditor?

OTItemEditor is a program used to edit the OTB data files used by OpenTibia
servers and tools to map the item IDs used by the client to a consistent set of
IDs used by the server and tools. This is necessary since CIPSoft changes the
client-side IDs every major version released and remapping of them is required.

This tool can read the dat and spr files distributed with the client
and either match them up with the server-side IDs via image recognition and also
add new items to the data files.


# Download

There are no builds of OTItemEditor available at the moment.


# Compiling

You can build the project using the provided otitemeditor.sln in Visual
Studio 2010 or later.

Just run ./build-linux.sh to compile. It uses xbuild from Mono package.

If somebody can contribute compiling instructions for other platforms, that'd be
great.


# Bugs

If you find a bug use the bug tracker on github to report it.


# Contact

The easiest way to contact the developers is to post to the forums at
[otfans.net](otfans.net).


# License

OTItemEditor is made available under the GPLv2 License, this means that forks
of the project must be distributed with sources available.


This project uses [famfamfam silk icons](http://www.famfamfam.com/lab/icons/silk/)