# Schema for button map and controller topology

Data is provided in two files:

1. buttonmap.xml
2. topology.xml

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

## topology.xml

The controller topology defines the possible ways that input devices can be connected. The root tag is `<logicaltopology>` because this describes the topology that the emulator's logic is capable of.

The topology has the following properties:

* Player limit (max number of players handled by the core, needed for FCEUmm)

The topology of the emulator is a tree structure, with hubs and daisy-chainable controllers as nodes. The root of the topology is a list of ports on the virtual game console.

Ports have the following properties:

1. Port type
2. Port ID (optional for keyboard and mouse)
3. List of accepted controllers

The port type is one of the following:

* `keyboard`
* `mouse`
* `controller`

The port ID is derived from the port's physical connector or the 1-indexed player number on the virtual console.

Accepted controllers have the following properties:

1. Add-on ID
2. List of ports

Ports accept controllers and controllers have ports, so data can be operated on recursively.
