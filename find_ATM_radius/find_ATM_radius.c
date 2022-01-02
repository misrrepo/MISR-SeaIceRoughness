// Ehsan Mosadegh 10 Nov 2019
// notes:
// to-do: 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef struct
{
    int path;
    int orbit;
    int block;
    int line;
    int sample;
    double lat;
    double lon;
    double an;
    double ca;
    double cf;
    double rms;
    double rms_model;
    double rms_ad_model;
    float weight;
    float tweight;
    int ascend;
} atm_type;

atm_type *atm_model;
int atm_np = 0;

int pearson(double *corr, double *corref, double *corr_ascend, double *corref_ascend, double *corr_descend, double *corref_descend);
int spearman(double *corr);

int spearman(double *corr)
{
    int i, j;
    double *a, *b;
    int *ar, *br;
    int ar_max, ar_min, br_max, br_min;
    double amax, amin, bmax, bmin;

    a = (double * ) malloc(atm_np * sizeof(double));
    b = (double * ) malloc(atm_np * sizeof(double));
    ar = (int * ) malloc(atm_np * sizeof(int));
    br = (int * ) malloc(atm_np * sizeof(int));

    for (i=0; i < atm_np; i++) {
	a[i] = atm_model[i].rms;
	b[i] = atm_model[i].rms_model;
    }

    for (i=0; i < (int) atm_np/2; i++) {
	amax = -1e23;
	amin = 1e23;
	bmax = -1e23;
	bmin = 1e23;
	for (j=0; j < atm_np; j++) {
	    if ((a[j] > 0) && (a[j] > amax)) {
		amax = a[j];
		ar_max = j;
	    }
	    if ((a[j] > 0) && (a[j] < amin)) {
		amin = a[j];
		ar_min = j;
	    }
	    if ((b[j] > 0) && (b[j] > bmax)) {
		bmax = b[j];
		br_max = j;
	    }
	    if ((b[j] > 0) && (b[j] < bmin)) {
		bmin = b[j];
		br_min = j;
	    }
	}
	ar[i] = ar_min;
	ar[atm_np - i - 1] = ar_max;
	br[i] = br_min;
	br[atm_np - i - 1] = br_max;
	a[ar_max] = -1;
	a[ar_min] = -1;
	b[br_max] = -1;
	b[br_min] = -1;
    }

    if (atm_np % 2) {
	for (i=0; i < atm_np; i++) {
	    if (a[i] > 0) ar[(int) atm_np/2] = i; 
	    if (b[i] > 0) br[(int) atm_np/2] = i; 
	}
    }

    *corr = 0;
    for (i=0; i < atm_np; i++) {
	*corr += (ar[i] - br[i]) * (ar[i] - br[i]);
    }

    *corr = 1.0 - 6.0*(*corr)/(atm_np * (atm_np*atm_np - 1.0));

    return 1;
}

int pearson(double *corr, double *corref, double *corr_ascend, double *corref_ascend, double *corr_descend, double *corref_descend)
{
    // from https://en.wikipedia.org/wiki/Pearson_product-moment_correlation_coefficient    

    int n;

    //Calculate mean
    double meanx = 0;
    double meany = 0;
    double weight = 0;
    double meanx_ascend = 0;
    double meany_ascend = 0;
    double weight_ascend = 0;
    double meanx_descend = 0;
    double meany_descend = 0;
    double weight_descend = 0;
    for (n = 0; n < atm_np; n++) {
	meanx += atm_model[n].tweight * atm_model[n].rms;
	meany += atm_model[n].tweight * atm_model[n].rms_model;
	weight += atm_model[n].tweight;
	if (atm_model[n].ascend) {
	    meanx_ascend += atm_model[n].tweight * atm_model[n].rms;
	    meany_ascend += atm_model[n].tweight * atm_model[n].rms_ad_model;
	    weight_ascend += atm_model[n].tweight;
	}
	else {
	    meanx_descend += atm_model[n].tweight * atm_model[n].rms;
	    meany_descend += atm_model[n].tweight * atm_model[n].rms_ad_model;
	    weight_descend += atm_model[n].tweight;
	}
    }
    meanx /= weight;
    meany /= weight;
    meanx_ascend /= weight_ascend;
    meany_ascend /= weight_ascend;
    meanx_descend /= weight_descend;
    meany_descend /= weight_descend;

    //Calculate covariance
    double covx = 0;
    double covy = 0;
    double covxy = 0;
    double covx_ascend = 0;
    double covy_ascend = 0;
    double covxy_ascend = 0;
    double covx_descend = 0;
    double covy_descend = 0;
    double covxy_descend = 0;
    for (n = 0; n < atm_np; n++) {
	covx += atm_model[n].tweight * (atm_model[n].rms - meanx) * (atm_model[n].rms - meanx);
	covy += atm_model[n].tweight * (atm_model[n].rms_model - meany) * (atm_model[n].rms_model - meany);
	covxy += atm_model[n].tweight * (atm_model[n].rms - meanx) * (atm_model[n].rms_model - meany);
	if (atm_model[n].ascend) {
	    covx_ascend += atm_model[n].tweight * (atm_model[n].rms - meanx_ascend) * (atm_model[n].rms - meanx_ascend);
	    covy_ascend += atm_model[n].tweight * (atm_model[n].rms_ad_model - meany_ascend) * (atm_model[n].rms_ad_model - meany_ascend);
	    covxy_ascend += atm_model[n].tweight * (atm_model[n].rms - meanx_ascend) * (atm_model[n].rms_ad_model - meany_ascend);
	}
	else {
	    covx_descend += atm_model[n].tweight * (atm_model[n].rms - meanx_descend) * (atm_model[n].rms - meanx_descend);
	    covy_descend += atm_model[n].tweight * (atm_model[n].rms_ad_model - meany_descend) * (atm_model[n].rms_ad_model - meany_descend);
	    covxy_descend += atm_model[n].tweight * (atm_model[n].rms - meanx_descend) * (atm_model[n].rms_ad_model - meany_descend);
	}
    }
    covx /= weight;
    covy /= weight;
    covxy /= weight;
    covx_ascend /= weight_ascend;
    covy_ascend /= weight_ascend;
    covxy_ascend /= weight_ascend;
    covx_descend /= weight_descend;
    covy_descend /= weight_descend;
    covxy_descend /= weight_descend;

    *corr = covxy/sqrt(covx * covy);
    *corr_ascend = covxy_ascend/sqrt(covx_ascend * covy_ascend);
    *corr_descend = covxy_descend/sqrt(covx_descend * covy_descend);

    //Calculate reflective covariance
    covx = 0;
    covy = 0;
    covxy = 0;
    meanx = 0;
    meany = 0;
    covx_ascend = 0;
    covy_ascend = 0;
    covxy_ascend = 0;
    meanx_ascend = 0;
    meany_ascend = 0;
    covx_descend = 0;
    covy_descend = 0;
    covxy_descend = 0;
    meanx_descend = 0;
    meany_descend = 0;
    for (n = 0; n < atm_np; n++) {
	covx += atm_model[n].tweight * (atm_model[n].rms - meanx) * (atm_model[n].rms - meanx);
	covy += atm_model[n].tweight * (atm_model[n].rms_model - meany) * (atm_model[n].rms_model - meany);
	covxy += atm_model[n].tweight * (atm_model[n].rms - meanx) * (atm_model[n].rms_model - meany);
	if (atm_model[n].ascend) {
	    covx_ascend += atm_model[n].tweight * (atm_model[n].rms - meanx_ascend) * (atm_model[n].rms - meanx_ascend);
	    covy_ascend += atm_model[n].tweight * (atm_model[n].rms_ad_model - meany_ascend) * (atm_model[n].rms_ad_model - meany_ascend);
	    covxy_ascend += atm_model[n].tweight * (atm_model[n].rms - meanx_ascend) * (atm_model[n].rms_ad_model - meany_ascend);
	}
	else {
	    covx_descend += atm_model[n].tweight * (atm_model[n].rms - meanx_descend) * (atm_model[n].rms - meanx_descend);
	    covy_descend += atm_model[n].tweight * (atm_model[n].rms_ad_model - meany_descend) * (atm_model[n].rms_ad_model - meany_descend);
	    covxy_descend += atm_model[n].tweight * (atm_model[n].rms - meanx_descend) * (atm_model[n].rms_ad_model - meany_descend);
	}
    }
    covx /= weight;
    covy /= weight;
    covxy /= weight;
    covx_ascend /= weight_ascend;
    covy_ascend /= weight_ascend;
    covxy_ascend /= weight_ascend;
    covx_descend /= weight_descend;
    covy_descend /= weight_descend;
    covxy_descend /= weight_descend;

    *corref = covxy/sqrt(covx * covy);
    *corref_ascend = covxy_ascend/sqrt(covx_ascend * covy_ascend);
    *corref_descend = covxy_descend/sqrt(covx_descend * covy_descend);

    return 1;
}

//#################################################### main ######################################################

int main(char argc, char *argv[])
{
    FILE *fp;
    char atmfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel_cloud_sorted.csv"; // what s this?
    char outfile[256] = "/home/mare/Projects/MISR/Julienne/IceBridge2016/SeaIce_Jul2016_atmmodel2_r025.csv"; // Ehsan: output at this path; input to <misr2rms.c>
    char razimuth[256] =  "/home/mare/Projects/MISR/Julienne/IceBridge2016/RelativeAzimuth_Jul2016_sorted.txt"; // what s this?
    char command[256];
    char wc_out[256];
    char *words;
    char *sline = NULL;
    char buf[19];

    int l, n, m, r, w;
    double radius, avg_rms;
    double radius_ascend, radius_descend;
    double weight_ascend, weight_descend;
    double avg_ascend_rms, avg_descend_rms;
    double dist, dist_min, rms_dist_min;
    double dist_ascend_min, rms_dist_ascend_min;
    double dist_descend_min, rms_dist_descend_min;
    double corr, corref, scorr;
    double corr_ascend, corref_ascend;
    double corr_descend, corref_descend;
    double corr_max = -1e23;
    double corr_ascend_max = -1e23;
    double corr_descend_max = -1e23;
    double rad_max, rad_ascend_max, rad_descend_max;
    double lat, lon, an, ca, cf, rms, weight, tweight; 

    size_t slen = 0;
    ssize_t read;

    int start_orbit, end_orbit, blocks;
    int raz_nlines;
    int *raz_table;
    int path, orbit, block, line, sample;
    int ascend;
    int cloud;
    int block1, block2;
    int atm_ascend_05_np = 0;
    int atm_ascend_10_np = 0;
    int atm_descend_05_np = 0;
    int atm_descend_10_np = 0;
    
    sprintf(command, "wc -l %s", razimuth);
    fp = popen(command, "r");
    if (fp == NULL) {
	printf("Failed to run command\n" );
	exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(wc_out, sizeof(wc_out)-1, fp) != NULL) {
	//printf("%s", wc_out);
	words = strtok(wc_out, " ");
	w = 0;
	if (w == 0) raz_nlines = atoi(words);
	//printf("nlines= %d\n", raz_nlines);
    }

    /* close */
    pclose(fp);

    fp = fopen(razimuth, "r");
    if (!fp) {
	fprintf(stderr, "main: couldn't open %s\n", razimuth);
	return 1;
    }
    l = 0;
    while ((read = getline(&sline, &slen, fp)) != -1) {
	words = strtok(sline, " ");
	w = 0;
	while ((l == 0) && (words != NULL)) {
	    if (w == 1) start_orbit = atoi(words);
    	    words = strtok (NULL, " ");
	    w++;
	}
	while ((l == raz_nlines - 1) && (words != NULL)) {
	    if (w == 1) end_orbit = atoi(words);
    	    words = strtok (NULL, " ");
	    w++;
	}
	l++;
    }
    //printf("start_orbit= %d  end_orbit= %d  blocks= %d\n", start_orbit, end_orbit, blocks);
    fseek(fp, 0, SEEK_SET);

    raz_table = (int *) malloc((end_orbit - start_orbit + 1) * sizeof(int));
    while ((read = getline(&sline, &slen, fp)) != -1) {
	words = strtok(sline, " ");
	w = 0;
	while (words != NULL) {
	    if (w == 1) orbit = atoi(words);
	    if (w == 3) blocks = 100*atoi(words);
	    if (w == 4) blocks += atoi(words);
    	    words = strtok (NULL, " ");
	    w++;
	}
	raz_table[orbit - start_orbit] = blocks;
    }

    fclose(fp);

    fp = fopen(atmfile, "r");
    if (!fp) {
	fprintf(stderr, "main: couldn't open %s\n", atmfile);
	return 1;
    }
    ascend = 0;
    while ((read = getline(&sline, &slen, fp)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", sline);
	words = strtok (sline," ,");
	w = 0;
	if (atm_np == 0) atm_model = (atm_type * ) malloc(sizeof(atm_type));
	else atm_model = (atm_type * ) realloc(atm_model, (atm_np + 1) * sizeof(atm_type));

	atm_model[atm_np].ascend = 0;

  	while (words != NULL) {
    	    //printf ("%s\n",words);
	    if (w == 0) path = atoi(words);
	    if (w == 1) orbit = atoi(words);
	    if (w == 2) block = atoi(words);
	    if (w == 3) line = atoi(words);
	    if (w == 4) sample = atoi(words);
	    if (w == 5) lat = atof(words);
	    if (w == 6) lon = atof(words);
	    if (w == 7) an = atof(words);
	    if (w == 8) ca = atof(words);
	    if (w == 9) cf = atof(words);
	    if (w == 10) rms = atof(words);
	    if (w == 11) weight = atof(words);
	    if (w == 12) tweight = atof(words);
	    if (w == 14) {
		cloud = atoi(words);
		if (cloud == 0) {
		    atm_model[atm_np].path = path;
		    atm_model[atm_np].orbit = orbit;
		    atm_model[atm_np].block = block;
		    atm_model[atm_np].line = line;
		    atm_model[atm_np].sample = sample;
		    atm_model[atm_np].lat = lat;
		    atm_model[atm_np].lon = lon;
		    atm_model[atm_np].an = an;
		    atm_model[atm_np].ca = ca;
		    atm_model[atm_np].cf = cf;
		    atm_model[atm_np].rms = rms;
		    atm_model[atm_np].weight = weight;
		    atm_model[atm_np].tweight = tweight;
		    atm_model[atm_np].ascend = ascend;
		}
	    }

	    if (w == 1) {
		block1 = (int) raz_table[atoi(words) - start_orbit]/100;
		block2 = raz_table[atoi(words) - start_orbit] % 100;
	    }
	    if (w == 2) {
		ascend = 0;
		if ((atoi(words) < 20) || ((block1 <= atoi(words)) && (atoi(words) <= block2))) {
		    ascend = 1;
		}
	    }
    	    words = strtok (NULL, " ,");
	    w++;
  	}
	if (cloud == 0) atm_np++;
    }
    printf("atm_np = %d\n", atm_np);

    for (r = 1; r <= 25; r++) {
	if (r < 6) radius = r * .001;
	else radius = (r - 5) * .005;
	for (n = 0; n < atm_np; n++) {
	    avg_rms = 0;
	    weight = 0;
            dist_min = 1e23;
	    avg_ascend_rms = 0;
	    weight_ascend = 0;
            dist_ascend_min = 1e23;
	    avg_descend_rms = 0;
	    weight_descend = 0;
            dist_descend_min = 1e23;
	    if (r==1) {
		if (atm_model[n].ascend) {
		    if (atm_model[n].weight == 0.5) atm_ascend_05_np += 1;
		    else atm_ascend_10_np += 1;
		}
		else {
		    if (atm_model[n].weight == 0.5) atm_descend_05_np += 1;
		    else atm_descend_10_np += 1;
		}
	    }
	    for (m = 0; m < atm_np; m++) {
		if (m == n) continue;
		if (atm_model[n].ascend != atm_model[m].ascend) continue;
		else {
		    if (~atm_model[n].ascend || ((atm_model[n].block < 20) && (atm_model[m].block < 20)) ||
			    ((atm_model[n].block > 20) && (atm_model[m].block > 20))) {
			dist = sqrt((atm_model[m].an - atm_model[n].an) * (atm_model[m].an - atm_model[n].an)
			    + (atm_model[m].ca - atm_model[n].ca) * (atm_model[m].ca - atm_model[n].ca)
			    + (atm_model[m].cf - atm_model[n].cf) * (atm_model[m].cf - atm_model[n].cf));
		    }
		    else {
			dist = sqrt((atm_model[m].an - atm_model[n].an) * (atm_model[m].an - atm_model[n].an)
			    + (atm_model[m].cf - atm_model[n].ca) * (atm_model[m].cf - atm_model[n].ca)
			    + (atm_model[m].ca - atm_model[n].cf) * (atm_model[m].ca - atm_model[n].cf));
		    }
		}
		if (dist <= radius) {
		    avg_rms += atm_model[m].tweight * atm_model[m].rms;
		    weight += atm_model[m].tweight;
		    if (atm_model[n].ascend) {
			avg_ascend_rms += atm_model[m].tweight * atm_model[m].rms;
			weight_ascend += atm_model[m].tweight;
		    }
		    else {
			avg_descend_rms += atm_model[m].tweight * atm_model[m].rms;
			weight_descend += atm_model[m].tweight;
		    }
		}
		if (dist < dist_min) {
		    dist_min = dist;
		    rms_dist_min = atm_model[m].rms;
		}
		if (atm_model[n].ascend) {
		    if (dist < dist_ascend_min) {
			dist_ascend_min = dist;
			rms_dist_ascend_min = atm_model[m].rms;
		    }
		}
		else {
		    if (dist < dist_descend_min) {
			dist_descend_min = dist;
			rms_dist_descend_min = atm_model[m].rms;
		    }
		}
	    }	
	    if (avg_rms == 0) avg_rms = rms_dist_min;
	    else avg_rms /= weight;
	    atm_model[n].rms_model = avg_rms;

	    if (atm_model[n].ascend) {
		if (avg_ascend_rms == 0) avg_ascend_rms = rms_dist_ascend_min;
		else avg_ascend_rms /= weight_ascend;
		atm_model[n].rms_ad_model = avg_ascend_rms;
	    }
	    else {
		if (avg_descend_rms == 0) avg_descend_rms = rms_dist_descend_min;
		else avg_descend_rms /= weight_descend;
		atm_model[n].rms_ad_model = avg_descend_rms;
	    }
	} 
	if (!pearson(&corr, &corref , &corr_ascend, &corref_ascend, &corr_descend, &corref_descend)) return 1;
	//if (!spearman(&scorr)) return 1;
	//printf("%lf %lf %lf %lf\n", radius, corr, corref, scorr);
	printf("%lf %lf %lf %lf %lf %lf %lf\n", radius, corr, corref, corr_ascend, corref_ascend, corr_descend, corref_descend);
	if (r > 6) {
	    if (corref > corr_max) { 
		corr_max = corref;
		rad_max = radius;
	    }
	    if (corref_ascend > corr_ascend_max) { 
		corr_ascend_max = corref_ascend;
		rad_ascend_max = radius;
	    }
	    if (corref_descend > corr_descend_max) { 
		corr_descend_max = corref_descend;
		rad_descend_max = radius;
	    }
	}
	//break;
    }
    printf("Total number of model points: %d\n", atm_np);
    printf("Total number of ascending points (weight=1.0): %d\n", atm_ascend_10_np);
    printf("Total number of ascending points (weight=0.5): %d\n", atm_ascend_05_np);
    printf("Total number of descending points (weight=1.0): %d\n", atm_descend_10_np);
    printf("Total number of descending points (weight=0.5): %d\n\n", atm_descend_05_np);

    fp = fopen(outfile, "w");
    if (!fp) {
	fprintf(stderr, "main: couldn't open %s\n", outfile);
	return 1;
    }

    fprintf(fp, "path, orbit, block, line, sample, lat, lon, an, ca, cf, rms_atm, rms_model, rms_ad_model, weight, tweight, ascend\n");

   radius = rad_max;
   radius_ascend = rad_ascend_max;
   radius_descend = rad_descend_max;
   radius = 0.025;
   printf("Optimum Combined Radius: %f\n", radius);
   printf("Ascend Radius: %f\n", radius_ascend);
   printf("Descend Radius: %f\n", radius_descend);

   for (n = 0; n < atm_np; n++) {
	avg_rms = 0;
	weight = 0;
        dist_min = 1e23;
	avg_ascend_rms = 0;
	weight_ascend = 0;
        dist_ascend_min = 1e23;
	avg_descend_rms = 0;
	weight_descend = 0;
        dist_descend_min = 1e23;
	for (m = 0; m < atm_np; m++) {
	    if (m == n) continue;
	    if (atm_model[n].ascend != atm_model[m].ascend) continue;
	    else {
		if (~atm_model[n].ascend || ((atm_model[n].block < 20) && (atm_model[m].block < 20)) ||
			((atm_model[n].block > 20) && (atm_model[m].block > 20))) {
		    dist = sqrt((atm_model[m].an - atm_model[n].an) * (atm_model[m].an - atm_model[n].an)
			+ (atm_model[m].ca - atm_model[n].ca) * (atm_model[m].ca - atm_model[n].ca)
			+ (atm_model[m].cf - atm_model[n].cf) * (atm_model[m].cf - atm_model[n].cf));
		}
		else {
		    dist = sqrt((atm_model[m].an - atm_model[n].an) * (atm_model[m].an - atm_model[n].an)
			+ (atm_model[m].cf - atm_model[n].ca) * (atm_model[m].cf - atm_model[n].ca)
			+ (atm_model[m].ca - atm_model[n].cf) * (atm_model[m].ca - atm_model[n].cf));
		}
	    }
	    if (dist <= radius) {
		avg_rms += atm_model[m].tweight * atm_model[m].rms;
		weight += atm_model[m].tweight;
	    }
	    if ((dist <= radius_ascend) && atm_model[n].ascend) {
		avg_ascend_rms += atm_model[m].tweight * atm_model[m].rms;
		weight_ascend += atm_model[m].tweight;
	    }
	    if ((dist <= radius_descend) && ~atm_model[n].ascend) {
		avg_descend_rms += atm_model[m].tweight * atm_model[m].rms;
		weight_descend += atm_model[m].tweight;
	    }
	    if (dist < dist_min) {
		dist_min = dist;
		rms_dist_min = atm_model[m].rms;
	    }
	    if (atm_model[n].ascend) {
		if (dist < dist_ascend_min) {
		    dist_ascend_min = dist;
		    rms_dist_ascend_min = atm_model[m].rms;
		}
	    }
	    else {
		if (dist < dist_descend_min) {
		    dist_descend_min = dist;
		    rms_dist_descend_min = atm_model[m].rms;
		}
	    }
	}	
	if (avg_rms == 0) avg_rms = rms_dist_min;
	else avg_rms /= weight;
	atm_model[n].rms_model = avg_rms;

	if (atm_model[n].ascend) {
	    if (avg_ascend_rms == 0) avg_ascend_rms = rms_dist_ascend_min;
	    else avg_ascend_rms /= weight_ascend;
	    atm_model[n].rms_ad_model = avg_ascend_rms;
	}
	else {
	    if (avg_descend_rms == 0) avg_descend_rms = rms_dist_descend_min;
	    else avg_descend_rms /= weight_descend;
	    atm_model[n].rms_ad_model = avg_descend_rms;
	}

    	fprintf(fp, "%d, %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %f, %f, %d\n", atm_model[n].path, atm_model[n].orbit, atm_model[n].block, atm_model[n].line, atm_model[n].sample, atm_model[n].lat, atm_model[n].lon, atm_model[n].an, atm_model[n].ca, atm_model[n].cf, atm_model[n].rms, atm_model[n].rms_model, atm_model[n].rms_ad_model, atm_model[n].weight, atm_model[n].tweight, atm_model[n].ascend);
    }

    return 0;
}
