### Generate cross section file

#### Using moltres_xs.py

`$MOLTRES/python/moltres_xs.py msfr_xs.inp`

#### Using extractSerpent2GCs.py

TODO: Add command for this part

#### Running the Moltres MSFR file

The `msfr.i` input file uses the json XS file by default.
Make sure you have your MOOSE conda environment activated.

`$MOLTRES/moltres-opt -i msfr.i`
