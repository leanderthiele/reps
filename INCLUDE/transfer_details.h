// For CAMB Nov 2015
#define CAMB_TRANSFER_NCOL 13
#define CAMB_TRANSFER_k 0
#define CAMB_TRANSFER_Tb 2
#define CAMB_TRANSFER_Tc 1
#define CAMB_TRANSFER_Tn 5

/* FIXME LFT
 * for current CLASS, there's one more column (which I don't think is needed here)
 * changed CLASS_NU_TRANSFER_NCOL to 12 and CLASS_NONU_TRANSFER_NCOL to 9
 *
 * Note that there are three identical columns in the CLASS output files so it doesn't
 * really matter whether Tn is set to 5, 6, or 7 for 3 neutrino species
 */

// For class v. 2.5.0
#define CLASS_NONU_TRANSFER_NCOL 9 /* LFT from 8 */
#define CLASS_NONU_TRANSFER_k 0
#define CLASS_NONU_TRANSFER_Tb 2
#define CLASS_NONU_TRANSFER_Tc 3
// #define CLASS_NONU_TRANSFER_Tn

#define CLASS_NU_TRANSFER_NCOL 12 /* LFT from 11 */
#define CLASS_NU_TRANSFER_k 0
#define CLASS_NU_TRANSFER_Tb 2
#define CLASS_NU_TRANSFER_Tc 3
#define CLASS_NU_TRANSFER_Tn 5
