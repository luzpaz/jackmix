# JackMix

## Authors

| Author                               | Responsibility           |
|--------------------------------------|--------------------------|
| Arnold Krille <arnold@arnoldarts.de> | Almost everything.       |
| Nick Bailey <nick@n-ism.org>         | MIDI control extensions. |
|                                      | Qt5 port attempt.        |

---
See INSTALL for installation instructions;
    AUTHORS for contact information;
    NEWS    for new features in this version.

JackMix is a matrix mixer allowing p input channels to be mixed into
q output channles by a matrix of faders. It relies on the Jack framework
to route these inputs and outputs.

This version of jackmix is enhanced for compatibility with an external
MIDI control surface and compiles for qt5.

To build the qt5 version, say `git clone -b qt5` or if you've already
cloned the project, `git checkout qt5`

Nick Bailey <nick@n-ism.org>
