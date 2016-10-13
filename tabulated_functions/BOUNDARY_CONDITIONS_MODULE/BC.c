#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>

/******************************************************************************/
/*    INITIAL SETTINGS - GLOBAL VARIABLES                                     */
/******************************************************************************/
#include "global_variables.h"

/******************************************************************************/
/*    DECLARATION OF FUNCTIONS                                                */
/******************************************************************************/
#include "include.h"
void which_k (double *wn, int n_k, char filename[]);
void lsq(double *x,double **y,int znum,int kindex,double *m,double *q);
void read_ith_pk(double z, int n_k, double *PC, double *PN, char psname[], char tname[]);
void num_deriv(int lines, double *FC, double *FN, double *Pcbminus, double *Pcbplus, double *Pnminus, double *Pnplus, double zminus, double zplus);
void print_help();
int wrong_ic = 0;

/******************************************************************************/
/*        MAIN                                                                */
/******************************************************************************/
int main(int argc, char *argv[])
{
  if (argc!=3)
  {
    if (argc==2)
    {
      if (strcmp(argv[1], "-h")==0 || strcmp(argv[1],"--help")==0 ||
          strcmp(argv[1],"--h")==0 || strcmp(argv[1], "-help")==0 )
      {
        print_help();
        exit(1);
      }
      printf("  Please, choose the kind of boundary conditions\n");
      printf("  required, between: \n");
      printf("    0] correct -numeric- from boltzmann code\n");
      printf("    1] fcb = Omega_m ^ 0.55\n");
      printf("    2] fnu = Omega_m ^ 0.55\n");
      printf("    3] fcb and fnu = Omega_m ^ 0.55\n");
      printf("\n  Your choice: ");
      scanf("%i",&wrong_ic); printf("\n");
      if (wrong_ic != 0 &&
          wrong_ic != 1 &&
          wrong_ic != 2 &&
          wrong_ic != 3)
      {
        printf("Error! Illegal value!\n");
        exit(1);
      }
    }
    else
    {
        printf("Error! You should run ./BC --help and read\n");
        printf("       the help page to have info on the usage of\n");
        printf("       this code.\n");
        exit(1);
    }
  }
  else
  {
    wrong_ic = atoi(argv[2]);
    if (wrong_ic != 0 &&
        wrong_ic != 1 &&
        wrong_ic != 2 &&
        wrong_ic != 3)
    {
      printf("Error! Illegal value!\n");
      exit(1);
    }
  }

  read_GG_FF_tabs();
  read_parameter_file(argv[1]);

  printf("Generating the boundary conditions...\n");

  int bc_nstep=50;
  double zmin = z_fc_in-2.; //99-2
  double zmax = z_fc_in+2.; //99+2
  double xmin = log(1.+zmin);
  double xmax = log(1.+zmax);
  double bc_zz[50];
  double bc_step = (xmax-xmin)/(50.-1.);

  int i;
  for (i=0; i<50; i++)
    bc_zz[i] = exp(xmin + i*bc_step) - 1.;

  char dir_chain[1000];
  if (getcwd(dir_chain,sizeof(dir_chain))==NULL)
  {
    printf("\nError retrieving current directory path.\n");
    exit(1);
  }

  mode=1;
  create_boltzmann_ini_file(dir_chain);
  mode=0;

  printf("Calling %s and creating a tab of power spectra \n",boltzmann_code);
  printf("distributed around z=%lf...\n",z_fc_in);

  char command[200];
  sprintf(command,"%s%s %s/BOUNDARY_CONDITIONS_MODULE/tabs/power.ini > boltzmann.log",
                  boltzmann_folder,boltzmann_code,dir_chain);
  system(command);

  char powfile[200];
  char tfile[200];

  sprintf(powfile,"BOUNDARY_CONDITIONS_MODULE/tabs/power_z1_pk.dat");

  int knum = count_lines(powfile);

  if (strcmp(boltzmann_code,"camb")==0) knum--;

  double **Pcb,**Pn,*k;
  Pcb = allocate_matrix(bc_nstep,knum);
  Pn = allocate_matrix(bc_nstep,knum);
  k = allocate_double_vec(knum);
  which_k(k,knum,powfile);

  for (i=0; i<bc_nstep; i++)
  {
    sprintf(powfile,"BOUNDARY_CONDITIONS_MODULE/tabs/power_z%i_pk.dat",i+1);
    sprintf(tfile,"BOUNDARY_CONDITIONS_MODULE/tabs/power_z%i_tk.dat",i+1);
    read_ith_pk(bc_zz[i],knum,Pcb[i],Pn[i],powfile,tfile);
  }

  double z_der[bc_nstep-1];
  double **fcb, **fn;

  fcb = allocate_matrix(bc_nstep-1,knum);
  fn = allocate_matrix(bc_nstep-1,knum);

  for (i=0; i<(bc_nstep-1); i++)
  {
    z_der[i] = 0.5*(bc_zz[i]+bc_zz[i+1]);
    num_deriv(knum,fcb[i],fn[i],Pcb[i],Pcb[i+1],Pn[i],Pn[i+1],bc_zz[i],bc_zz[i+1]);
  }

  double z_out = z_fc_in;
  double m_cb[knum];
  double q_cb[knum];
  double m_nu[knum];
  double q_nu[knum];

  for (i=0; i<knum; i++)
  {
    lsq(z_der,fcb,bc_nstep-1,i,m_cb,q_cb);
    lsq(z_der,fn,bc_nstep-1,i,m_nu,q_nu);
  }

  mode=2;
  create_boltzmann_ini_file(dir_chain);
  mode=0;

  printf("Calling %s for computing beta at z=%lf...\n",boltzmann_code,z_fc_in);

  sprintf(command,"%s%s %s/BOUNDARY_CONDITIONS_MODULE/tabs/power.ini > boltzmann.log",
                  boltzmann_folder,boltzmann_code,dir_chain);
  system(command);

  double PPcb[knum],PPn[knum];
  read_ith_pk(99., knum, PPcb, PPn, "./BOUNDARY_CONDITIONS_MODULE/tabs/power_zin_pk.dat", "./BOUNDARY_CONDITIONS_MODULE/tabs/power_zin_tk.dat");

  double beta[knum];

  for (i=0; i<knum; i++) beta[i]=sqrt(PPn[i]/PPcb[i]);

  double A, OM99;
  char outfinal_file[300];
  sprintf(outfinal_file,"%s",input_file);
  FILE *outfinal = fopen(outfinal_file,"w");
  if (outfinal==NULL)
  {
    printf("Error creating file %s\n",outfinal_file);
    exit(1);
  }
  if (wrong_ic==0)
  {
    for (i=0; i<knum; i++)
    {
      fprintf(outfinal,"%.12e\t%.12e\t%.12e\t%.12e\n",
      k[i],beta[i],m_cb[i]*z_fc_in+q_cb[i],m_nu[i]*z_fc_in+q_nu[i]);
    }
  }
  else if (wrong_ic==1)
  {
    A = 1./(1.+z_output[output_number-1]);
    OM99 = pow((OM0/(A*A*A))/E2(A,ONE2(A)),0.55);
    for (i=0; i<knum; i++)
    {
      fprintf(outfinal,"%.12e\t%.12e\t%.12e\t%.12e\n",
      k[i],beta[i],OM99,m_nu[i]*z_fc_in+q_nu[i]);
    }
  }
  else if (wrong_ic==2)
  {
    A = 1./(1.+z_output[output_number-1]);
    OM99 = pow((OM0/(A*A*A))/E2(A,ONE2(A)),0.55);
    for (i=0; i<knum; i++)
    {
      fprintf(outfinal,"%.12e\t%.12e\t%.12e\t%.12e\n",
      k[i],beta[i],m_cb[i]*z_fc_in+q_cb[i],OM99);
    }
  }
  else if (wrong_ic==3)
  {
    A = 1./(1.+z_output[output_number-1]);
    OM99 = pow((OM0/(A*A*A))/E2(A,ONE2(A)),0.55);
    for (i=0; i<knum; i++)
    {
      fprintf(outfinal,"%.12e\t%.12e\t%.12e\t%.12e\n",
      k[i],beta[i],OM99,OM99);
    }
  }

  fclose(outfinal);

  printf("Boundary conditions written in file %s\n",input_file);
  system("rm -rf BOUNDARY_CONDITIONS_MODULE/tabs");

  deallocate_matrix(Pcb,bc_nstep,knum);
  deallocate_matrix(Pn,bc_nstep,knum);
  free(k);
  deallocate_matrix(fcb,bc_nstep-1,knum);
  deallocate_matrix(fn,bc_nstep-1,knum);

  return 0;
}

void which_k (double *wn, int n_k, char filename[])
{
  int i; double val;
  int fscanfcheck=0;
  char buf[5000];
  char *dummy;

  FILE *f = fopen(filename,"r");
  if (strcmp(boltzmann_code,"camb")==0) dummy=fgets (buf, sizeof(buf), f);
  for (i=0; i<n_k; i++)
    fscanfcheck=fscanf(f,"%lf %lf",&wn[i],&val);
  fclose(f);
}

void lsq(double *x,double **y,int znum,int kindex,double *m,double *q)
{
  double Sum_x = 0;
  double Sum_y = 0;
  double Sum_x2 = 0;
  double Sum_x_y = 0;

  int i;
  for (i=0; i < znum; i++)
  {
    Sum_x += x[i];
    Sum_y += y[i][kindex];
    Sum_x_y += x[i]*y[i][kindex];
    Sum_x2 += pow(x[i],2);
  }

  int N = znum;

  m[kindex] = (N*Sum_x_y-Sum_x*Sum_y)/(N*Sum_x2-pow(Sum_x,2));

  q[kindex] = (Sum_y*Sum_x2-Sum_x*Sum_x_y)/(N*Sum_x2-pow(Sum_x,2));
}

void num_deriv(int lines, double *FC, double *FN, double *Pcbminus, double *Pcbplus, double *Pnminus, double *Pnplus, double zminus, double zplus)
{
  int i;
  for (i=0; i < lines; i++)
  {
    FC[i] = (log(sqrt(Pcbplus[i]/Pcbminus[i]))/log((1.+zminus)/(1.+zplus)));
    if (N_nu!=0) FN[i] = (log(sqrt(Pnplus[i]/Pnminus[i]))/log((1.+zminus)/(1.+zplus))); else FN[i] = 0.;
  }
}

void read_ith_pk(double z, int n_k, double *PC, double *PN, char psname[], char tname[])
{
  int fscanfcheck=0;

  FILE *ft = fopen(tname,"r");
  if (ft == NULL) {printf("Problem opening file %s!\n",tname); exit(1);}

  int i; double val;
  double Tc[n_k];
  double Tb[n_k];
  double Tn[n_k];
  double Tm[n_k];
  double k[n_k];

  char buf[5000];
  char *dummy;

  double Tc0,Tm0,Tb0,Tn0;

  dummy = fgets(buf, sizeof(buf), ft);

  if (strcmp(boltzmann_code,"camb")==0)
  {
    if (N_nu!=0)
    {
      for(i = 0; i < n_k; i++)
      {
        fscanfcheck=fscanf(ft,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&k[i],&Tc[i],&Tb[i],&val,&val,&Tn[i],&val,&val,&val,&val,&val,&val,&val);
      }
    }
    else
    {
      for(i = 0; i < n_k; i++)
      {
        fscanfcheck=fscanf(ft,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&k[i],&Tc[i],&Tb[i],&val,&val,&Tn[i],&val,&val,&val,&val,&val,&val,&val);
      }
    }
    double Tcb;
    double kpivot=0.05;
    for (i=0; i < n_k; i++)
    {
      Tcb = (OB0/(OB0+OC0))*Tb[i] + (OC0/(OC0+OB0))*Tc[i];
      PC[i] = As*pow((k[i]*h)/(kpivot),ns-1.)*(2.0*M_PI*M_PI*k[i]*(h*h*h*h))*(Tcb*Tcb);
      PN[i] = As*pow((k[i]*h)/(kpivot),ns-1.)*(2.0*M_PI*M_PI*k[i]*(h*h*h*h))*(Tn[i]*Tn[i]);
    }
  }
  else
  {
    if (N_nu==0)
    {
      for(i = 0; i < n_k; i++)
      {
        fscanfcheck=fscanf(ft,"%lf %lf %lf %lf %lf %lf",&k[i],&val,&Tb[i],&Tc[i],&val,&val);
        Tn[i] = 0.0;
      }
    }
    else
    {
      for(i = 0; i < n_k; i++)
      {
        fscanfcheck=fscanf(ft,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&k[i],&val,&Tb[i],&Tc[i],&val,&Tn[i],&val,&val,&val);
      }
    }
    double Tcb;
    double kpivot=0.05;
    for (i=0; i < n_k; i++)
    {
      Tcb = (OB0/(OB0+OC0))*Tb[i] + (OC0/(OC0+OB0))*Tc[i];
      PC[i] = As*pow((k[i]*h)/(kpivot),ns-1.)*(2.0*M_PI*M_PI/(k[i]*k[i]*k[i]))*(Tcb*Tcb);
      PN[i] = As*pow((k[i]*h)/(kpivot),ns-1.)*(2.0*M_PI*M_PI/(k[i]*k[i]*k[i]))*(Tn[i]*Tn[i]);
    }
  }
  fclose(ft);
}

void print_help()
{
  printf("\n\tUsage:\n");
  printf("\t./BC param_file wrong_nu\n");
  printf("\n\tparam_file = the same parameter file needed for rkps\n");
  printf("\twrong_nu = a number chosen among:\n");
  printf("\t  0] correct -numeric- from boltzmann code\n");
  printf("\t  1] fcb = Omega_m ^ 0.55\n");
  printf("\t  2] fnu = Omega_m ^ 0.55\n");
  printf("\t  3] fcb and fnu = Omega_m ^ 0.55\n");
  printf("\n");
}
