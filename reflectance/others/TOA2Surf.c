// TOA2Surf.c
// Read TERRAIN and GMP data, correct for atmosphere using SMAC, save as data and PNG
// Sky Coyote 10 Mar 09
// Ehsan: originally at path: /home/mare/Projects/MISR/Tools

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fftw3.h>
#include <png.h>
#include <MisrToolkit.h>
#include <MisrError.h>

#define NO_DATA -999999.0
#define SINC_DIST 10.0
#define BORDER 10
#define DAMPING 3.25
#define VERBOSE 0

typedef struct
{
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
} coef_atmos;

char fname[4][256];
int path = 0;
int block = 0;
int band = 2;
int camera = 1;
int ysize = 512;
int nlines = 0;
int nsamples = 0;
double *data = 0;
int nvalid = 0;
double *sa, *sz, *cfa, *cfz, *caa, *caz;
double min;
double max;
double mean;
double stddev;
double displayMax = -1.0;
png_structp png_ptr = 0;
png_infop info_ptr = 0;
coef_atmos *smac_coefs = 0;

char *data2image(double *data, int ny, int nx, int mode);
int write_png(char *fname, char *image, int ny, int nx);
int read_data(char *fname, double **data, int nlines, int nsamples);
int readTerrainFile(char *fname);
int readGmpFile(char *fname);
int resizeData();
int getDataStats(double *data, int nlines, int nsamples, char *s);
double sinc(double x);
double *convolve2d(double *data, double *filter, int nlines, int nsamples);
double *zoom2d(double *data, int nlines, int nsamples, int zoom);
double *extendArray(double *data, int nlines, int nsamples, int border);
double *extractArray(double *data, int nlines, int nsamples, int border);
double *zoomArray(double *data, int nlines, int nsamples, int zoom);
int SMAC(double tetas, double tetav, double phis, double phiv, double uh2o, double uo3, 
	double taup550, double pression, double r_toa, double *r_surf, char *fichier_wl);
int toa2surf(void);
int pixel2grid(int path, int block, int line, int sample, int *j, int *i);


int pixel2grid(int path, int block, int line, int sample, int *j, int *i)
{
int status;
char *errs[] = MTK_ERR_DESC;
double lat, lon;
double r = 6371228.0;
double r2 = 2.0 * r;
double c = 626.688125;
double x0 = -2443770.3;
double y0 = -313657.41;
double x, y, z;

status = MtkBlsToLatLon(path, 275, block, line * 1.0, sample * 1.0, &lat, &lon);
if (status != MTK_SUCCESS) 
	{
	printf("pixel2grid: MtkBlsToLatLon failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (VERBOSE) printf("pixel2grid: lat = %.6f, lon = %.6f\n", lat, lon);

lat *= M_PI / 180.0;
lon -= 90.0;
lon *= M_PI / 180.0;
z = sin(M_PI_4 - lat / 2.0);
x = (r2 * cos(lon) * z - x0) / c + 0.5;
y = (r2 * sin(lon) * z - y0) / c - 0.5;
*i = x + 0.5;
*j = -y + 0.5;

if (VERBOSE) printf("pixel2grid: x = %.6f, y = %.6f\n", x, y);

return 1;
}


int demlines = 3480;
int demsamples = 3720;
double *psdata = 0, *uwdata = 0, *uo3data = 0;

int getPressure(int path, int block, int line, int sample, 
	double *ps, double *uw, double *uo3)
{
int i, j;

if (psdata == 0)
	{
	if (!read_data("/Users/sky/Projects/Nolin/MISR/Tools/ps.dat", 
		&psdata, demlines, demsamples)) return 0;
	}
if (uwdata == 0)
	{
	if (!read_data("/Users/sky/Projects/Nolin/MISR/Tools/uw.dat", 
		&uwdata, demlines, demsamples)) return 0;
	}
if (uo3data == 0)
	{
	if (!read_data("/Users/sky/Projects/Nolin/MISR/Tools/uo3.dat", 
		&uo3data, demlines, demsamples)) return 0;
	}
	
if (!pixel2grid(path, block, line, sample, &j, &i)) return 0;

if (j < 0 || j >= demlines || i < 0 || i >= demsamples)
	{
	//printf("getPressure: j = %d, i = %d, out of range\n", j, i);
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


int toa2surf(void)
{
int j, i;
double toa, sunAz, sunZen, camAz, camZen, press, tau, h2o, o3, surf;
char fname[256];

strcpy(fname, "/Users/sky/Projects/Nolin/MISR/Tools/SMAC_Coefs/coef_MISR3_CONT.dat");
tau = 0.05;

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		{
		toa = data[i + j * nsamples];
		sunAz = sa[i + j * nsamples];
		sunAz -= 180.0;
		if (sunAz < 0.0) sunAz += 360.0;
		sunZen = sz[i + j * nsamples];
		if (camera == 1)
			{
			camAz = cfa[i + j * nsamples];
			camZen = cfz[i + j * nsamples];
			}
		else
			{
			camAz = caa[i + j * nsamples];
			camZen = caz[i + j * nsamples];
			}
			
		if (toa != NO_DATA && sunAz != NO_DATA && sunZen != NO_DATA 
			&& camAz != NO_DATA && camZen != NO_DATA)
			{
			if (!getPressure(path, block, j, i, &press, &h2o, &o3)) return 0;
			if (!SMAC(sunZen, camZen, sunAz, camAz, h2o, o3, tau, press, toa, 
				&surf, fname)) return 0;
			if (surf < 0.0) surf = 0.0;
			data[i + j * nsamples] = surf;
			}
		else
			data[i + j * nsamples] = NO_DATA;
		}
		
return 1;
}


int SMAC(double tetas, double tetav, double phis, double phiv, double uh2o, double uo3, 
	double taup550, double pression, double r_toa, double *r_surf, char *fichier_wl) 
{
/* Declarations SMAC */
/*-------------------*/
double  cksi;
double s;
double m;
double tg;
double us,uv,dphi;
double wo;

double crd=180./M_PI;
double cdr=M_PI/180.;

double to3,th2o,to2, tco2;
double  tco, tno2,tch4;
double ttetas,ttetav,ksiD;
double atm_ref;

double ak2, ak, e, f, dp, d, b, del, ww, ss, q1, q2, q3, c1, c2, cp1 ;
double cp2, z, x, y, aa1, aa2, aa3 ;

double uo2, uco2, uch4, uco, uno2  ;
double taup,tautot,taurz;
double pressure, Peq ;
double Res_ray, Res_aer, Res_6s;
double ray_phase, ray_ref, aer_ref, aer_phase ;

FILE *fcoef;

if (smac_coefs == 0)
	{
/* Reservation memoire de la structure */
/*-------------------------------------*/
 smac_coefs = (coef_atmos *)malloc(sizeof(coef_atmos));
 if (!smac_coefs)
 	{
 	printf("SMAC: couldn't malloc smac_coefs\n");
 	return 0;
 	}

/* Lecture des coefficients  */
/*---------------------------*/

	fcoef=fopen(fichier_wl,"r");
	if (!fcoef)
		{
		printf("SMAC: erreur d'ouverture du fichier de coefficients: %s\n", fichier_wl);
		return 0;
		}

   fscanf(fcoef,"%lf %lf", &(smac_coefs->ah2o),&(smac_coefs->nh2o));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->ao3),&(smac_coefs->no3));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->ao2),&(smac_coefs->no2),&(smac_coefs->po2));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->aco2),&(smac_coefs->nco2),&(smac_coefs->pco2));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->ach4),&(smac_coefs->nch4),&(smac_coefs->pch4));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->ano2),&(smac_coefs->nno2),&(smac_coefs->pno2));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->aco),&(smac_coefs->nco),&(smac_coefs->pco));
   fscanf(fcoef,"%lf %lf %lf %lf", &(smac_coefs->a0s),&(smac_coefs->a1s),&(smac_coefs->a2s),&(smac_coefs->a3s));
   fscanf(fcoef,"%lf %lf %lf %lf", &(smac_coefs->a0T),&(smac_coefs->a1T),&(smac_coefs->a2T),&(smac_coefs->a3T));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->taur),&(smac_coefs->sr));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->a0taup),&(smac_coefs->a1taup));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->wo),&(smac_coefs->gc));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->a0P),&(smac_coefs->a1P),&(smac_coefs->a2P));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->a3P),&(smac_coefs->a4P));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->Rest1),&(smac_coefs->Rest2));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->Rest3),&(smac_coefs->Rest4));
   fscanf(fcoef,"%lf %lf %lf", &(smac_coefs->Resr1),&(smac_coefs->Resr2),&(smac_coefs->Resr3));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->Resa1),&(smac_coefs->Resa2));
   fscanf(fcoef,"%lf %lf", &(smac_coefs->Resa3),&(smac_coefs->Resa4));
   
   fclose(fcoef);

	if (0)
		{
		printf("%f %f\n", (smac_coefs->ah2o),(smac_coefs->nh2o));
		printf("%f %f\n", (smac_coefs->ao3),(smac_coefs->no3));
		printf("%f %f %f\n", (smac_coefs->ao2),(smac_coefs->no2),(smac_coefs->po2));
		printf("%f %f %f\n", (smac_coefs->aco2),(smac_coefs->nco2),(smac_coefs->pco2));
		printf("%f %f %f\n", (smac_coefs->ach4),(smac_coefs->nch4),(smac_coefs->pch4));
		printf("%f %f %f\n", (smac_coefs->ano2),(smac_coefs->nno2),(smac_coefs->pno2));
		printf("%f %f %f\n", (smac_coefs->aco),(smac_coefs->nco),(smac_coefs->pco));
		printf("%f %f %f %f\n", (smac_coefs->a0s),(smac_coefs->a1s),(smac_coefs->a2s),(smac_coefs->a3s));
		printf("%f %f %f %f\n", (smac_coefs->a0T),(smac_coefs->a1T),(smac_coefs->a2T),(smac_coefs->a3T));
		printf("%f %f\n", (smac_coefs->taur),(smac_coefs->sr));
		printf("%f %f\n", (smac_coefs->a0taup),(smac_coefs->a1taup));
		printf("%f %f\n", (smac_coefs->wo),(smac_coefs->gc));
		printf("%f %f %f\n", (smac_coefs->a0P),(smac_coefs->a1P),(smac_coefs->a2P));
		printf("%f %f\n", (smac_coefs->a3P),(smac_coefs->a4P));
		printf("%f %f\n", (smac_coefs->Rest1),(smac_coefs->Rest2));
		printf("%f %f\n", (smac_coefs->Rest3),(smac_coefs->Rest4));
		printf("%f %f %f\n", (smac_coefs->Resr1),(smac_coefs->Resr2),(smac_coefs->Resr3));
		printf("%f %f\n", (smac_coefs->Resa1),(smac_coefs->Resa2));
		printf("%f %f\n", (smac_coefs->Resa3),(smac_coefs->Resa4));
		
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
to3 = 1. ;
th2o= 1. ;
to2 = 1. ;
tco2= 1. ;
tch4= 1. ;
 
uo2= pow (Peq , (smac_coefs->po2));
uco2= pow (Peq , (smac_coefs->pco2));
uch4= pow (Peq , (smac_coefs->pch4));
uno2= pow (Peq , (smac_coefs->pno2));
uco = pow (Peq , (smac_coefs->pco));

/*------  4) if uh2o <= 0 and uo3 <= 0 no gaseous absorption is computed*/
if( (uh2o > 0.) || ( uo3 > 0.) )
{
        to3   = exp ( (smac_coefs->ao3)  * pow ( (uo3 *m)  , (smac_coefs->no3)  ) ) ;
        th2o  = exp ( (smac_coefs->ah2o) * pow ( (uh2o*m)  , (smac_coefs->nh2o) ) ) ;
        to2   = exp ( (smac_coefs->ao2)  * pow ( (uo2 *m)  , (smac_coefs->no2)  ) ) ;
        tco2  = exp ( (smac_coefs->aco2) * pow ( (uco2*m)  , (smac_coefs->nco2) ) ) ;
        tch4  = exp ( (smac_coefs->ach4) * pow ( (uch4*m)  , (smac_coefs->nch4) ) ) ;
        tno2  = exp ( (smac_coefs->ano2) * pow ( (uno2*m)  , (smac_coefs->nno2) ) ) ;
        tco   = exp ( (smac_coefs->aco)  * pow ( (uco*m)   , (smac_coefs->nco) ) ) ;
}
 
/*------  5) Total scattering transmission */
ttetas = (smac_coefs->a0T) + (smac_coefs->a1T)*taup550/us + ((smac_coefs->a2T)*Peq + (smac_coefs->a3T))/(1.+us) ; /* downward */
ttetav = (smac_coefs->a0T) + (smac_coefs->a1T)*taup550/uv + ((smac_coefs->a2T)*Peq + (smac_coefs->a3T))/(1.+uv) ; /* upward   */

/*------  6) spherical albedo of the atmosphere */
s = (smac_coefs->a0s) * Peq +  (smac_coefs->a3s) + (smac_coefs->a1s)*taup550 + (smac_coefs->a2s) *pow (taup550 , 2) ;

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
tautot=taup+taurz;
Res_6s= ( (smac_coefs->Rest1) + (smac_coefs->Rest2) * ( tautot * m *cksi ) 
        + (smac_coefs->Rest3) * pow( (tautot*m*cksi),2) ) + (smac_coefs->Rest4) * pow( (tautot*m*cksi),3);

/*------  11) total atmospheric reflectance */
atm_ref = ray_ref - Res_ray + aer_ref - Res_aer + Res_6s;

/*-------- reflectance at toa*/

tg      = th2o * to3 * to2 * tco2 * tch4* tco * tno2 ;

 /* reflectance at surface */
/*------------------------*/
  *r_surf = r_toa - (atm_ref * tg) ;
  *r_surf = *r_surf / ( (tg * ttetas * ttetav) + (*r_surf * s) ) ;
  
return 1;
}    


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
		if (z > max) max = z;
		}
	if (max == -1.0e23)
		{
		if (VERBOSE) printf("data2image: no valid data\n");
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
			if (z < min) min = z;
			}
		if (min == 1.0e23)
			{
			if (VERBOSE) printf("data2image: no valid data\n");
			min = 0.0;
			//return 0;
			}
		}
	else min = 0.0;
	}
if (VERBOSE) printf("data2image: min=%.3f, max=%.3f\n", min, max);
if (max != min) dz =  255.0 / (max - min);
else dz = 0.0;

image = (char *) malloc(nlines * nsamples);
if (!image)
	{
	printf("data2image: couldn't malloc image\n");
	return 0;
	}

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z != NO_DATA) 
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


int write_png(char *fname, char *image, int ny, int nx)
{
FILE *fp;
png_bytepp row_ptrs;
int j;

if (!image)
	{
	printf("write_png: null image\n");
	return 0;
	}

row_ptrs = (png_bytepp) malloc(ny * sizeof(png_bytep));
if (!row_ptrs)
	{
	printf("write_png: couldn't malloc row_ptrs\n");
	return 0;
	}
for (j = 0; j < ny; j ++) row_ptrs[j] = (png_bytep)(image + j * nx);

fp = fopen(fname, "wb");
if (!fp)
	{
	printf("write_png: couldn't open %s\n", fname);
	return 0;
	}

png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
	png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
if (!png_ptr)
	{
	fclose(fp);
	printf("write_png: png_create_write_struct failed\n");
	return 0;
	}

info_ptr = png_create_info_struct(png_ptr);
if (!info_ptr)
	{
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
	printf("write_png: png_create_info_struct failed\n");
	return 0;
	}

if (setjmp(png_jmpbuf(png_ptr)))
	{
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	printf("write_png: longjmp from png error\n");
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


int read_data(char *fname, double **data, int nlines, int nsamples)
{
FILE *f;

f = fopen(fname, "rb");
if (!f)
	{
	printf("read_data: couldn't open %s\n", fname);
	return 0;
	}
	
*data = (double *) malloc(nlines * nsamples * sizeof(double));
if (!*data)
	{
	printf("read_data: couldn't malloc data\n");
	return 0;
	}
	
if (fread(*data, sizeof(double), nlines * nsamples, f) != nlines * nsamples)
	{
	printf("read_data: couldn't read data\n");
	return 0;
	}
	
fclose(f);
return 1;
}


int print_data(double *data, int nlines, int nsamples)
{
int j, i;
char s[256];

for (j = 0; j < nlines; j ++)
	{
	for (i = 0; i < nsamples; i ++)
		if (i == 0) printf("%14.6f", data[i + j * nsamples]);
		else printf("  %14.6f", data[i + j * nsamples]);
	printf("\n");
	}
}


int readTerrainFile(char *fname)
{
MTKt_DataBuffer databuf = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer scalebuf = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer fillbuf = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer cfbuf = MTKT_DATABUFFER_INIT;
char gridName[256];
char fieldName[256];
int status;
MTKt_FileType filetype;
char *types[] = MTKd_DataType;
char *errs[] = MTK_ERR_DESC;
int i, j, lf, sf, n;
double *brfcf, *brfcf2;

if (VERBOSE) printf("\nreadTerrainFile: fname=%s, block=%d, band=%d\n", fname, block, band);

status = MtkFileType(fname, &filetype);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (filetype != MTK_GRP_TERRAIN_GM)
	{
	printf("readTerrainFile: only L1B2 global mode TERRAIN files are supported!!!\n");
	return 0;
	}

if (band == 0) { strcpy(gridName, "BlueBand"); strcpy(fieldName, "Blue Radiance/RDQI"); }
else if (band == 1) { strcpy(gridName, "GreenBand"); strcpy(fieldName, "Green Radiance/RDQI"); }
else if (band == 2) { strcpy(gridName, "RedBand"); strcpy(fieldName, "Red Radiance/RDQI"); }
else if (band == 3) { strcpy(gridName, "NIRBand"); strcpy(fieldName, "NIR Radiance/RDQI"); }
else
	{
	printf("readTerrainFile: unsupported band = %d\n", band);
	return 0;
	}

if (VERBOSE) printf("readTerrainFile: grid=%s, field=%s\n", gridName, fieldName);

//if (!getOffsets(fname, gridName, offset, NOFFSETS)) return 0;

status = MtkReadBlock(fname, gridName, fieldName, block, &databuf);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (VERBOSE) printf("readTerrainFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	databuf.nline, databuf.nsample, databuf.datasize, databuf.datatype, types[databuf.datatype]);
	
if (databuf.nline != 512 || databuf.nsample != 2048)
	{
	printf("readTerrainFile: databuf is not 8x32: (%d, %d)\n", databuf.nline, databuf.nsample);
	return 0;
	}

status = MtkFillValueGet(fname, gridName, fieldName, &fillbuf);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkFillValueGet failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
//printf("readTerrainFile: FillValue: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s), value=%u\n", 
//	fillbuf.nline, fillbuf.nsample, fillbuf.datasize, fillbuf.datatype, types[fillbuf.datatype], fillbuf.data.u16[0][0]);

/* If filetype is TERRAIN adjust fill value to account for obscured by topography flag. */
status = MtkFileType(fname, &filetype);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}

if (filetype ==  MTK_GRP_TERRAIN_GM || filetype ==  MTK_GRP_TERRAIN_LM)
	fillbuf.data.u16[0][0] -= 4;
if (VERBOSE) printf("readTerrainFile: FillValue: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s), value=%u\n", 
	fillbuf.nline, fillbuf.nsample, fillbuf.datasize, fillbuf.datatype, types[fillbuf.datatype], fillbuf.data.u16[0][0]);

status = MtkGridAttrGet(fname, gridName, "Scale factor", &scalebuf);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkGridAttrGet failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readTerrainFile: Scale factor: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s), value=%f\n", 
	scalebuf.nline, scalebuf.nsample, scalebuf.datasize, scalebuf.datatype, types[scalebuf.datatype], scalebuf.data.d[0][0]);
	
if (band == 0) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "BlueConversionFactor"); }
else if (band == 1) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "GreenConversionFactor"); }
else if (band == 2) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "RedConversionFactor"); }
else if (band == 3) { strcpy(gridName, "BRF Conversion Factors"); strcpy(fieldName, "NIRConversionFactor"); }
else
	{
	printf("readTerrainFile: unsupported band = %d\n", band);
	return 0;
	}

if (VERBOSE) printf("readTerrainFile: grid=%s, field=%s\n", gridName, fieldName);

status = MtkReadBlock(fname, gridName, fieldName, block, &cfbuf);
if (status != MTK_SUCCESS) 
	{
	printf("readTerrainFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (VERBOSE) printf("readTerrainFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	cfbuf.nline, cfbuf.nsample, cfbuf.datasize, cfbuf.datatype, types[cfbuf.datatype]);
lf = databuf.nline / cfbuf.nline;
sf = databuf.nsample / cfbuf.nsample;	

if (cfbuf.nline != 8 || cfbuf.nsample != 32)
	{
	printf("readTerrainFile: cfbuf is not 8x32: (%d, %d)\n", cfbuf.nline, cfbuf.nsample);
	return 0;
	}
brfcf = (double *) malloc(8 * 32 * sizeof(double));
if (!brfcf)
	{
	printf("readTerrainFile: brfcf malloc failed!!!\n");
	return 0;
	}
n = 0;
for (j = 0; j < 8; j ++)
	for (i = 0; i < 32; i ++)
		{
		if (cfbuf.data.f[j][i] > 0.0) 
			{
			n ++;
			brfcf[i + j * 32] = cfbuf.data.f[j][i];
			}
		else
			brfcf[i + j * 32] = NO_DATA;
		}
if (n != 256)
	{
	if (VERBOSE) printf("readTerrainFile: fewer than 256 valid in cfbuf: %d\n", n);
	//return 0;
	}
getDataStats(brfcf, 8, 32, "brfcf1");
if (!write_png("brfcf1.png", data2image(brfcf, 8, 32, 1), 8, 32)) return 0;
brfcf2 = zoomArray(brfcf, 8, 32, 64);
if (!brfcf2) return 0;
getDataStats(brfcf2, 512, 2048, "brfcf2");
if (!write_png("brfcf2.png", data2image(brfcf2, 512, 2048, 1), 512, 2048)) return 0;

if (data != 0) free(data);
nlines = databuf.nline;
nsamples = databuf.nsample;
data = (double *) malloc(nlines * nsamples * sizeof(double));
if (!data)
	{
	printf("readTerrainFile: data malloc failed!!!\n");
	return 0;
	}

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		{
		if (databuf.data.u16[j][i] < fillbuf.data.u16[0][0]) 
			data[i + j * nsamples] = (databuf.data.u16[j][i] >> 2) * scalebuf.data.d[0][0];
		else 
			data[i + j * nsamples] = NO_DATA;
		}
getDataStats(data, 512, 2048, "data1");
if (!write_png("data1.png", data2image(data, 512, 2048, 1), 512, 2048)) return 0;

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		{
		if (databuf.data.u16[j][i] < fillbuf.data.u16[0][0] && cfbuf.data.f[j/lf][i/sf] > 0.0) 
			data[i + j * nsamples] = (databuf.data.u16[j][i] >> 2) * 
				scalebuf.data.d[0][0] * cfbuf.data.f[j/lf][i/sf];
		else 
			data[i + j * nsamples] = NO_DATA;
		}
getDataStats(data, 512, 2048, "data2");
if (!write_png("data2.png", data2image(data, 512, 2048, 1), 512, 2048)) return 0;

for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		{
		if (databuf.data.u16[j][i] < fillbuf.data.u16[0][0] && brfcf2[i + j * 2048] != NO_DATA) 
			data[i + j * nsamples] = (databuf.data.u16[j][i] >> 2) * 
				scalebuf.data.d[0][0] * brfcf2[i + j * 2048];
		else 
			data[i + j * nsamples] = NO_DATA;
		}
getDataStats(data, 512, 2048, "data3");
if (!write_png("data3.png", data2image(data, 512, 2048, 1), 512, 2048)) return 0;

if (VERBOSE) printf("readTerrainFile: data is %d x %d\n", nlines, nsamples);

if (!resizeData()) return 0;

MtkDataBufferFree(&databuf);
MtkDataBufferFree(&scalebuf);
MtkDataBufferFree(&fillbuf);
MtkDataBufferFree(&cfbuf);
if (brfcf) free(brfcf);
return 1;
}


int readGmpFile(char *fname)
{
MTKt_DataBuffer tmpbuf1 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf2 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf3 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf4 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf5 = MTKT_DATABUFFER_INIT;
MTKt_DataBuffer tmpbuf6 = MTKT_DATABUFFER_INIT;
char gridName[256];
char fieldName[256];
int status;
MTKt_FileType filetype;
char *types[] = MTKd_DataType;
char *errs[] = MTK_ERR_DESC;
int i, j, n;
double *tmp;
int zoom = 64, wrap = 0;

if (VERBOSE) printf("\nreadGmpFile: fname=%s, block=%d\n", fname, block, band);

status = MtkFileType(fname, &filetype);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkFileType failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
	
if (filetype != MTK_GP_GMP)
	{
	printf("readGmpFile: only L1B2 Geometric Parameters files are supported!!!\n");
	return 0;
	}

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "SolarAzimuth");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf1);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf1.nline, tmpbuf1.nsample, tmpbuf1.datasize, tmpbuf1.datatype, types[tmpbuf1.datatype]);
if (tmpbuf1.nline != 8 || tmpbuf1.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf1.nline, tmpbuf1.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
			}
		else tmp[i + j * 32] = NO_DATA;
		}
//print_data(tmp, 8, 32);
if (n != 256)
	{
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "sa1");
if (!write_png("sa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
if (min <= 10.0 && max >= 350.0)
	{
	for (i = 0; i < 8 * 32; i ++)
		{
		if (tmp[i] == NO_DATA) continue;
		if (tmp[i] < 180.0) tmp[i] += 360.0;
		}
	wrap = 1;
	}
sa = zoomArray(tmp, 8, 32, 64);
if (!sa) return 0;
if (wrap)
	{
	for (i = 0; i < 512 * 2048; i ++)
		{
		if (sa[i] == NO_DATA) continue;
		if (sa[i] >= 360.0) sa[i] -= 360.0;
		}
	wrap = 0;
	}
getDataStats(sa, 512, 2048, "sa2");
if (!write_png("sa2.png", data2image(sa, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf1);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "SolarZenith");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf2);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf2.nline, tmpbuf2.nsample, tmpbuf2.datasize, tmpbuf2.datatype, types[tmpbuf2.datatype]);
if (tmpbuf2.nline != 8 || tmpbuf2.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf2.nline, tmpbuf2.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "sz1");
if (!write_png("sz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
sz = zoomArray(tmp, 8, 32, 64);
if (!sz) return 0;
getDataStats(sz, 512, 2048, "sz2");
if (!write_png("sz2.png", data2image(sz, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf2);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "CfAzimuth");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf3);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf3.nline, tmpbuf3.nsample, tmpbuf3.datasize, tmpbuf3.datatype, types[tmpbuf3.datatype]);
if (tmpbuf3.nline != 8 || tmpbuf3.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf3.nline, tmpbuf3.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
			}
		else tmp[i + j * 32] = NO_DATA;
		}
if (n != 256)
	{
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "cfa1");
if (!write_png("cfa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
if (min <= 10.0 && max >= 350.0)
	{
	for (i = 0; i < 8 * 32; i ++)
		{
		if (tmp[i] == NO_DATA) continue;
		if (tmp[i] < 180.0) tmp[i] += 360.0;
		}
	wrap = 1;
	}
cfa = zoomArray(tmp, 8, 32, 64);
if (!cfa) return 0;
if (wrap)
	{
	for (i = 0; i < 512 * 2048; i ++)
		{
		if (cfa[i] == NO_DATA) continue;
		if (cfa[i] >= 360.0) cfa[i] -= 360.0;
		}
	wrap = 0;
	}
getDataStats(cfa, 512, 2048, "cfa2");
if (!write_png("cfa2.png", data2image(cfa, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf3);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "CfZenith");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf4);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf4.nline, tmpbuf4.nsample, tmpbuf4.datasize, tmpbuf4.datatype, types[tmpbuf4.datatype]);
if (tmpbuf4.nline != 8 || tmpbuf4.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf4.nline, tmpbuf4.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "cfz1");
if (!write_png("cfz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
cfz = zoomArray(tmp, 8, 32, 64);
if (!cfz) return 0;
getDataStats(cfz, 512, 2048, "cfz2");
if (!write_png("cfz2.png", data2image(cfz, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf4);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "CaAzimuth");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf5);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf5.nline, tmpbuf5.nsample, tmpbuf5.datasize, tmpbuf5.datatype, types[tmpbuf5.datatype]);
if (tmpbuf5.nline != 8 || tmpbuf5.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf5.nline, tmpbuf5.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
			}
		else tmp[i + j * 32] = NO_DATA;
		}
if (n != 256)
	{
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "caa1");
if (!write_png("caa1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
if (min <= 10.0 && max >= 350.0)
	{
	for (i = 0; i < 8 * 32; i ++)
		{
		if (tmp[i] == NO_DATA) continue;
		if (tmp[i] < 180.0) tmp[i] += 360.0;
		}
	wrap = 1;
	}
caa = zoomArray(tmp, 8, 32, 64);
if (!caa) return 0;
if (wrap)
	{
	for (i = 0; i < 512 * 2048; i ++)
		{
		if (caa[i] == NO_DATA) continue;
		if (caa[i] >= 360.0) caa[i] -= 360.0;
		}
	wrap = 0;
	}
getDataStats(caa, 512, 2048, "caa2");
if (!write_png("caa2.png", data2image(caa, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf5);
if (tmp) free(tmp);

strcpy(gridName, "GeometricParameters"); 
strcpy(fieldName, "CaZenith");
if (VERBOSE) printf("readGmpFile: grid=%s, field=%s\n", gridName, fieldName);
status = MtkReadBlock(fname, gridName, fieldName, block, &tmpbuf6);
if (status != MTK_SUCCESS) 
	{
	printf("readGmpFile: MtkReadBlock failed!!!, status = %d (%s)\n", status, errs[status]);
	return 0;
	}
if (VERBOSE) printf("readGmpFile: nline=%d, nsample=%d, datasize=%d, datatype=%d (%s)\n", 
	tmpbuf6.nline, tmpbuf6.nsample, tmpbuf6.datasize, tmpbuf6.datatype, types[tmpbuf6.datatype]);
if (tmpbuf6.nline != 8 || tmpbuf6.nsample != 32)
	{
	printf("readGmpFile: %s is not 8x32: (%d, %d)\n", fieldName, tmpbuf6.nline, tmpbuf6.nsample);
	return 0;
	}
tmp = (double *) malloc(8 * 32 * sizeof(double));
if (!tmp)
	{
	printf("readGmpFile: tmp malloc failed!!!\n");
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
	if (VERBOSE) printf("readGmpFile: fewer than 256 valid in %s: %d\n", fieldName, n);
	//return 0;
	}
getDataStats(tmp, 8, 32, "caz1");
if (!write_png("caz1.png", data2image(tmp, 8, 32, 1), 8, 32)) return 0;
caz = zoomArray(tmp, 8, 32, 64);
if (!caz) return 0;
getDataStats(caz, 512, 2048, "caz2");
if (!write_png("caz2.png", data2image(caz, 512, 2048, 1), 512, 2048)) return 0;
MtkDataBufferFree(&tmpbuf6);
if (tmp) free(tmp);

return 1;
}


int resizeData()
{
int i, j, zoom, nlines2, nsamples2;
double *data2;

if (nlines != ysize)
	{
	nlines2 = ysize;
	nsamples2 = 4 * ysize;
	
	data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
	if (!data2)
		{
		printf("resizeData: malloc of zoomed double array failed!!!\n");
		return 0;
		}

	if (nlines2 > nlines)
		{
		zoom = nlines2 / nlines;
		if (VERBOSE) printf("resizeData: zooming by %dx\n", zoom);
		
		for (j = 0; j < nlines2; j ++)
			for (i = 0; i < nsamples2; i ++)
				data2[i + j * nsamples2] = data[i / zoom + (j / zoom) * nsamples];
		}
	else
		{
		zoom = nlines / nlines2;
		if (VERBOSE) printf("resizeData: zooming by 1/%dx\n", zoom);
		
		for (j = 0; j < nlines2; j ++)
			for (i = 0; i < nsamples2; i ++)
				data2[i + j * nsamples2] = data[i * zoom + (j * zoom) * nsamples];
		}
				
	if (data != 0) free(data);
	data = data2;
	nlines = nlines2;
	nsamples = nsamples2;
	}
	
return 1;
}


int getDataStats(double *data, int nlines, int nsamples, char *s)
{
int i;
double z;

min = 1.0e23;
max = -1.0e23;
mean = 0.0;
stddev = 0.0;
nvalid = 0;

for (i = 0; i < nlines * nsamples; i ++)
	{
	z = data[i];
	if (z == NO_DATA) continue;
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
			stddev += (z - mean) * (z - mean);
			}
		stddev = sqrt(stddev / (nvalid - 1.0));
		}
	}

if (nvalid > 0)
	printf("%9s  %9d  %9d  %9d  %9.3f  %9.3f  %9.3f  %9.3f\n", s, nlines, nsamples, nvalid, min, max, mean, stddev);
else
	printf("%9s  %9d  %9d  %9d  %9.3f  %9.3f  %9.3f  %9.3f\n", s, nlines, nsamples, nvalid, -1.0, -1.0, -1.0, -1.0);

return 1;
}


double sinc(double x)
{
if (x == 0.0) return 1.0;
x *= M_PI;
return sin(x) / x;
}


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
rbuf2 = (double *) fftw_malloc(n * sizeof(double));
cbuf1 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
cbuf2 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));
cbuf3 = (fftw_complex *) fftw_malloc(n2 * sizeof(fftw_complex));

for (i = 0; i < n; i ++) rbuf1[i] = data[i];
for (i = 0; i < n; i ++) rbuf2[i] = filter[i];

if (VERBOSE > 1) printf("fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf1, cbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) printf("fftw_execute\n");	
fftw_execute(p);
fftw_destroy_plan(p);

if (VERBOSE > 1) printf("fftw_plan_dft_r2c_2d\n");	
p = fftw_plan_dft_r2c_2d(nlines, nsamples, rbuf2, cbuf2, FFTW_ESTIMATE);
if (VERBOSE > 1) printf("fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) printf("fftw_destroy_plan\n");	
fftw_destroy_plan(p);

for (i = 0; i < n2; i ++)
	{
	cbuf3[i][0] = cbuf1[i][0] * cbuf2[i][0] - cbuf1[i][1] * cbuf2[i][1];
	cbuf3[i][1] = cbuf1[i][0] * cbuf2[i][1] + cbuf1[i][1] * cbuf2[i][0];
	}

if (VERBOSE > 1) printf("fftw_plan_dft_c2r_2d\n");	
p = fftw_plan_dft_c2r_2d(nlines, nsamples, cbuf3, rbuf1, FFTW_ESTIMATE);
if (VERBOSE > 1) printf("fftw_execute\n");	
fftw_execute(p);
if (VERBOSE > 1) printf("fftw_destroy_plan\n");	
fftw_destroy_plan(p);

result = (double *) malloc(nsamples * nlines * sizeof(double));
if (!result)
	{
	printf("convolve2d: couldn't malloc result\n");
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


double *zoom2d(double *data, int nlines, int nsamples, int zoom)
{
double dx, dy, *data2, *filter, *result, dd2 = DAMPING * DAMPING;
int i, j, i0, j0, nlines2 = nlines * zoom, nsamples2 = nsamples * zoom;

if (VERBOSE) printf("zoom2d: nlines=%d, nsamples=%d, zoom=%d\n", nlines, nsamples, zoom);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	printf("zoom2d: couldn't malloc data2\n");
	return 0;
	}
	
for (i = 0; i < nsamples2 * nlines2; i ++) data2[i] = 0.0;
i0 = (nsamples2 - nsamples) / 2;
j0 = (nlines2 - nlines) / 2;
for (j = 0; j < nlines; j ++)
	for (i = 0; i < nsamples; i ++)
		data2[i * zoom + zoom/2 + (j * zoom + zoom/2) * nsamples2] = data[i + j * nsamples];
		
filter = (double *) malloc(nsamples2 * nlines2 * sizeof(double));
if (!filter)
	{
	printf("zoom2d: couldn't malloc filter\n");
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
		
result = convolve2d(data2, filter, nlines2, nsamples2);

if (data2) free(data2);
if (filter) free(filter);
return result;
}


double *extendArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int k, j, i, nlines2 = nlines + 2 * border, nsamples2 = nsamples + 2 * border;

if (VERBOSE) printf("extendArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	printf("extendArray: couldn't malloc data2\n");
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
		printf("extendArray: holes in data array\n");
		return 0;
		}

return data2;
}


double *extractArray(double *data, int nlines, int nsamples, int border)
{
double *data2;
int j, i, nlines2 = nlines - 2 * border, nsamples2 = nsamples - 2 * border;

if (VERBOSE) printf("extractArray: nlines=%d, nsamples=%d, border=%d\n", nlines, nsamples, border);

data2 = (double *) malloc(nlines2 * nsamples2 * sizeof(double));
if (!data2)
	{
	printf("extractArray: couldn't malloc data2\n");
	return 0;
	}
	
for (j = 0; j < nlines2; j ++)
	for (i = 0; i < nsamples2; i ++)
		data2[i + j * nsamples2] = data[i + border + (j + border) * nsamples];

return data2;
}


double *zoomArray(double *data, int nlines, int nsamples, int zoom)
{
double *data2 = 0, *data3 = 0, *data4 = 0;
int nlines2, nsamples2, nlines3, nsamples3, nlines4, nsamples4;
int i, j;
int *mask;

mask = (int *) malloc(nlines * nsamples * sizeof(int));
if (!mask)
	{
	printf("zoomArray: couldn't malloc mask\n");
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
//if (!write_png("extended.png", data2image(data2, nlines2, nsamples2, 1), nlines2, nsamples2)) return 0;
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
	printf("zoomArray: zoomed nlines mismatch: %d, %d\n", nlines4, nlines * zoom);
	return 0;
	}
if (nsamples4 != nsamples * zoom)
	{
	printf("zoomArray: zoomed nsamples mismatch: %d, %d\n", nsamples4, nsamples * zoom);
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

// ################################  main  #############################

int main(int argc, char *argv[])
{
int i;
char ps[4];

if (argc < 5)
	{
	printf("Usage: TOA2Surf terrain_file gmp_file output_data_file block [display-max=-1.0(autoscale)]\n"); // Ehsan: gmp = geometric parameter
	return 1;
	}
	
strcpy(fname[0], argv[1]); //terrain_file
strcpy(fname[1], argv[2]);
strcpy(fname[2], argv[3]); // output_data_file
block = atoi(argv[4]);

if (argc > 5) displayMax = atof(argv[5]);

strcpy(fname[3], fname[2]);
/*if (strstr(fname[3], ".dat")) strcpy(strstr(fname[3], ".dat"), ".png");
else*/ strcpy(fname[3], "TOA2Surf.png");

if (strstr(fname[0], "_P"))
	{
	strncpy(ps, strstr(fname[0], "_P") + 2, 3);
	ps[3] = 0;
	path = atoi(ps);
	}
else
	{
	printf("No path info in file name\n");
	return 1;
	}

if (strstr(fname[0], "_CF_")) camera = 1;
else if (strstr(fname[0], "_CA_")) camera = 7;
else
	{
	printf("Unsupported camera\n");
	return 1;
	}

printf("      var     nlines   nsamples     nvalid        min        max       mean     stddev\n");
printf("---------  ---------  ---------  ---------  ---------  ---------  ---------  ---------\n");

if (!readTerrainFile(fname[0])) return 1;
if (!readGmpFile(fname[1])) return 1;
if (!toa2surf()) return 1;
getDataStats(data, 512, 2048, "data4");
if (!write_png("data4.png", data2image(data, nlines, nsamples, 1), nlines, nsamples)) return 1;

return 0;
}


