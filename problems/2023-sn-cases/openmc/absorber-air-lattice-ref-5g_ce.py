import openmc
import sys
import os
import matplotlib.pyplot as plt
import numpy as np
#sys.path.insert(1, '/home/smpark/projects/hybrid-sn-diff')
sys.path.insert(1, '/home/smpark/projects/alt-moltres/python')
from moltres_xs import openmc_xs # noqa: E402

# %% Materials

fuel = openmc.Material(material_id=103)
# Fuel composition after additional loadings (U235 = 71.71kg)
r = (71.71-65.25) / (72-65.25)
fuel_dens = 2.3275 * (1 + ((4641.50 / 4629.80) - 1) * r)
fuel.set_density('g/cm3', fuel_dens)
fuel_elements = {'Li7': (507.27 + (507.85-507.27) * r) * .99995,
                 'Li6': (507.27 + (507.85-507.27) * r) * .00005,
                 'Be': 293.96,
                 'F': 3103.22 + (3107.08-3103.22) * r,
                 'Zr': 513.97,
                 'U234': .67 + (.73-.67) * r,
                 'U235': 71.71,
                 'U236': .27 + (.30-.27) * r,
                 'U238': 141.91 + (142.32-141.91) * r}
total_fuel_weight = sum(fuel_elements.values())
for i in fuel_elements.keys():
    if i == 'Zr' or i == 'F' or i == 'Be':
        fuel.add_element(i, fuel_elements[i]/total_fuel_weight, 'wo')
    else:
        fuel.add_nuclide(i, fuel_elements[i]/total_fuel_weight, 'wo')
fuel.add_s_alpha_beta('c_Be')
fuel.temperature = 900

ctrlrod = openmc.Material(material_id=101)
ctrlrod.set_density('g/cm3', 5.873)
ctrlrod_elements = {'Gd': 70 * 157.25 * 2 / (157.25 * 2 + 15.999 * 3),
                    'Al': 30 * 26.982 * 2 / (26.982 * 2 + 15.999 * 3),
                    'O': 15.999 * 3 * (70 / (157.25 * 2 + 15.999 * 3) +
                                       30 / (26.982 * 2 + 15.999 * 3))}
total_ctrlrod_weight = sum(ctrlrod_elements.values())
for i in ctrlrod_elements.keys():
    ctrlrod.add_element(i, ctrlrod_elements[i]/total_ctrlrod_weight, 'wo')
ctrlrod.temperature = 900

air = openmc.Material(material_id=102)
air.set_density('g/cm3', .006)
air_elements = {'N': 14.007 * .95,
                'O': 15.999 * .05}
total_air_weight = sum(air_elements.values())
for i in air_elements.keys():
    air.add_element(i, air_elements[i]/total_air_weight, 'wo')
air.temperature = 900

graphite = openmc.Material(material_id=104)
graphite_dens = 1.86
graphite.set_density('g/cm3', graphite_dens)
graphite.add_nuclide('C0', 1, 'wo')
graphite.add_s_alpha_beta('c_Graphite')
graphite.temperature = 900

water = openmc.Material()
water_dens = .997
water.set_density('g/cm3', water_dens)
water.add_element('H', 2/3, 'ao')
water.add_element('O', 1/3, 'ao')
# water.add_s_alpha_beta('c_H_in_H2O')
water.temperature = 300

steel = openmc.Material()
steel_dens = 7.87
steel.set_density('g/cm3', steel_dens)
steel.add_element('Fe', 1, 'ao')
# steel.add_s_alpha_beta('c_Fe56')
steel.temperature = 300

reflector = openmc.Material.mix_materials([water, steel], [.5, .5],
                                          'vo', name='reflector')
reflector.add_s_alpha_beta('c_H_in_H2O')
reflector.add_s_alpha_beta('c_Fe56')
reflector.temperature = 300
reflector.id = 105

mats = openmc.Materials((ctrlrod, air, fuel, graphite, reflector))

# %% Geometry

bot_plane = openmc.YPlane(y0=0, boundary_type="reflective")
top_plane = openmc.YPlane(y0=1, boundary_type="reflective")
left_plane = openmc.XPlane(x0=0, boundary_type="reflective")
ctrlrod_plane = openmc.XPlane(x0=.5, boundary_type="transmission")
air_plane = openmc.XPlane(x0=1.5, boundary_type="transmission")
fuel_plane = openmc.XPlane(x0=3.0625, boundary_type="transmission")
graphite_plane = openmc.XPlane(x0=6.9375, boundary_type="transmission")

all_cells = []
ctrlrod_cell = openmc.Cell(fill=ctrlrod)
ctrlrod_cell.region = -top_plane & +bot_plane & +left_plane & -ctrlrod_plane
all_cells.append(ctrlrod_cell)
air_cell = openmc.Cell(fill=air)
air_cell.region = -top_plane & +bot_plane & +ctrlrod_plane & -air_plane
all_cells.append(air_cell)
fuel_cell = openmc.Cell(fill=fuel)
fuel_cell.region = -top_plane & +bot_plane & +air_plane & -fuel_plane
all_cells.append(fuel_cell)
graphite_cell = openmc.Cell(fill=graphite)
graphite_cell.region = -top_plane & +bot_plane & +fuel_plane & -graphite_plane
all_cells.append(graphite_cell)

graphite_bound = 6.9375
for i in range(7):
    fuel_bound = graphite_bound + 1.125
    fuel_plane = openmc.XPlane(x0=fuel_bound, boundary_type="transmission")
    fuel_cell = openmc.Cell(fill=fuel)
    fuel_cell.region = -top_plane & +bot_plane & +graphite_plane & -fuel_plane
    all_cells.append(fuel_cell)
    graphite_bound = fuel_bound + 3.875
    graphite_plane = openmc.XPlane(x0=graphite_bound,
                                   boundary_type="transmission")
    graphite_cell = openmc.Cell(fill=graphite)
    graphite_cell.region = -top_plane & +bot_plane & \
        -graphite_plane & +fuel_plane
    all_cells.append(graphite_cell)

fuel_bound = graphite_bound + 3.0625
fuel_plane = openmc.XPlane(x0=fuel_bound, boundary_type="transmission")
fuel_cell = openmc.Cell(fill=fuel)
fuel_cell.region = -top_plane & +bot_plane & +graphite_plane & -fuel_plane
all_cells.append(fuel_cell)

right_plane = openmc.XPlane(x0=60, boundary_type="vacuum")
reflector_cell = openmc.Cell(fill=reflector)
reflector_cell.region = -top_plane & +bot_plane & +fuel_plane & -right_plane
all_cells.append(reflector_cell)

universe = openmc.Universe(cells=all_cells)

# %% Plot
universe.plot(origin=(60/2, 0.5, 0),
              width=(60, 1),
              color_by="material",
              colors={ctrlrod: "black",
                      air: "cyan",
                      fuel: "red",
                      graphite: "grey",
                      reflector: "lightgrey"})

# %% settings
batches = 150
inactive = 25
particles = 20000

settings = openmc.Settings()
box = openmc.stats.Box((0, 0, 0), (60, 1, 0))
src = openmc.Source(space=box)
# settings.seed = 55
settings.source = src
settings.batches = batches
settings.inactive = inactive
settings.particles = particles
settings.output = {'tallies': False}
settings.temperature = {'multipole': True,
                        'method': 'interpolation'}

# %% Moltres group constants
tallies_file = openmc.Tallies()
mats_id = []
for mat in mats:
    mats_id.append(mat.id)
domain_dict = openmc_xs.generate_openmc_tallies_xml(
    [1e-5, 5.8e-2, 5e-1, 5.0045e3, 4.9787e5, 2e7],
    list(range(1, 7)),
    mats,
    mats_id,
    tallies_file,
)

# %% flux tally
# mesh = openmc.RegularMesh()
# mesh.dimension = [600, 1]
# mesh.lower_left = [0, 0]
# mesh.upper_right = [60, 1]
# 
# mesh_filter = openmc.MeshFilter(mesh)
# energy_boundaries = [1e-5, 1e0, 1e2, 1e5, 1e8]
# energy_filter = openmc.EnergyFilter(energy_boundaries)
# 
# tally = openmc.Tally(name='flux')
# tally.filters = [energy_filter, mesh_filter]
# tally.scores = ['flux']
# tallies_file.append(tally)

# generate XML
mats.export_to_xml()

geom = openmc.Geometry(universe)
geom.export_to_xml()

settings.export_to_xml()
tallies_file.export_to_xml()

# %% Setup MGXS tally

# groups = openmc.mgxs.EnergyGroups(energy_boundaries)
# mgxs_lib = openmc.mgxs.Library(geom)
# mgxs_lib.energy_groups = groups
# mgxs_lib.mgxs_types = ['total', 'absorption', 'nu-fission', 'fission',
#                        'nu-scatter matrix', 'multiplicity matrix', 'chi']
# mgxs_lib.domain_type = "material"
# mgxs_lib.domains = geom.get_all_materials().values()
# mgxs_lib.by_nuclide = False
# mgxs_lib.legendre_order = 2
# mgxs_lib.check_library_for_openmc_mgxs()
# mgxs_lib.build_library()
# 
# # tallies_file = openmc.Tallies()
# mgxs_lib.add_to_tallies_file(tallies_file, merge=False)
# tallies_file.export_to_xml()

# %% Run OpenMC

# openmc.run(threads=8)
