/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   LAMMPS development team: developers@lammps.org

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef FIX_CLASS
// clang-format off
FixStyle(rigid/abrade,FixRigidAbrade);
// clang-format on
#else

#ifndef LMP_FIX_RIGID_ABRADE_H
#define LMP_FIX_RIGID_ABRADE_H

#include "fix.h"
#include <map>
#include <vector>
#include <algorithm> 

namespace LAMMPS_NS {

class FixRigidAbrade : public Fix {
  friend class ComputeRigidLocal;

 public:
  FixRigidAbrade(class LAMMPS *, int, char **);
  ~FixRigidAbrade() override;
  int setmask() override;
  void init() override;
  void init_list(int, class NeighList *) override;
  void setup(int) override;
  void initial_integrate(int) override;
  void post_force(int) override;
  void final_integrate() override;
  void end_of_step() override;
  void initial_integrate_respa(int, int, int) override;
  void final_integrate_respa(int, int) override;
  void write_restart_file(const char *) override;
  enum { CONSTANT, EQUAL};

  void grow_arrays(int) override;
  void copy_arrays(int, int, int) override;
  void set_arrays(int) override;
  void set_molecule(int, tagint, int, double *, double *, double *) override;
  int pack_exchange(int, double *) override;
  int unpack_exchange(int, double *) override;
  int pack_forward_comm(int, int *, double *, int, int *) override;
  void unpack_forward_comm(int, int, double *) override;
  int pack_reverse_comm(int, int, double *) override;
  void unpack_reverse_comm(int, int *, double *) override;

  void setup_pre_neighbor() override;
  void pre_neighbor() override;
  bigint dof(int) override;
  void deform(int) override;
  void reset_dt() override;
  void zero_momentum() override;
  void zero_rotation() override;
  int modify_param(int, char **) override;
  void *extract(const char *, int &) override;
  double extract_ke();
  double extract_erotational();
  double compute_scalar() override;
  double memory_usage() override;

  double **vertexdata;   // array to store the normals, areas, and displacement velocities, and cumulative displacement of each atom (public to allow access from pairstyles)
  

private:

  class NeighList *list;
  void areas_and_normals();
  void displacement_of_atom(int, double, double[3], double[3]);
  
  
  
  int allflag, compress_flag, bond_flag, mol_flag;
  

  int setup_surface_density_threshold_flag;
  bool angle_check(int, int, std::vector<std::vector<double>>, std::vector<std::vector<double>>, double[3]);
  void remesh(std::vector<int>);
  
  std::vector<int> dlist;
  std::vector<int> total_dlist;

  int delete_atom_flag;

  bigint lastcheck;
  int equalise_surface_flag = 0;
  
  
  void equalise_surface();

 protected:
  int me, nprocs;
  double dtv, dtf, dtq;
  double *step_respa;
  int triclinic;
  bool proc_abraded_flag; // A flag which marks if any owned or ghost atoms on a processor have been abraded

  // Modified Commflags
  enum{FULL_BODY, INITIAL, FINAL, FORCE_TORQUE, VCM, ANGMOM, XCM_MASS, MASS_NATOMS, DISPLACE, NORMALS, BODYTAG, ITENSOR, UNWRAP, DOF, ABRADED_FLAG, PROC_ABRADED_FLAG, OWNING_ATOMS};

  char *inpfile;       // file to read rigid body attributes from
  int setupflag;       // 1 if body properties are setup, else 0
  int dynamic_flag;    // 0 if bodies are held sationary and prevents the COM from being integrated
  int remesh_flag;    //  1 if bodies are to be remeshed following a change in shape
  int initial_remesh_flag; //  1 if bodies are to be remeshed at the start of the simulation to equalise the area_per_atom
  int earlyflag;       // 1 if forces/torques are computed at post_force()
  int commflag;        // various modes of forward/reverse comm
  int customflag;      // 1 if custom property/variable define bodies
  int nbody;           // total # of rigid bodies
  int nlinear;         // total # of linear rigid bodies
  tagint maxmol;       // max mol-ID
  double maxextent;    // furthest distance from body owner to body atom

  double hardness, fric_coeff, density;       // hardness and friction coefficient and particle density
  int varflag;
  int hstyle, mustyle, densitystyle;
  int hvar, muvar, densityvar;
  char *hstr, *mustr, *densitystr;

  struct Body {
    int natoms;            // total number of atoms in body
    int ilocal;            // index of owning atom
    double mass;           // total mass of body
    double volume;         // total volume of body

    double surface_area; // total surface area of the body
    double surface_density_threshold; // natoms/surface_area density calculated at setup to act as a condition for remeshing
    double min_area_atom; // the smallest area associated with an atom within the body
    tagint min_area_atom_tag; // tag of the atom in the body which has the smallest associated area

    double density;        // mass density of the body
    double xcm[3];         // COM position
    double xgc[3];         // geometric center position - should equal xcm for the assumed homogenous mass density
    double vcm[3];         // COM velocity
    double fcm[3];         // force on COM
    double torque[3];      // torque around COM
    double quat[4];        // quaternion for orientation of body
    double inertia[3];     // 3 principal components of inertia
    double ex_space[3];    // principal axes in space coords
    double ey_space[3];
    double ez_space[3];
    double xgc_body[3];    // geometric center relative to xcm in body coords
    double angmom[3];      // space-frame angular momentum of body
    double omega[3];       // space-frame omega of body
    double conjqm[4];      // conjugate quaternion momentum
    int remapflag[4];      // PBC remap flags
    bool abraded_flag;     // flag which marks that the body has abraded and changed shape
    bool body_remesh_flag;      // flag used to limit one atom being remeshed from a given body for each call of remesh() 
    imageint image;        // image flags of xcm
    imageint dummy;        // dummy entry for better alignment
  
    double old_STD_area; // TODO: This would be better placed as an array initiated and delated around their use in equalise_surface()
    
  
  };

  Body *body;         // list of rigid bodies, owned and ghost
  int nlocal_body;    // # of owned rigid bodies
  int nghost_body;    // # of ghost rigid bodies
  int nmax_body;      // max # of bodies that body can hold
  int bodysize;       // sizeof(Body) in doubles

  // per-atom quantities
  // only defined for owned atoms, except bodyown for own+ghost

  int *bodyown;          // index of body if atom owns a body, -1 if not
  tagint *bodytag;       // ID of body this atom is in, 0 if none
                         // ID = tag of atom that owns body
  int *atom2body;        // index of owned/ghost body this atom is in, -1 if not
                         // can point to original or any image of the body
  imageint *xcmimage;    // internal image flags for atoms in rigid bodies
                         // set relative to in-box xcm of each body
  double **displace;     // displacement of each atom in body coords
  double **unwrap;     // unwrapped coords of each atom in global coords
  int *eflags;           // flags for extended particles
  double **orient;       // orientation vector of particle wrt rigid body
  double **dorient;      // orientation of dipole mu wrt rigid body

  int extended;       // 1 if any particles have extended attributes
  int orientflag;     // 1 if particles store spatial orientation
  int dorientflag;    // 1 if particles store dipole orientation
  int reinitflag;     // 1 if re-initialize rigid bodies between runs

  class AtomVecEllipsoid *avec_ellipsoid;
  class AtomVecLine *avec_line;
  class AtomVecTri *avec_tri;

  // temporary per-body storage

  int **counts;        // counts of atom types in bodies
  double **itensor;    // 6 space-frame components of inertia tensor

  // mass per body, accessed by granular pair styles

  double *mass_body;
  int nmax_mass;

  // Langevin thermostatting
  int langflag;                        // 0/1 = no/yes Langevin thermostat
  double t_start, t_stop, t_period;    // thermostat params
  double **langextra;                  // Langevin thermostat forces and torques
  int maxlang;                         // max size of langextra
  class RanMars *random;               // RNG

  int tstat_flag, pstat_flag;    // 0/1 = no/yes thermostat/barostat

  int t_chain, t_iter, t_order;

  double p_start[3], p_stop[3];
  double p_period[3], p_freq[3];
  int p_flag[3];
  int pcouple, pstyle;
  int p_chain;

  int allremap;            // remap all atoms
  int dilate_group_bit;    // mask for dilation group
  char *id_dilate;         // group name to dilate

  char *id_gravity;    // ID of fix gravity command to add gravity forces
  double *gvec;        // ptr to gravity vector inside the fix

  double p_current[3], p_target[3];

  // molecules added on-the-fly as rigid bodies

  class Molecule **onemols;
  int nmol;

  // class data used by ring communication callbacks

  double rsqfar;

  struct InRvous {
    int me, ilocal;
    tagint atomID, bodyID;
    double x[3];
  };

  struct OutRvous {
    int ilocal;
    tagint atomID;
  };

  // local methods

  void image_shift();
  void set_xv();
  void set_v();
  void create_bodies(tagint *);
  void setup_bodies_static();
  void resetup_bodies_static();
  void setup_bodies_dynamic();
  void apply_langevin_thermostat();
  void compute_forces_and_torques();
  void enforce2d();
  void readfile(int, double **, int *);
  void grow_body();
  void reset_atom2body();

  // callback function for rendezvous communication

  static int rendezvous_body(int, char *, int &, int *&, char *&, void *);

  // debug

  //void check(int);
};

}    // namespace LAMMPS_NS

#endif
#endif
