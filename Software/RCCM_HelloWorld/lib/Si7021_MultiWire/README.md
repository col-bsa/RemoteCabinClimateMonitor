# Si7021_MultiWire

A Particle library for the Si7021 Temperature and Humidity sensor, based on the Sparkfun Si7021 Breakout, but with support for Particle devices with multiple I2C buses (for instance, the Electron).

## Usage

Connect A Photon or Electron, add the Si7021_MultiWire library to your project and follow this simple example:

```
#include "Si7021_MultiWire.h"
Si7021_MultiWire si7021_MultiWire = Si7021_MultiWire(Wire); // Use Wire (Photon) or Wire1 (Photon and Electron)

void setup() {
  si7021_MultiWire.begin();
}

void loop() {
  si7021_MultiWire.process();
}
```

See the [examples](examples) folder for more details.

## Documentation

TODO: Describe `Si7021_MultiWire`

## Contributing

Here's how you can make changes to this library and eventually contribute those changes back.

To get started, [clone the library from GitHub to your local machine](https://help.github.com/articles/cloning-a-repository/).

Change the name of the library in `library.properties` to something different. You can add your name at then end.

Modify the sources in <src> and <examples> with the new behavior.

To compile an example, use `particle compile examples/usage` command in [Particle CLI](https://docs.particle.io/guide/tools-and-features/cli#update-your-device-remotely) or use our [Desktop IDE](https://docs.particle.io/guide/tools-and-features/dev/#compiling-code).

After your changes are done you can upload them with `particle library upload` or `Upload` command in the IDE. This will create a private (only visible by you) library that you can use in other projects. Do `particle library add Si7021_MultiWire_myname` to add the library to a project on your machine or add the Si7021_MultiWire_myname library to a project on the Web IDE or Desktop IDE.

At this point, you can create a [GitHub pull request](https://help.github.com/articles/about-pull-requests/) with your changes to the original library.

If you wish to make your library public, use `particle library publish` or `Publish` command.

## LICENSE

Copyright 2018 Brandon Satorm <mailto:brandon@particle.io>

Licensed under the MIT license
