Libretro compatibility layer for the Kodi Game API

================================================================================

This add-on translates between the Kodi Game API and the libretro API,
possibly across a TCP/IP network (TODO).

================================================================================

This system will include a client executable that connects via TCP/IP to the
add-on loaded by Kodi. The client executable loads a game add-on or libretro
add-on and uses protobufs to pack C structs across the network.

