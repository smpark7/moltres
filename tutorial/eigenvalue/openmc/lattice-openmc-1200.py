import openmc
import sys
sys.path.insert(1, '../../../python')
from moltres_xs import openmc_xs  # noqa: E402

# %% Materials

fuel = openmc.Material(material_id=101)
# Fuel composition at initial criticality (U235 = 65.25kg)
fuel_dens = 2.1929
fuel.set_density('g/cm3', 2.3275)
fuel_elements = {'Li7': 507.27 * .99995,
                 'Li6': 507.27 * .00005,
                 'Be': 293.96,
                 'F': 3103.22,
                 'Zr': 513.97,
                 'Hf': 0.0029,
                 'Fe': 0.75,
                 'Cr': 0.13,
                 'Ni': 0.14,
                 'O': 2.27,
                 'U234': .67,
                 'U235': 65.25,
                 'U236': .27,
                 'U238': 141.91}
total_fuel_weight = sum(fuel_elements.values())
for i in fuel_elements.keys():
    if len(i) < 3:
        fuel.add_element(i, fuel_elements[i]/total_fuel_weight, 'wo')
    else:
        fuel.add_nuclide(i, fuel_elements[i]/total_fuel_weight, 'wo')
fuel.add_s_alpha_beta('c_Be')
fuel.temperature = 1200

graphite = openmc.Material(material_id=102)
graphite_dens = 1.8585
graphite.set_density('g/cm3', graphite_dens)
graphite.add_nuclide('C0', 1, 'wo')
graphite.add_s_alpha_beta('c_Graphite')
graphite.temperature = 1200

mats = openmc.Materials((fuel, graphite))

# %% Geometry

bot_plane = openmc.ZPlane(z0=0, boundary_type="vacuum")
top_plane = openmc.ZPlane(z0=165, boundary_type="vacuum")
fuel_cyl = openmc.ZCylinder(r=.6, boundary_type="transmission")
graphite_cyl = openmc.ZCylinder(r=4.6, boundary_type="transmission")

all_cells = []
fuel_cell = openmc.Cell(fill=fuel)
fuel_cell.region = -top_plane & +bot_plane & -fuel_cyl
all_cells.append(fuel_cell)
graphite_cell = openmc.Cell(fill=graphite)
graphite_cell.region = -top_plane & +bot_plane & +fuel_cyl & -graphite_cyl
all_cells.append(graphite_cell)

graphite_bound = 4.6
for i in range(13):
    fuel_bound = graphite_bound + 1.2
    fuel_cyl = openmc.ZCylinder(r=fuel_bound, boundary_type="transmission")
    fuel_cell = openmc.Cell(fill=fuel)
    fuel_cell.region = -top_plane & +bot_plane & +graphite_cyl & -fuel_cyl
    all_cells.append(fuel_cell)
    graphite_bound = fuel_bound + 4
    if i == 12:
        graphite_cyl = openmc.ZCylinder(r=graphite_bound,
                                        boundary_type="vacuum")
    else:
        graphite_cyl = openmc.ZCylinder(r=graphite_bound,
                                        boundary_type="transmission")
    graphite_cell = openmc.Cell(fill=graphite)
    graphite_cell.region = -top_plane & +bot_plane & +fuel_cyl & -graphite_cyl
    all_cells.append(graphite_cell)

universe = openmc.Universe(cells=all_cells)

# %% Plot
universe.plot(origin=(2.5, 0.5, 0),
              width=(5, 1),
              color_by="material",
              colors={fuel: "red",
                      graphite: "grey"})

# %% settings
batches = 100
inactive = 20
particles = 10000

settings = openmc.Settings()
box = openmc.stats.Box((-70, -70, 0), (70, 70, 200), only_fissionable=True)
src = openmc.Source(space=box)
settings.source = src
settings.batches = batches
settings.inactive = inactive
settings.particles = particles
settings.output = {'tallies': False}
settings.temperature = {'multipole': True,
                        'method': 'interpolation',
                        'default': 1200.}

# %% Moltres group constants
tallies_file = openmc.Tallies()
mats_id = []
for m in mats:
    mats_id.append(m.id)
domain_dict = openmc_xs.generate_openmc_tallies_xml(
    [1e-5, 1e0, 1e8],
    list(range(1, 7)),
    mats,
    mats_id,
    tallies_file,
)

# generate XML
mats.export_to_xml()

geom = openmc.Geometry(universe)
geom.export_to_xml()

settings.export_to_xml()
tallies_file.export_to_xml()
