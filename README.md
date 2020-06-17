# wmcellauto

Window Maker dockapp for displaying cellular automata

## Installation

You will need [libdockapp](http://dockapps.net/libdockapp) to build wmcellauto.

If building from git (not necessary if using a tarball):

	autoreconf -fvi

Then the usual

    ./configure
    make
    sudo make install

## Usage

    wmcellauto [OPTIONS]

### Options
    -h, --help                   show this help text and exit
    -v, --version                show program version and exit
    -w, --windowed               run the application in windowed mode
    -a, --alivecolor <string>    color of live cells (default: light sea green)
    -d, --deadcolor <string>     color of dead cells (default: black)
    -t, --time <number>          time in ms between generations (default: 250)
    -b, --birth <string>         custom ruleset - specify number of adjacent
                                 cells required for birth (default: 3)
    -s, --survival <string>      custom ruleset - specify number of adjacent
                                 cells required for survival (default: 23)
    -r, --ruleset <string>       select ruleset (life (default), 2x2, day &
                                 night, flock, fredkin, highlife, life without
								 death, live free or die, maze, mazectric, move,
                                 replicator, seeds)
    --density <number>           percentage of live cells in initial random grid
                                 (default: 50)
    -R, --reset <integer>        number of generations until grid resets
                                 (default: -1, i.e., don't reset)


# Bugs

Please report bugs and feature requests at the
[issues page](https://github.com/d-torrance/wmcellauto/issues).

# Copyright

Copyright (C) 2017 Doug Torrance <dtorrance@piedmont.edu>

License: [GPL-2+](https://www.gnu.org/licenses/gpl-2.0.html)
