extern char use_boundary_conditions_from_file;
extern char boundaryconditionsfile[256];
/* LFT removed this */
//extern char outputfile[200];
extern char output_format[200];
extern char boltzmann_code[200];
extern char boltzmann_folder[500];
extern char neutrino_tab_file[200];
extern char class_precision_file[500];
extern char class_base_par_file[500];

/* LFT added -- this is where all the files go */
extern char workdir[256];

extern int wrong_ic;
extern int wrong_nu;
extern char print_hubble;
extern double step,z_initial,z_final;
extern double *z_output;
extern int output_number;
extern int mode;
extern double H0;
extern double OM0,OB0,OC0,OX0,OG0,OR0,h,M_nu,tau_reio,ns,As,kmax,N_nu,Neff,kpivot;
extern double w0,wa;
extern double Tcmb_0;
extern double Kb;
extern double Gamma_nu;
extern double *ytab;
extern double *FFtab;
extern double *GGtab;
extern double ytab_max;
extern double ytab_min;
extern double F_inf,F_0;
extern double G_inf,G_0;
extern double ytabstep;
extern int ntab;
extern double alpha;
extern int verb;
extern int k_per_logint_camb;
extern char compute_Pk_0;
extern char file_Pk_0_in[200];
