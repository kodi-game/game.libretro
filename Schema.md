# Schema for button map

## buttonmap.xml

A button map has the following properties:

1. Button map schema version
2. List of controllers

A controller has the following properties:

1. Controller ID (equal to Kodi add-on ID)
2. Libretro device type (from the libretro API)
3. Libretro subclass index (defined by the core)
4. List of features

The purpose of the properties is to assign the libretro device type and subclass to the specified add-on ID.

A `<feature>` tag maps a feature name to a libretro constant. It has the following properties:

1. Feature name (from the kodi-game-controllers repo)
2. Libretro constant (from the libretro API)
