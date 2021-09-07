#!/bin/bash

echo "Mesh fuel_pin.i"
~/projects/moltres/moltres-opt -i fuel_pin.i --mesh-only fuel_pin.e
echo
echo "Mesh square.i"
~/projects/moltres/moltres-opt -i square.i --mesh-only square.e
echo
echo "Mesh full_mesh.i"
~/projects/moltres/moltres-opt -i full_mesh.i --mesh-only full_mesh.e
