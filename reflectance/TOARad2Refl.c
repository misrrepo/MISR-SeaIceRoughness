// SurfSeaIce.c
// Read TOA and GMP (Geometric Parameter) data, no (TOA) correction for atmosphere using SMAC,
// instead only do Rayleigh atmospheric correction, save as data and PNG
// Gene Mar 23 Nov 17
//----------------------------
// Ehsan Mosadegh (emosadegh@nevada.unr.edu)
// 24 Nov 2019
// notes: from directory: /home/mare/Projects/MISR/Tools
// to-do:

#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include <png.h>
#include <MisrToolkit.h>
#include <MisrError.h>

#define NO_DATA -999999.0
#define TDROPOUT -999996.0
#define SINC_DIST 10.0
#define BORDER 10
#define DAMPING 3.25
#define VERBOSE 0

typedef struct { // 20
double ah2o, nh2o;
double ao3,  no3; 
double ao2,  no2,  po2;
double aco2, nco2, pco2;
double ach4, nch4, pch4;
double ano2, nno2, pno2;
double aco,  nco,  pco;
double a0u, a1u, a2u ;
double a0s, a1s, a2s, a3s;
double a0T, a1T, a2T, a3T;
double taur,sr;
double a0taup, a1taup ;
double wo, gc;
double a0P,a1P,a2P,a3P;
double a4P,a5P;
double Resa1,Resa2;
double Resa3,Resa4;
double Resr1, Resr2, Resr3;
double Rest1,Rest2;
double Rest3,Rest4;
} coef_atmos_fileObj;

char fname[4][256];
int path = 0;
int orbit = 0;
int block = 0;
int band = 2;
int camera = 1;
int nlines = 512;
int nsamples = 2048;
int zoom = 64;
double *data = 0;
double *sa = 0, *sz = 0, *cfa = 0, *cfz = 0, *ana = 0, *anz = 0, *caa = 0, *caz = 0;
int noData = 0;
int nvalid = 0;
int ndropouts = 0;
double min;
double max;
double mean;
double stddev;
double displayMax = -1.0;
png_structp png_ptr = 0;
png_infop info_ptr = 0;
coef_atmos_fileObj *smac_coefs = 0;
int demlines = 3480;
int demsamples = 3720;
double *psdata = 0, *uwdata = 0, *uo3data = 0;
double solarZenith = -1.0;

int SMAC(double tetas, double tetav, double phis, double phiv, double uh2o, double uo3,
         double taup550, double pression, double toa_reflectance, double *surf_refl, char *fichier_wl);
int toa2surf(void);
int pixel2grid(int path, int block, int line, int sample, int *j, int *i);
char *data2image(double *data, int ny, int nx, int mode);
int write_png(char *fname, char *image, int ny, int nx);
int readGmpFile(char *fname);
int getDataStats(double *data, int nlines, int nsamples);
double sinc(double x);
double *convolve2d(double *data, double *filter, int nlines, int nsamples);
double *zoom2d(double *data, int nlines, int nsamples, int zoom);
double *extendArray(double *data, int nlines, int nsamples, int border);
double *extractArray(double *data, int nlines, int nsamples, int border);
double *zoomArray(double *data, int nlines, int nsamples, int zoom);
int write_data(char *fname, double *data, int nlines, int nsamples);
int read_data(char *fname, double **data, int nlines, int nsamples);

//#####################################################################################################

int pixel2grid(int path, int block, int line, int sample, int *j, int *i)
{
//printf("we're inside pixel2grid\n"); //Ehsan
int status;
char *errs[] = MTK_ERR_DESC;
double lat, lon;
double r = 6371228.0;
double r2 = 2.0 * r;
double c = 626.688125;
double x0 = -2443770.3;
double y0 = -313657.41;
double x, y, z;
double som_x, som_y;

status = MtkBlsToLatLon(path, 275, block, line * 1.0, sample * 1.0, &lat, &lon);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "C: pixel2grid: MtkBlsToLatLon failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) fprintf(stderr, "C: pixel2grid: lat = %.6f, lon = %.6f\n", lat, lon);

lat *= M_PI / 180.0;
lon -= 90.0;
lon *= M_PI / 180.0;
z = sin(M_PI_4 - lat / 2.0);
x = r2 * cos(lon) * z;
y = r2 * sin(lon) * z;
if (VERBOSE) fprintf(stderr, "C: pixel2grid: absolute x = %.6f, y = %.6f\n", x, y);
x = x - x0 + c / 2.0;
y = y - y0 - c / 2.0;
if (VERBOSE) fprintf(stderr, "C: pixel2grid: relative x = %.6f, y = %.6f\n", x, y);
x = x / c;
y = y / c;
*i = rint(x);
*j = rint(-y);

if (VERBOSE) fprintf(stderr, "C: pixel2grid: scaled x = %.6f, y = %.6f\n", x, y);

return 1;
}

//#####################################################################################################

int getPressure(int path, int block, int line, int sample, double *ps, double *uw, double *uo3)
{
//printf("we're inside gerPressure\n"); //Ehsan
int i, j;

if (psdata == 0)
	{
	if (!read_data("ps.dat", &psdata, demlines, demsamples)) return 0;
	}
if (uwdata == 0)
	{
	if (!read_data("uw.dat", &uwdata, demlines, demsamples)) return 0;
	}
if (uo3data == 0)
	{
	if (!read_data("uo3.dat", &uo3data, demlines, demsamples)) return 0;
	}
//printf("go inside pixel2grid\n");
if (!pixel2grid(path, block, line, sample, &j, &i)) return 0;

if (j < 0 || j >= demlines || i < 0 || i >= demsamples)
	{
	//fprintf(stderr, "getPressure: j = %d, i = %d, out of range\n", j, i); //Ehsan
	*ps = 1010.0;
	*uw = 0.098750;
	*uo3 = 0.091667;
	return 1;
	}

*ps = psdata[i + j * demsamples];
*uw = uwdata[i + j * demsamples];
*uo3 = uo3data[i + j * demsamples];

return 1;
}

//#####################################################################################################
//int toa2surf_original(void) {

int toa2surf(void) {
int j, i;
double toa_rad, sunAz, sunZen, camAz, camZen, press, tau, h2o, o3, toa_refl;
char fname[256];
int indx; //Ehsan


if (noData) {
	for (i = 0; i < nlines * nsamples; i ++) data[i] = NO_DATA;
	return 1;
	}

// printf("\n");
// printf("before loop\n");
// printf("toa_rad : %f \n", toa_rad);
// printf("toa_refl: %f \n", toa_refl);

strcpy(fname, "coef_MISR3_CONT.dat");
tau = 0.05;

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++) {
		// indx = i + j * nsamples; // E: why index is this?
		// printf("\n");
		// printf("new index: %d\n", indx);
		// printf("before update\n");
		// printf("toa_rad : %f \n", toa_rad);
		// printf("toa_refl: %f \n", toa_refl);
		//printf("toa2surf: for j, lines= %d, i, samples= %d, nsamples= %d, nlines= %d, index is: %d\n", j, i, nsamples, nlines, indx);
		toa_rad = data[i + j * nsamples]; // toa_rad=is each pixel digital number// data array is filled from previous step in main

		// printf("\n");
		// printf("toa_rad updates\n");
		// printf("toa_rad : %f \n", toa_rad);
		// printf("toa_refl: %f \n", toa_refl);

		sunAz = sa[i + j * nsamples];
		sunAz -= 180.0;
		// E:
		if (sunAz < 0.0) {
		//printf("sunAz condition\n"); //Ehsan
		sunAz += 360.0;
		sunZen = sz[i + j * nsamples];
		}
		// E:
		if (camera == 1) {
			//printf("camera\n"); //Ehsan
			camAz = cfa[i + j * nsamples];
			camZen = cfz[i + j * nsamples];
		}
		else if (camera == 4) {
			camAz = ana[i + j * nsamples];
			camZen = anz[i + j * nsamples];
		}
		else {
			camAz = caa[i + j * nsamples];
			camZen = caz[i + j * nsamples];
		}
		// E: to deal with no data
		if (toa_rad == NO_DATA || sunAz == NO_DATA || sunZen == NO_DATA || camAz == NO_DATA || camZen == NO_DATA) {
			//printf("no data\n");
			data[i + j * nsamples] = NO_DATA;
		}
		// E: to deal with train dropout
		else if (toa_rad == TDROPOUT) {
			//printf("toa_rad\n"); //Ehsan
			data[i + j * nsamples] = TDROPOUT;
		}
		else { // final checking ...?
			// printf("\n");
			// printf("before getPressure\n");
			// printf("toa_rad : %f \n", toa_rad);
			// printf("toa_refl: %f \n", toa_refl);

			//printf("go inside getPressur\n"); //Ehsan
			if (!getPressure(path, block, j, i, &press, &h2o, &o3)) return 0; // we dont need it anymore cos we dont use SMAC anymore?

			// printf("\n");
			// printf("before SMAC toa_rad=surface\n");
			// printf("toa_rad  : %f \n", toa_rad);
			// printf("toa_refl : %f \n", toa_refl);
			// printf("&toa_refl: %f\n", &toa_refl);


            /* Ehsan 31 jan 2020: */
            /* we don't use SMAC anymore; we use toa_rad-->toa_refl directly from here*/
            // from BlockStats2.c code: we convert toa_rad to toa_refl
            if (databuf.data.u16[j][i] < fillbuf.data.u16[0][0] && cfbuf.data.f[j/lf][i/sf] > 0.0) {
                data[index][i + j * nsamples[index]] = (databuf.data.u16[j][i] >> 2) * scalebuf.data.d[0][0] * cfbuf.data.f[j/lf][i/sf];
                nvalid[index] ++;
            }
            else {
                data[index][i + j * nsamples[index]] = -1.0;
            }



            /* renamed toa_refl--> toa_refl */
			//if (!SMAC(sunZen, camZen, sunAz, camAz, h2o, o3, tau, press, toa_rad, &toa_refl, fname)) return 0; // Q- where toa_refl is coming from? m? toa_refl is our output. it is defined in this function and then we pass its mem-add as the arg and then inside of the function it is defined as r_surf which is a ptr
			
			toa_refl =
			if (toa_refl < 0.0) toa_refl = 0.0;
			printf("outside of SMAC surf_refl: %.2lf \n\n", toa_refl);
			data[i + j * nsamples] = toa_refl;
			}
		}
		
return 1;
}

//#####################################################################################################

int SMAC(double tetas, double tetav, double phis, double phiv, double uh2o, double uo3,
         double taup550, double pression, double toa_reflectance, double *surf_refl, char *fichier_wl)  // r_surf is ptr to surf, it is updated inside this function
{ // start with surf_refl==toa_reflectance // Ehsan
// printf("\n");
// printf("in to SMAC\n");
// printf("toa : %lf\n", toa_reflectance);
// printf("surf: %lf\n", surf_refl);

/* Declarations SMAC */
/*-------------------*/
double cksi;
double spherical_albedo;
double m;
double gaseous_transmittance;
double us,uv,dphi;
double wo;

double crd=180./M_PI;
double cdr=M_PI/180.;

double trans_o3,trans_h2o,trans_o2, trans_co2;
double  trans_co, trans_no2,trans_ch4;
double ttetas,ttetav,ksiD;
double atm_ref;

double ak2, ak, e, f, dp, d, b, del, ww, ss, q1, q2, q3, c1, c2, cp1 ;
double cp2, z, x, y, aa1, aa2, aa3 ;

double uo2, uco2, uch4, uco, uno2  ;
double taup,tautot,taurz;
double pressure, Peq ;
double Res_ray, Res_aer, Res_6s;
double ray_phase, ray_ref, aer_ref, aer_phase ;

FILE *coef_filePtr;

if (smac_coefs == 0) {
/* Reservation memoire de la structure */
/*-------------------------------------*/
		
 smac_coefs = (coef_atmos_fileObj *)malloc(sizeof(coef_atmos_fileObj));
 if (!smac_coefs) {
 	fprintf(stderr, "C: SMAC: couldn't malloc smac_coefs\n");
 	return 0;
 	}

/* Lecture des coefficients  */
/*---------------------------*/

	coef_filePtr=fopen(fichier_wl, "r"); // ptr to fileObj==stream
	if (!coef_filePtr){
		fprintf(stderr, "C: SMAC: erreur d'ouverture du fichier de coefficients (E: error opening the coefficient file): %s\n", fichier_wl);
		return 0;
		}

   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->ah2o), &(smac_coefs->nh2o));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->ao3), &(smac_coefs->no3));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->ao2), &(smac_coefs->no2), &(smac_coefs->po2));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->aco2), &(smac_coefs->nco2), &(smac_coefs->pco2));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->ach4), &(smac_coefs->nch4), &(smac_coefs->pch4));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->ano2), &(smac_coefs->nno2), &(smac_coefs->pno2));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->aco), &(smac_coefs->nco), &(smac_coefs->pco));
   fscanf(coef_filePtr, "%lf %lf %lf %lf", &(smac_coefs->a0s), &(smac_coefs->a1s), &(smac_coefs->a2s), &(smac_coefs->a3s));
   fscanf(coef_filePtr, "%lf %lf %lf %lf", &(smac_coefs->a0T), &(smac_coefs->a1T), &(smac_coefs->a2T), &(smac_coefs->a3T));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->taur), &(smac_coefs->sr));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->a0taup), &(smac_coefs->a1taup));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->wo), &(smac_coefs->gc));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->a0P), &(smac_coefs->a1P), &(smac_coefs->a2P));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->a3P), &(smac_coefs->a4P));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->Rest1), &(smac_coefs->Rest2));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->Rest3), &(smac_coefs->Rest4));
   fscanf(coef_filePtr, "%lf %lf %lf", &(smac_coefs->Resr1), &(smac_coefs->Resr2), &(smac_coefs->Resr3));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->Resa1), &(smac_coefs->Resa2));
   fscanf(coef_filePtr, "%lf %lf", &(smac_coefs->Resa3), &(smac_coefs->Resa4));
   
   fclose(coef_filePtr);

	if (0){
		fprintf(stderr, "%f %f\n", (smac_coefs->ah2o),(smac_coefs->nh2o));
		fprintf(stderr, "%f %f\n", (smac_coefs->ao3),(smac_coefs->no3));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->ao2),(smac_coefs->no2),(smac_coefs->po2));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->aco2),(smac_coefs->nco2),(smac_coefs->pco2));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->ach4),(smac_coefs->nch4),(smac_coefs->pch4));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->ano2),(smac_coefs->nno2),(smac_coefs->pno2));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->aco),(smac_coefs->nco),(smac_coefs->pco));
		fprintf(stderr, "%f %f %f %f\n", (smac_coefs->a0s),(smac_coefs->a1s),(smac_coefs->a2s),(smac_coefs->a3s));
		fprintf(stderr, "%f %f %f %f\n", (smac_coefs->a0T),(smac_coefs->a1T),(smac_coefs->a2T),(smac_coefs->a3T));
		fprintf(stderr, "%f %f\n", (smac_coefs->taur),(smac_coefs->sr));
		fprintf(stderr, "%f %f\n", (smac_coefs->a0taup),(smac_coefs->a1taup));
		fprintf(stderr, "%f %f\n", (smac_coefs->wo),(smac_coefs->gc));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->a0P),(smac_coefs->a1P),(smac_coefs->a2P));
		fprintf(stderr, "%f %f\n", (smac_coefs->a3P),(smac_coefs->a4P));
		fprintf(stderr, "%f %f\n", (smac_coefs->Rest1),(smac_coefs->Rest2));
		fprintf(stderr, "%f %f\n", (smac_coefs->Rest3),(smac_coefs->Rest4));
		fprintf(stderr, "%f %f %f\n", (smac_coefs->Resr1),(smac_coefs->Resr2),(smac_coefs->Resr3));
		fprintf(stderr, "%f %f\n", (smac_coefs->Resa1),(smac_coefs->Resa2));
		fprintf(stderr, "%f %f\n", (smac_coefs->Resa3),(smac_coefs->Resa4));
		
		exit(1);
   		}
	}

us = cos (tetas*cdr);
uv = cos (tetav*cdr);
dphi=(phis-phiv)*cdr;
Peq=pression/1013.0;

/*------ 1) air mass */
m =  1./us + 1./uv;

/*------  2) aerosol optical depth in the spectral band, taup  */
taup = (smac_coefs->a0taup) + (smac_coefs->a1taup) * taup550 ;

/*------  3) gaseous transmissions (downward and upward paths)*/
trans_o3 = 1. ;
trans_h2o= 1. ;
trans_o2 = 1. ;
trans_co2= 1. ;
trans_ch4= 1. ;
 
uo2=  pow (Peq , (smac_coefs->po2));
uco2= pow (Peq , (smac_coefs->pco2));
uch4= pow (Peq , (smac_coefs->pch4));
uno2= pow (Peq , (smac_coefs->pno2));
uco = pow (Peq , (smac_coefs->pco));

/*------  4) if uh2o <= 0 and uo3 <= 0 no gaseous absorption is computed*/
if( (uh2o > 0.) || ( uo3 > 0.) ) {
    trans_o3   = exp ((smac_coefs->ao3) * pow ((uo3 * m)  , (smac_coefs->no3)  ) ) ;
    trans_h2o  = exp ((smac_coefs->ah2o) * pow ((uh2o * m)  , (smac_coefs->nh2o) ) ) ;
    trans_o2   = exp ((smac_coefs->ao2) * pow ((uo2 * m)  , (smac_coefs->no2)  ) ) ;
    trans_co2  = exp ((smac_coefs->aco2) * pow ((uco2 * m)  , (smac_coefs->nco2) ) ) ;
    trans_ch4  = exp ((smac_coefs->ach4) * pow ((uch4 * m)  , (smac_coefs->nch4) ) ) ;
    trans_no2  = exp ((smac_coefs->ano2) * pow ((uno2 * m)  , (smac_coefs->nno2) ) ) ;
    trans_co   = exp ((smac_coefs->aco) * pow ((uco * m)   , (smac_coefs->nco) ) ) ;
}
 
/*------  5) Total scattering transmission */
ttetas = (smac_coefs->a0T) + (smac_coefs->a1T)*taup550/us + ((smac_coefs->a2T)*Peq + (smac_coefs->a3T))/(1.+us) ; /* downward */
ttetav = (smac_coefs->a0T) + (smac_coefs->a1T)*taup550/uv + ((smac_coefs->a2T)*Peq + (smac_coefs->a3T))/(1.+uv) ; /* upward   */

/*------  6) spherical albedo of the atmosphere */
spherical_albedo = (smac_coefs->a0s) * Peq +  (smac_coefs->a3s) + (smac_coefs->a1s)*taup550 + (smac_coefs->a2s) *pow (taup550 , 2) ;

/*------  7) scattering angle cosine */
cksi = - ( (us*uv) + (sqrt(1. - us*us) * sqrt (1. - uv*uv)*cos(dphi) ) );
if (cksi < -1 ) cksi=-1.0 ;

/*------  8) scattering angle in degree */
ksiD = crd*acos(cksi) ;

/*------  9) rayleigh atmospheric reflectance */
/* pour 6s on a delta = 0.0279 */
ray_phase = 0.7190443 * (1. + (cksi*cksi))  + 0.0412742 ;

taurz=(smac_coefs->taur)*Peq;

ray_ref   = ( taurz*ray_phase ) / (4.*us*uv) ;

/*-----------------Residu Rayleigh ---------*/
Res_ray= (smac_coefs->Resr1) + (smac_coefs->Resr2) * taurz*ray_phase / (us*uv) + 
         (smac_coefs->Resr3) * pow( (taurz*ray_phase/(us*uv)),2); 

/*------  10) aerosol atmospheric reflectance */
aer_phase = (smac_coefs->a0P) + (smac_coefs->a1P)*ksiD + (smac_coefs->a2P)*ksiD*ksiD +(smac_coefs->a3P)*pow(ksiD,3) + (smac_coefs->a4P) * pow(ksiD,4); 

ak2 = (1. - (smac_coefs->wo))*(3. - (smac_coefs->wo)*3*(smac_coefs->gc)) ;
ak  = sqrt(ak2) ;
e   = -3.*us*us*(smac_coefs->wo) /  (4.*(1. - ak2*us*us) ) ;
f   = -(1. - (smac_coefs->wo))*3.*(smac_coefs->gc)*us*us*(smac_coefs->wo) / (4.*(1. - ak2*us*us) ) ;
dp  = e / (3.*us) + us*f ;
d   = e + f ;
b   = 2.*ak / (3. - (smac_coefs->wo)*3*(smac_coefs->gc));
del = exp( ak*taup )*(1. + b)*(1. + b) - exp(-ak*taup)*(1. - b)*(1. - b) ;
ww  = (smac_coefs->wo)/4.;
ss  = us / (1. - ak2*us*us) ;
q1  = 2. + 3.*us + (1. - (smac_coefs->wo))*3.*(smac_coefs->gc)*us*(1. + 2.*us) ;
q2  = 2. - 3.*us - (1. - (smac_coefs->wo))*3.*(smac_coefs->gc)*us*(1. - 2.*us) ;
q3  = q2*exp( -taup/us ) ;
c1  =  ((ww*ss) / del) * ( q1*exp(ak*taup)*(1. + b) + q3*(1. - b) ) ;
c2  = -((ww*ss) / del) * (q1*exp(-ak*taup)*(1. - b) + q3*(1. + b) ) ;
cp1 =  c1*ak / ( 3. - (smac_coefs->wo)*3.*(smac_coefs->gc) ) ;
cp2 = -c2*ak / ( 3. - (smac_coefs->wo)*3.*(smac_coefs->gc) ) ;
z   = d - (smac_coefs->wo)*3.*(smac_coefs->gc)*uv*dp + (smac_coefs->wo)*aer_phase/4. ;
x   = c1 - (smac_coefs->wo)*3.*(smac_coefs->gc)*uv*cp1 ;
y   = c2 - (smac_coefs->wo)*3.*(smac_coefs->gc)*uv*cp2 ;
aa1 = uv / (1. + ak*uv) ;
aa2 = uv / (1. - ak*uv) ;
aa3 = us*uv / (us + uv) ;

aer_ref = x*aa1* (1. - exp( -taup/aa1 ) ) ;
aer_ref = aer_ref + y*aa2*( 1. - exp( -taup / aa2 )  ) ;
aer_ref = aer_ref + z*aa3*( 1. - exp( -taup / aa3 )  ) ;
aer_ref = aer_ref / ( us*uv );

/*--------Residu Aerosol --------*/
Res_aer= ( (smac_coefs->Resa1) + (smac_coefs->Resa2) * ( taup * m *cksi ) + (smac_coefs->Resa3) * pow( (taup*m*cksi ),2) ) + (smac_coefs->Resa4) * pow( (taup*m*cksi),3);


/*---------Residu 6s-----------*/
tautot = taup + taurz;
Res_6s= ( (smac_coefs->Rest1) + (smac_coefs->Rest2) * ( tautot * m *cksi ) 
        + (smac_coefs->Rest3) * pow( (tautot*m*cksi),2) ) + (smac_coefs->Rest4) * pow( (tautot*m*cksi),3);

/*------  11) total atmospheric reflectance */

atm_ref = ray_ref - Res_ray + aer_ref - Res_aer + Res_6s;

/*-------- reflectance at toa*/ //gaseus transmittance

gaseous_transmittance      = trans_h2o * trans_o3 * trans_o2 * trans_co2 * trans_ch4 * trans_co * trans_no2;



 /* reflectance at surface */
 /* note: SMAC needs toa_reflectance value, we need to convert toa_radiance to toa_reflectance before using SMAC */
/*------------------------*/
// E: what is gaseous_transmittance?
// E: it says refl at surf, but how should we calculate it????????
*surf_refl = toa_reflectance - (atm_ref * gaseous_transmittance); // what is this?
printf("*surf_refl: %.2lf | toa_reflectance: %.2lf | atm_ref: %lf  | gaseous_transmittance: %lf \n" , *surf_refl , toa_reflectance, atm_ref, gaseous_transmittance);

*surf_refl = *surf_refl / ((gaseous_transmittance * ttetas * ttetav) + (*surf_refl * spherical_albedo));
printf("gaseous_transmittance: %.2lf, ttetas: %.2lf, ttetav: %.2lf, spherical_albedo: %.2lf \n" , gaseous_transmittance, ttetas, ttetav, spherical_albedo);

printf("toa_reflectance: %.2lf | surf_refl: %.2lf \n" , toa_reflectance, *surf_refl); // why surf_refl is larger than toa-rad?

// E: commented the below lines, and used the above lines

  // if (band != 3) {
  // 	*surf_refl = toa_reflectance - ray_ref;} // toa_reflectance is input to this function

  // else {
  // 	*surf_refl = toa_reflectance;}
  
// printf("\n");
// printf("out of SMAC \n");
// printf("toa : %lf \n", toa_reflectance);

return 1;
}    

//#####################################################################################################

char *data2image(double *data, int nlines, int nsamples, int mode)
{
char *image;
double min, max, z, dz;
int i;

if (displayMax > 0.0) 
	{
	max = displayMax;
	min = 0.0;
	}
else
	{
	max = -1.0e23;
	for (i = 0; i < nlines * nsamples; i ++)
		{
		z = data[i];
		if (z == NO_DATA) continue;
		if (z == TDROPOUT) continue;
		if (z > max) max = z;
		}
	if (max == -1.0e23)
		{
		if (VERBOSE) fprintf(stderr, "C: data2image: no valid data\n");
		max = 0.0;
		//return 0;
		}
	if (mode > 0) 
		{
		min = 1.0e23;
		for (i = 0; i < nlines * nsamples; i ++)
			{
			z = data[i];
			if (z == NO_DATA) continue;
			if (z == TDROPOUT) continue;
			if (z < min) min = z;
			}
		if (min == 1.0e23)
			{
			if (VERBOSE) fprintf(stderr, "C: data2image: no valid data\n");
			min = 0.0;
			//return 0;
			}
		}
	else min = 0.0;
	}
if (VERBOSE) fprintf(stderr, "C: data2image: min=%.3f, max=%.3f\n", min, max);
if (max != min) dz =  255.0 / (max - min);
else dz = 0.0;

image = (char *) malloc(nlines * nsamples);
if (!image)
	{
	fprintf(stderr, "C: data2image: couldn't malloc image\n");
	return 0;
	}

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z != NO_DATA && z != TDROPOUT) 
		{
		z = (z - min) * dz;
		if (z > 255.0) image[i] = 255;
		else if (z < 0.0) image[i] = 0;
		else image[i] = z;
		}
	else image[i] = 0;
	}

return image;
}

//#####################################################################################################

int write_png(char *fname, char *image, int ny, int nx)
{
FILE *fp;
png_bytepp row_ptrs;
int j;

if (!image)
	{
	fprintf(stderr, "C: write_png: null image\n");
	return 0;
	}

row_ptrs = (png_bytepp) malloc(ny * sizeof(png_bytep));
if (!row_ptrs)
	{
	fprintf(stderr, "C: write_png: couldn't malloc row_ptrs\n");
	return 0;
	}
for (j = 0; j < ny; j ++) row_ptrs[j] = (png_bytep)(image + j * nx);

fp = fopen(fname, "wb");
if (!fp)
	{
	fprintf(stderr, "C: write_png: couldn't open %s\n", fname);
	return 0;
	}

png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
	png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
if (!png_ptr)
	{
	fclose(fp);
	fprintf(stderr, "C: write_png: png_create_write_struct failed\n");
	return 0;
	}

info_ptr = png_create_info_struct(png_ptr);
if (!info_ptr)
	{
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
	fprintf(stderr, "C: write_png: png_create_info_struct failed\n");
	return 0;
	}

if (setjmp(png_jmpbuf(png_ptr)))
	{
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	fprintf(stderr, "C: write_png: longjmp from png error\n");
	return 0;
	}

png_init_io(png_ptr, fp);

png_set_IHDR(png_ptr, info_ptr, nx, ny, 8, PNG_COLOR_TYPE_GRAY, 
	PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
png_set_rows(png_ptr, info_ptr, row_ptrs);

png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
       
png_destroy_write_struct(&png_ptr, &info_ptr);

fclose(fp);
if (row_ptrs) free(row_ptrs);
return 1;
}

//#####################################################################################################

int readGmpFile(char *fname)
{
MTKt_DataBuffer tmpbuf1 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf2 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf3 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf4 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf5 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf6 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf7 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf8 = MTKT_DATABUFFER_INIT;
char gridName[256];
char fieldName[256];
int status;
MTKt_FileType filetype;
char *types[] = MTKd_DataType;
char *errs[] = MTK_ERR_DESC;
int i, j, n;
double *tmp = 0;
double *saz = 0;
double *cfaz = 0;
double *caaz = 0;
double *raz = 0;
double araz;
double sum;
//int zoom = 64, wrap = 0;
int wrap = 0;

if (VERBOSE) fprintf(stderr, "\n C: readGmpFile: fname=%s, block=%d, band=%i \n", fname, block, band);

status = MtkFileType(fname, &filetype);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "C: readGmpFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (filetype != MTK_GP_GMP)
	{
	fprintf(stderr, "C: readGmpFile: only L1B2 Geometric Parameters files are supported!!!\n");
	return 0;
	}

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "SolarAzimuth");
if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf1);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf1.nline, tmpbuf1.nsample, tmpbuf1.datasize, tmpbuf1.datatype, types[tmpbuf1.datatype]);
if (tmpbuf1.nline != 8 || tmpbuf1.nsample != 32)
	{
	fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf1.nline, tmpbuf1.nsample);
	return 0;
	}

saz = (double *) malloc(8 * 32 * sizeof(double));
if (!saz)
	{
	fprintf(stderr, "C: readGmpFile: saz malloc failed!!!\n");
	return 0;
	}

tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
	return 0;
	}
n = 0;
for (j = 0; j < 8; j ++)
	for (i = 0; i < 32; i ++)
		{
		if (tmpbuf1.data.d[j][i] >= 0.0) 
			{
			n ++;
			tmp[i + j * 32] = tmpbuf1.data.d[j][i];
			saz[i + j * 32] = tmpbuf1.data.d[j][i];
			}
		else 
			{
			tmp[i + j * 32] = NO_DATA;
			saz[i + j * 32] = NO_DATA;
			}
		}
if (n != 256)
	{
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
//getDataStats(tmp, 8, 32);
//if (!write_png("sa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;

if (n > 0)
	{
	getDataStats(tmp, 8, 32);
	if (min <= 30.0 && max >= 330.0)
		{
		for (i = 0; i < 8 * 32; i ++)
			{
			if (tmp[i] == NO_DATA) continue;
			if (tmp[i] < 180.0) tmp[i] += 360.0;
			}
		wrap = 1;
		}
	sa = zoomArray(tmp, 8, 32, zoom);
	if (!sa) return 0;
	if (wrap)
		{
		//for (i = 0; i < 512 * 2048; i ++)
		for (i = 0; i < nlines * nsamples; i ++)
			{
			if (sa[i] == NO_DATA) continue;
			if (sa[i] >= 360.0) sa[i] -= 360.0;
			}
		wrap = 0;
		}
	}
else
	{
	noData = 1;
	MtkDataBufferFree(&tmpbuf1);
	if (tmp) free(tmp);
	return 1;
	}
//getDataStats(sa, 512, 2048, "sa2");
//if (!write_png("sa2.png", data2image(sa, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf1);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "SolarZenith");
if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf2);
if (status != MTK_SUCCESS) 
	{
	fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf2.nline, tmpbuf2.nsample, tmpbuf2.datasize, tmpbuf2.datatype, types[tmpbuf2.datatype]);
if (tmpbuf2.nline != 8 || tmpbuf2.nsample != 32)
	{
	fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf2.nline, tmpbuf2.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
	return 0;
	}
n = 0;
for (j = 0; j < 8; j ++)
	for (i = 0; i < 32; i ++)
		{
		if (tmpbuf2.data.d[j][i] >= 0.0) 
			{
			n ++;
			tmp[i + j * 32] = tmpbuf2.data.d[j][i];
			}
		else tmp[i + j * 32] = NO_DATA;
		}
if (n != 256)
	{
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
//getDataStats(tmp, 8, 32, "sz1");
//if (!write_png("sz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
if (n > 0)
	{
	getDataStats(tmp, 8, 32);
	solarZenith = mean;
	sz = zoomArray(tmp, 8, 32, zoom);
	if (!sz) return 0;
	}
else
	{
	noData = 1;
	MtkDataBufferFree(&tmpbuf2);
	if (tmp) free(tmp);
	return 1;
	}
//getDataStats(sz, 512, 2048, "sz2");
//if (!write_png("sz2.png", data2image(sz, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf2);
if (tmp) free(tmp);

if (camera == 1)
	{
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "CfAzimuth");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf3);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf3.nline, tmpbuf3.nsample, tmpbuf3.datasize, tmpbuf3.datatype, types[tmpbuf3.datatype]);
	if (tmpbuf3.nline != 8 || tmpbuf3.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf3.nline, tmpbuf3.nsample);
		return 0;
		}

	raz = (double *) malloc(8 * 32 * sizeof(double));
	if (!raz)
		{
		fprintf(stderr, "C: readGmpFile: raz malloc failed!!!\n");
		return 0;
		}
	cfaz = (double *) malloc(8 * 32 * sizeof(double));
	if (!cfaz)
		{
		fprintf(stderr, "C: readGmpFile: cfaz malloc failed!!!\n");
		return 0;
		}

	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf3.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf3.data.d[j][i];
				cfaz[i + j * 32] = tmpbuf3.data.d[j][i];
				raz[i + j * 32] = cfaz[i + j * 32] - saz[i + j * 32];
				}
			else 
				{
				tmp[i + j * 32] = NO_DATA;
				cfaz[i + j * 32] = NO_DATA;
				raz[i + j * 32] = NO_DATA;
				}
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
/***
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			fprintf(stderr, "%.1f ", cfaz[i + j * 32]);
			}
	fprintf(stderr, "\n\n");
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			fprintf(stderr, "%.1f ", saz[i + j * 32]);
			}

	fprintf(stderr, "\n\n");
	n = 0;
	sum = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (raz[i + j * 32] >= 0.0)
				{
				n++;
				sum += raz[i + j * 32];
				}
			araz = sum/n;
			fprintf(stderr, "%.1f ", raz[i + j * 32]);
			}
	fprintf(stderr, "\nAverage Cf Relative Azimuth = %.1f\n\n", araz);
***/
	//getDataStats(tmp, 8, 32);
	//if (!write_png("cfa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;

	if (n > 0)
		{
		getDataStats(tmp, 8, 32);
		if (min <= 30.0 && max >= 330.0)
			{
			for (i = 0; i < 8 * 32; i ++)
				{
				if (tmp[i] == NO_DATA) continue;
				if (tmp[i] < 180.0) tmp[i] += 360.0;
				}
			wrap = 1;
			}
		cfa = zoomArray(tmp, 8, 32, zoom);
		if (!cfa) return 0;
		if (wrap)
			{
			//for (i = 0; i < 512 * 2048; i ++)
			for (i = 0; i < nlines * nsamples; i ++)
				{
				if (cfa[i] == NO_DATA) continue;
				if (cfa[i] >= 360.0) cfa[i] -= 360.0;
				}
			wrap = 0;
			}
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf3);
		if (tmp) free(tmp);
		return 1;
		}
	//getDataStats(cfa, 512, 2048, "cfa2");
	//if (!write_png("cfa2.png", data2image(cfa, 512, 2048, 1), 512, 2048)) return 0;
	MtkDataBufferFree(&tmpbuf3);
	if (tmp) free(tmp);
	
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "CfZenith");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf4);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf4.nline, tmpbuf4.nsample, tmpbuf4.datasize, tmpbuf4.datatype, types[tmpbuf4.datatype]);
	if (tmpbuf4.nline != 8 || tmpbuf4.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf4.nline, tmpbuf4.nsample);
		return 0;
		}
	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf4.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf4.data.d[j][i];
				}
			else tmp[i + j * 32] = NO_DATA;
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
	//getDataStats(tmp, 8, 32, "cfz1");
	//if (!write_png("cfz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
	if (n > 0)
		{
		cfz = zoomArray(tmp, 8, 32, zoom);
		if (!cfz) return 0;
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf4);
		if (tmp) free(tmp);
		return 1;
		}
	//getDataStats(cfz, 512, 2048, "cfz2");
	//if (!write_png("cfz2.png", data2image(cfz, 512, 2048, 1), 512, 2048)) return 0;
	MtkDataBufferFree(&tmpbuf4);
	if (tmp) free(tmp);
	}

if (camera == 4)
	{
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "AnAzimuth");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf7);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf7.nline, tmpbuf7.nsample, tmpbuf7.datasize, tmpbuf7.datatype, types[tmpbuf7.datatype]);
	if (tmpbuf7.nline != 8 || tmpbuf7.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf7.nline, tmpbuf7.nsample);
		return 0;
		}
	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf7.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf7.data.d[j][i];
				}
			else tmp[i + j * 32] = NO_DATA;
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
	//getDataStats(tmp, 8, 32, "cfa1");
	//if (!write_png("cfa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
	if (n > 0)
		{
		getDataStats(tmp, 8, 32);
		if (min <= 30.0 && max >= 330.0)
			{
			for (i = 0; i < 8 * 32; i ++)
				{
				if (tmp[i] == NO_DATA) continue;
				if (tmp[i] < 180.0) tmp[i] += 360.0;
				}
			wrap = 1;
			}
		ana = zoomArray(tmp, 8, 32, zoom);
		if (!ana) return 0;
		if (wrap)
			{
			//for (i = 0; i < 512 * 2048; i ++)
			for (i = 0; i < nlines * nsamples; i ++)
				{
				if (ana[i] == NO_DATA) continue;
				if (ana[i] >= 360.0) ana[i] -= 360.0;
				}
			wrap = 0;
			}
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf7);
		if (tmp) free(tmp);
		return 1;
		}
	MtkDataBufferFree(&tmpbuf7);
	if (tmp) free(tmp);
	
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "AnZenith");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf8);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf8.nline, tmpbuf8.nsample, tmpbuf8.datasize, tmpbuf8.datatype, types[tmpbuf8.datatype]);
	if (tmpbuf8.nline != 8 || tmpbuf8.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf8.nline, tmpbuf8.nsample);
		return 0;
		}
	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf8.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf8.data.d[j][i];
				}
			else tmp[i + j * 32] = NO_DATA;
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
	//getDataStats(tmp, 8, 32, "cfz1");
	//if (!write_png("cfz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
	if (n > 0)
		{
		anz = zoomArray(tmp, 8, 32, zoom);
		if (!anz) return 0;
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf8);
		if (tmp) free(tmp);
		return 1;
		}
	//getDataStats(anz, 512, 2048, "cfz2");
	//if (!write_png("cfz2.png", data2image(anz, 512, 2048, 1), 512, 2048)) return 0;
	MtkDataBufferFree(&tmpbuf8);
	if (tmp) free(tmp);
	}

if (camera == 7)
	{
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "CaAzimuth");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf5);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf5.nline, tmpbuf5.nsample, tmpbuf5.datasize, tmpbuf5.datatype, types[tmpbuf5.datatype]);
	if (tmpbuf5.nline != 8 || tmpbuf5.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf5.nline, tmpbuf5.nsample);
		return 0;
		}

	raz = (double *) malloc(8 * 32 * sizeof(double));
	if (!raz)
		{
		fprintf(stderr, "C: readGmpFile: raz malloc failed!!!\n");
		return 0;
		}
	caaz = (double *) malloc(8 * 32 * sizeof(double));
	if (!caaz)
		{
		fprintf(stderr, "C: readGmpFile: caaz malloc failed!!!\n");
		return 0;
		}

	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf5.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf5.data.d[j][i];
				caaz[i + j * 32] = tmpbuf5.data.d[j][i];
				raz[i + j * 32] = caaz[i + j * 32] - saz[i + j * 32]; 
				}
			else 
				{
				tmp[i + j * 32] = NO_DATA;
				caaz[i + j * 32] = NO_DATA;
				raz[i + j * 32] = NO_DATA;
				}
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
/***
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			fprintf(stderr, "%.1f ", caaz[i + j * 32]);
			}
	fprintf(stderr, "\n\n");
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			fprintf(stderr, "%.1f ", saz[i + j * 32]);
			}
	fprintf(stderr, "\n\n");
	n = 0;
	sum = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (raz[i + j * 32] >= 0.0)
				{
				n++;
				sum += raz[i + j * 32];
				}
			araz = sum/n;
			fprintf(stderr, "%.1f ", raz[i + j * 32]);
			}
	fprintf(stderr, "\nAverage Ca Relative Azimuth = %.1f\n\n", araz);
***/
	//getDataStats(tmp, 8, 32, "caa1");
	//if (!write_png("caa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
	if (n > 0)
		{
		getDataStats(tmp, 8, 32);
		if (min <= 30.0 && max >= 330.0)
			{
			for (i = 0; i < 8 * 32; i ++)
				{
				if (tmp[i] == NO_DATA) continue;
				if (tmp[i] < 180.0) tmp[i] += 360.0;
				}
			wrap = 1;
			}
		caa = zoomArray(tmp, 8, 32, zoom);
		if (!caa) return 0;
		if (wrap)
			{
			//for (i = 0; i < 512 * 2048; i ++)
			for (i = 0; i < nlines * nsamples; i ++)
				{
				if (caa[i] == NO_DATA) continue;
				if (caa[i] >= 360.0) caa[i] -= 360.0;
				}
			wrap = 0;
			}
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf5);
		if (tmp) free(tmp);
		return 1;
		}
	//getDataStats(caa, 512, 2048, "caa2");
	//if (!write_png("caa2.png", data2image(caa, 512, 2048, 1), 512, 2048)) return 0;
	MtkDataBufferFree(&tmpbuf5);
	if (tmp) free(tmp);
	
	strcpy(gridName, "GeometricParameters"); 
	strcpy(fieldName, "CaZenith");
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
	status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf6);
	if (status != MTK_SUCCESS) 
		{
		fprintf(stderr, "C: readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
		return 0;
		}
	if (VERBOSE) fprintf(stderr, "C: readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
		tmpbuf6.nline, tmpbuf6.nsample, tmpbuf6.datasize, tmpbuf6.datatype, types[tmpbuf6.datatype]);
	if (tmpbuf6.nline != 8 || tmpbuf6.nsample != 32)
		{
		fprintf(stderr, "C: readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf6.nline, tmpbuf6.nsample);
		return 0;
		}
	tmp = (double *) malloc(8 * 32 * sizeof(double));
	if (!tmp)
		{
		fprintf(stderr, "C: readGmpFile: tmp malloc failed!!!\n");
		return 0;
		}
	n = 0;
	for (j = 0; j < 8; j ++)
		for (i = 0; i < 32; i ++)
			{
			if (tmpbuf6.data.d[j][i] >= 0.0) 
				{
				n ++;
				tmp[i + j * 32] = tmpbuf6.data.d[j][i];
				}
			else tmp[i + j * 32] = NO_DATA;
			}
	if (n != 256)
		{
		if (VERBOSE) fprintf(stderr, "C: readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
		//return 0;
		}
	//getDataStats(tmp, 8, 32, "caz1");
	//if (!write_png("caz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
	if (n > 0)
		{
		caz = zoomArray(tmp, 8, 32, zoom);
		if (!caz) return 0;
		}
	else
		{
		noData = 1;
		MtkDataBufferFree(&tmpbuf6);
		if (tmp) free(tmp);
		return 1;
		}
	//getDataStats(caz, 512, 2048, "caz2");
	//if (!write_png("caz2.png", data2image(caz, 512, 2048, 1), 512, 2048)) return 0;
	MtkDataBufferFree(&tmpbuf6);
	if (tmp) free(tmp);
	}

return 1;
}

//#####################################################################################################

int getDataStats(double *data, int nlines, int nsamples)
{
int i;
double z;

min = 1.0e23;
max = -1.0e23;
mean = 0.0;
stddev = 0.0;
nvalid = 0;
ndropouts = 0;

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) continue;
	if (z == TDROPOUT) 
		{
		ndropouts ++;
		continue;
		}
	nvalid ++;
	if (z < min) min = z;
	if (z > max) max = z;
	mean += z;
	}
	
if (nvalid > 0) 
	{
	mean /= nvalid;
	if (nvalid > 1)
		{
		for (i = 0; i < nlines * nsamples; i ++)
			{
			z = data[i];
			if (z == NO_DATA) continue;
			if (z == TDROPOUT) continue;
			stddev += (z - mean) * (z - mean);
			}
		stddev = sqrt(stddev / (nvalid - 1.0));
		}
	}

return 1;
}

//#####################################################################################################

double sinc(double x)
{
if (x == 0.0) return 1.0;
x *= M_PI;
return sin(x) / x;
}

//#####################################################################################################

double *convolve2d(double *data, double *filter, int nlines, int nsamples)
{
double *result;
int i, j, i0, j0, n, n2;
double *rbuf1, *rbuf2;
fftw_complex *cbuf1, *cbuf2, *cbuf3;
fftw_plan p;

n = nsamples * nlines;
n2 = (nsamples / 2 + 1) * nlines;
rbuf1 = (double *) fftw_malloc(n * sizeof(double));
if (!rbuf1)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc rbuf1\n");
	return 0;
	}
rbuf2 = (double *) fftw_malloc(n * sizeof(double));
if (!rbuf2)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc rbuf2\n");
	return 0;
	}
cbuf1 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf1)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc cbuf1\n");
	return 0;
	}
cbuf2 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf2)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc cbuf2\n");
	return 0;
	}
cbuf3 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
if (!cbuf3)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc cbuf3\n");
	return 0;
	}

for (i = 0; i < n; i ++) rbuf1[i] = data[i];
for (i = 0; i < n; i ++) rbuf2[i] = filter[i];

if (VERBOSE > 1) fprintf(stderr, "C fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf1, cbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "fftw_execute\n");	
fftw_execute(p);
fftw_destroy_plan(p);

if (VERBOSE > 1) fprintf(stderr, "C fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf2, cbuf2, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "C fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) fprintf(stderr, "C fftw_destroy_plan\n");	
fftw_destroy_plan(p);

for (i = 0; i < n2; i ++)
	{
	cbuf3[i][0] = cbuf1[i][0] * cbuf2[i][0] - cbuf1[i][1] * cbuf2[i][1];
	cbuf3[i][1] = cbuf1[i][0] * cbuf2[i][1] + cbuf1[i][1] * cbuf2[i][0];
	}

if (VERBOSE > 1) fprintf(stderr, "C fftw_plan_dft_c2r_2d\n");	
p = fftw_plan_dft_c2r_2d(nlines, nsamples, cbuf3, rbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) fprintf(stderr, "C fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) fprintf(stderr, "C fftw_destroy_plan\n");	
fftw_destroy_plan(p);

result = (double *) malloc(nsamples * nlines * sizeof(double));
if (!result)
	{
	fprintf(stderr, "C convolve2d: couldn't malloc result\n");
	return 0;
	}
	
j0 = nlines / 2;
i0 = nsamples / 2;
for (j = 0; j < nlines; j ++) 
	for (i = 0; i < nsamples; i ++) 
		result[j * nsamples + i] = 
			rbuf1[((j0 + j) % nlines) * nsamples + ((i0 + i) % nsamples)] / n;

fftw_free(rbuf1);
fftw_free(rbuf2);
fftw_free(cbuf1);
fftw_free(cbuf2);
fftw_free(cbuf3);

return result;
}

//#####################################################################################################

double *filter = 0;

double *zoom2d(double *data, int nlines, int nsamples, int zoom)
{
double dx, dy, *data2, *result, dd2 = DAMPING * DAMPING;
int i, j, i0, j0, nlines2 = nlines * zoom, nsamples2 = nsamples * zoom;

if (VERBOSE) fprintf(stderr, "C zoom2d: nlines=%d, nsamples=%d, zoom=%d\n", nlines, nsamples, zoom);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "C zoom2d: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nsamples2 * nlines2; i ++) data2[i] = 0.0;
i0 = (nsamples2 - nsamples) / 2;
j0 = (nlines2 - nlines) / 2;
for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		data2[i * zoom + zoom/2 + (j * zoom + zoom/2) * nsamples2] = data[i + j * nsamples];
	
if (filter == 0)
	{
	filter = (double *) malloc(nsamples2 * nlines2 * sizeof(double));
	if (!filter)
		{
		fprintf(stderr, "C zoom2d: couldn't malloc filter\n");
		return 0;
		}
		
	for (i = 0; i < nsamples2 * nlines2; i ++) filter[i] = 0.0;
	for (j = 0; j < nlines2; j ++)
		for (i = 0; i < nsamples2; i ++)
			{
			dx = fabs(1.0 / (2.0 * zoom) + (1.0 * i) / zoom - nsamples / 2);
			dy = fabs(1.0 / (2.0 * zoom) + (1.0 * j) / zoom - nlines / 2);
			if (dx <= SINC_DIST && dy <= SINC_DIST) 
				filter[i + j * nsamples2] = sinc(dx) * sinc(dy) 
					* exp(-(dx * dx) / dd2) * exp(-(dy * dy) / dd2);
			}	
	}
		
result = convolve2d(data2, filter, nlines2, nsamples2);

if (data2) free(data2);
//if (filter) free(filter);
return result;
}

//#####################################################################################################

double *extendArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int k, j, i, nlines2 = nlines + 2 * border, nsamples2 = nsamples + 2 * border;

if (VERBOSE) fprintf(stderr, "C extendArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "C extendArray: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nlines2 * nsamples2; i ++) data2[i] = NO_DATA;
	
for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		data2[i + border + (j + border) * nsamples2] = data[i + j * nsamples];

for (j = 0; j < nlines2; j ++)
	{
	for (i = 0; i < nsamples2; i ++)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = 0; k < i; k ++)
				data2[k + j * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	for (i = nsamples2 - 1; i > -1; i --)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = nsamples2 - 1; k > i; k --)
				data2[k + j * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	}
	
for (i = 0; i < nsamples2; i ++)
	{
	for (j = 0; j < nlines2; j ++)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = 0; k < j; k ++)
				data2[i + k * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	for (j = nlines2 - 1; j > -1; j --)
		if (data2[i + j * nsamples2] != NO_DATA)
			{
			for (k = nlines2 - 1; k > j; k --)
				data2[i + k * nsamples2] = data2[i + j * nsamples2];
			break;
			}
	}
	
for (i = 0; i < nlines2 * nsamples2; i ++) 
	if (data2[i] == NO_DATA)
		{
		fprintf(stderr, "C extendArray: holes in GMP data array\n");
		//return 0;
		data2[i] = TDROPOUT;
		}

return data2;
}

//#####################################################################################################

double *extractArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int j, i, nlines2 = nlines - 2 * border, nsamples2 = nsamples - 2 * border;

if (VERBOSE) fprintf(stderr, "C extractArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	fprintf(stderr, "C extractArray: couldn't malloc data2\n");
	return 0;
	}
	
for (j = 0; j < nlines2; j ++)
	for (i = 0; i < nsamples2; i ++)
		data2[i + j * nsamples2] = data[i + border + (j + border) * nsamples];

return data2;
}

//#####################################################################################################

double *zoomArray(double *data, int nlines, int nsamples, int zoom)
{
double *data2 = 0, *data3 = 0, *data4 = 0;
int nlines2, nsamples2, nlines3, nsamples3, nlines4, nsamples4;
int i, j;
int *mask;

mask = (int *) malloc(nlines * nsamples * sizeof(int));
if (!mask)
	{
	fprintf(stderr, "C zoomArray: couldn't malloc mask\n");
	return 0;
	}
for (j = 0; j < nlines; j ++) 
	for (i = 0; i < nsamples; i ++) 
		if (data[i + j * nsamples] != NO_DATA) mask[i + j * nsamples] = 1;
		else mask[i + j * nsamples] = 0;

data2 = extendArray(data, nlines, nsamples, BORDER);
if (!data2) return 0;
nlines2 = nlines + 2 * BORDER;
nsamples2 = nsamples + 2 * BORDER;
data3 = zoom2d(data2, nlines2, nsamples2, zoom);
if (!data3) return 0;
nlines3 = nlines2 * zoom;
nsamples3 = nsamples2 * zoom;
data4 = extractArray(data3, nlines3, nsamples3, zoom * BORDER);
if (!data4) return 0;
nlines4 = nlines3 - 2 * zoom * BORDER;
nsamples4 = nsamples3 - 2 * zoom * BORDER;
if (nlines4 != nlines * zoom)
	{
	fprintf(stderr, "C zoomArray: zoomed nlines mismatch: %d, %d\n", nlines4, nlines * zoom);
	return 0;
	}
if (nsamples4 != nsamples * zoom)
	{
	fprintf(stderr, "C zoomArray: zoomed nsamples mismatch: %d, %d\n", nsamples4, nsamples * zoom);
	return 0;
	}

for (j = 0; j < nlines4; j ++) 
	for (i = 0; i < nsamples4; i ++) 
		if (mask[i / zoom + (j / zoom) * nsamples] == 0) data4[i + j * nsamples4] = NO_DATA;

if (data2) free(data2);
if (data3) free(data3);
if (mask) free(mask);
return data4;
}

//#####################################################################################################

int write_data(char *fname, double *data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "wb");
if (!f)
	{
	fprintf(stderr, "C write_data: couldn't open %s\n", fname);
	return 0;
	}
	
if (fwrite(data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
	{
	fprintf(stderr, "C write_data: couldn't write data\n");
	return 0;
	}
	
fclose(f);
return 1;
}

//#####################################################################################################

int read_data(char *fname, double **data, int nlines, int nsamples) { // define a dynamic array=fname
FILE *f; // FILE structure, declare ptr 
//printf("inside read_data...\n");
//printf("processing fname: %s\n", fname);

f = fopen(fname, "rb");
if (!f) {
	fprintf(stderr, "C: read_data2: couldn't open %s\n", fname);
	return 0;
	}
// 
*data = (double *) malloc(nlines * nsamples * sizeof(double));

if (!*data) {
	fprintf(stderr, "C: read_data: couldn't malloc data\n");
	return 0;
	}
	
if (fread(*data, sizeof(double), nlines * nsamples, f) != nlines * nsamples) { // reads all data==pixels at once from file_ptr==stream into array_ptr in mem- @ ptr == data array; The total number of elements successfully read are returned as a size_t object == nlines * nsamples
	fprintf(stderr, "C: read_data: couldn't read data %s\n", fname);
	return 0;
	}
	
fclose(f);
return 1;
}

//#####################################################################################################
// main
//#####################################################################################################

int main(int argc, char* argv[]) {
int i;
char tmp_str[256]; // s=tmp_str
//char* tmp_str[]=malloc(256); // s=tmp_str

if (argc < 6)
	{
	fprintf(stderr, "Usage: SurfSeaIce_exe, toa_data_file, GP_GMP_file, band, output_data_file, output_image_file \n");
	return 1;
	}
	
strcpy(fname[0], argv[1]);
strcpy(fname[1], argv[2]);
band = atoi(argv[3]);
strcpy(fname[2], argv[4]); // is path to camera_dir/surf_file_name update later
strcpy(fname[3], argv[5]);

//printf("processing fname[0]: %s\n", fname[0]);

if (strstr(fname[0], "_P")) {
	strncpy(tmp_str, strstr(fname[0], "_P")+2, 3); // strstr: points to begining of"_p"+2
	tmp_str[3] = 0;
	//printf("path: %s\n", tmp_str);
	path = atoi(tmp_str);
	memset(tmp_str, '\0', sizeof tmp_str);
	//printf("path: %d\n", path);
	}
else {
	fprintf(stderr, "C: No path info in file name \n"); // %(fname[0]));
	return 1;
	}

if (strstr(fname[0], "_O")) {
	strncpy(tmp_str, strstr(fname[0], "_O")+2, 6); // based on pointer; move pointer 2 characters forward and copy 6 characters to s
	//printf("orbit: %s\n", tmp_str);
	tmp_str[6] = 0;
	orbit = atoi(tmp_str);
	memset(tmp_str, '\0', sizeof tmp_str);
	}
else {
	fprintf(stderr, "C: No orbit info in file name %s\n", fname[0]);
	return 1;
	}

if (strstr(fname[0], "_B")) // if finds "_b" pattern
	{
	strncpy(tmp_str, strstr(fname[0], "_B")+2, 2); // if in future we get 3-digit blocks, update it
	//printf("block: %s \n", tmp_str);
	tmp_str[3] = 0;
	block = atoi(tmp_str);
	memset(tmp_str, '\0', sizeof tmp_str);
	//printf("now block is: %d\n", block)	
	}
else
	{
	fprintf(stderr, "C: No block info in file name \n"); //%(fname[0]));
	return 1;
	}

if 		(strstr(fname[0], "_cf")) camera = 1;
else if (strstr(fname[0], "_an")) camera = 4;
else if (strstr(fname[0], "_ca")) camera = 7;
else {
	fprintf(stderr, "C: Unsupported camera\n");
	return 1;
	}

// printf("processing &fname[0]: %p\n", &fname[0]); //Ehsan
// printf("processing &fname[1]: %p\n", &fname[1]);
// printf("processing *fname[0]: %s\n", *fname[0]);

nlines = 512;
nsamples = 2048;
zoom = 64;
// if band != red, based on data files
if ((band != 2) && (camera != 4)) {
    nlines = 128;
    nsamples = 512;
    zoom = 16;
    }

if (!read_data(fname[0], &data, nlines, nsamples)) return 1;
if (!readGmpFile(fname[1])) return 1;
if (!toa2surf()) return 1;
if (!getDataStats(data, nlines, nsamples)) return 1;
if (nvalid > 0)
	{
	printf("%03d  %06d  %03d  %s  %5d  %5d  %10d  %14.6f  %14.6f  %14.6f  %14.6f  %14.6f  %10d\n", 
		path, orbit, block, camera == 1 ? "cf" : camera == 4 ? "an" : camera == 7 ? "ca" : "??", nlines, nsamples, nvalid, min, max, mean, stddev, solarZenith, ndropouts);
	
	if (!write_data(fname[2], data, nlines, nsamples)) return 1;
	if (!write_png(fname[3], data2image(data, nlines, nsamples, 1), nlines, nsamples)) return 1;
	}

return 0;
}