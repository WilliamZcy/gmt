/*	$Id: x_remove.c,v 1.8 2010-03-24 02:36:45 guru Exp $
 *
 * XREMOVE will read a list of bad legs from a file, and then remove all
 * trace of these files from the x_system data base files. New x_system files
 * will be created with the suffix _new appended to the file names.
 * All COE generated by the bad legs will be skipped when writing out the
 * new files, and the mean/stdev statistics will be recalculated
 *
 * Author:	Paul Wessel
 * Date:	25-JAN-1988
 * Last rev:	06-MAR-2000	POSIX
 *
 */
 
#include "gmt.h"
#include "x_system.h"

#define MAXLEGS 10000
#define MAXBADLEGS 1000
#define MAX_X 10000

struct XOVERS crossover[MAX_X];

struct LEG leg[MAXLEGS];

size_t legsize = sizeof (struct LEG);
GMT_LONG nbadlegs = 0, nlegs = 0;

char badlegs[MAXBADLEGS][10];

GMT_LONG findleg (char *name);
GMT_LONG get_id (char *name);

int main (int argc, char **argv)
{
	FILE *fpb = NULL, *fpl = NULL, *fpr = NULL, *fpl2, *fpb2;
	GMT_LONG i, j, n_x, error = FALSE, id1, id2, internal, ok, nrecs = 1, verbose = FALSE;
	char header[BUFSIZ], lega[10], legb[10], line[BUFSIZ];
	double mean;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'X':
					fpb = fopen (&argv[i][2], "rb");
					break;
				case 'L':
					fpl = fopen (&argv[i][2], "rb");
					break;
				case 'V':
					verbose = TRUE;
					break;
				default:
					error = TRUE;
					break;
			}
		}
		else
			fpr = fopen (argv[i], "r");
	}

	if (fpr == NULL) error = TRUE;
	if (argc == 1 || error) {
		fprintf(stderr, "xremove - Remove selected info from xover database\n\n");
		fprintf(stderr, "usage: xremove removefile [-X<xbasefile> -L<xlegsfile> -V]\n");
		fprintf(stderr, "	removefile is a list of legs to remove\n");
		fprintf(stderr, "	-X to select alternate xx_base.b file\n");
		fprintf(stderr, "	-L to select alternate xx_legs.b file\n");
		fprintf(stderr, "	-V Verbose, report when removing a leg\n");
		exit (EXIT_FAILURE);
	}

	if (fpb == NULL && (fpb = fopen("xx_base.b","rb")) == NULL) {
		fprintf (stderr, "Could not find xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	if (fpl == NULL && (fpl = fopen("xx_legs.b","rb")) == NULL) {
		fprintf (stderr, "Could not find xx_legs.b\n");
		exit (EXIT_FAILURE);
	}

	/* Read all the leg info into memory */

	nlegs = 0;
	while (fread ((void *)(&leg[nlegs]), legsize, (size_t)1, fpl) == (size_t)1) {
		for (j = 0; j < 3; j++) {
			leg[nlegs].n_gmtint[j] = leg[nlegs].n_gmtext[j] = 0;
			leg[nlegs].mean_gmtint[j] = leg[nlegs].mean_gmtext[j] = 0.0;
			leg[nlegs].st_dev_gmtint[j] = leg[nlegs].st_dev_gmtext[j] = 0.0;
			leg[nlegs].dc_shift_gmt[j] = leg[nlegs].drift_rate_gmt[j] = 0.0;
		}
		leg[nlegs].n_x_int = leg[nlegs].n_x_ext = 0;
		leg[nlegs].next_leg = 0;
		nlegs++;
		if (nlegs == MAXLEGS) {
			fprintf (stderr, "xremove: nlegs > MAXLEGS, recompile!\n");
			exit (EXIT_FAILURE);
		}
	}
	fclose (fpl);

	while (fgets (line, BUFSIZ, fpr)) {
		sscanf (line, "%s", badlegs[nbadlegs]);
		nbadlegs++;
		if (nbadlegs == MAXBADLEGS) {
			fprintf (stderr, "xremove: nbadlegs > MAXBADLEGS, recompile!\n");
			exit (EXIT_FAILURE);
		}
	}
	fclose (fpr);

	if ((fpb2 = fopen("xx_base.b_new", "wb")) == NULL) {
		fprintf (stderr, "Could not create xx_base.b_new\n");
		exit (EXIT_FAILURE);
	}
	if ((fpl2 = fopen("xx_legs.b_new","wb")) == NULL) {
		fprintf (stderr, "Could not create xx_legs.b_new\n");
		exit (EXIT_FAILURE);
	}

	if (fread((void *)header, (size_t)REC_SIZE, (size_t)1, fpb) != (size_t)1) {
		fprintf (stderr, "xremove: Read error on xx_base.b\n");
		exit (EXIT_FAILURE);
	}

	if (fwrite ((void *)header, (size_t)REC_SIZE, (size_t)1, fpb2) != (size_t)1) {
		fprintf (stderr, "xremove: Write error on xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	ok = fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fpb);
	while (ok) {
		sscanf(header, "%s %s %" GMT_LL "d",lega, legb, &n_x);
		if (findleg (lega) || findleg (legb)) {
			if (verbose) fprintf (stderr, "xremove: Skipping %s - %s\n", lega, legb);
			fseek (fpb, (long int)(REC_SIZE*n_x), SEEK_CUR);
		}
		else {
			internal = !strcmp(lega, legb);
			id1 = get_id (lega);
			id2 = (internal) ? id1 : get_id (legb);
			if (n_x > MAX_X) {
				fprintf (stderr, "xremove: nx (= %ld) > MAX_X, recompile!\n", n_x);
				exit (EXIT_FAILURE);
			}
			if (fread ((void *)crossover, (size_t)REC_SIZE, (size_t)n_x, fpb) != (size_t)n_x) {
          			fprintf (stderr, "xremove: Read error on xx_base.b\n");
          			exit (EXIT_FAILURE);
          		}
          		if (fwrite ((void *)header, (size_t)REC_SIZE, (size_t)1, fpb2) != (size_t)1) {
          			fprintf (stderr, "xremove: Write error on xx_base.b\n");
          			exit (EXIT_FAILURE);
          		}
          		for (i = 0; i < n_x; i++) {
          			if (internal) {
          				for (j = 0; j < 3; j++) {
          					if (crossover[i].x_val[j] == NODATA) continue;
          					leg[id1].mean_gmtint[j] += crossover[i].x_val[j];
          					leg[id1].st_dev_gmtint[j] += crossover[i].x_val[j]*crossover[i].x_val[j];
          					leg[id1].n_gmtint[j]++;
          				}
          			}
          			else {
          				for (j = 0; j < 3; j++) {
          					if (crossover[i].x_val[j] == NODATA) continue;
          					leg[id1].mean_gmtext[j] += crossover[i].x_val[j];
          					leg[id1].st_dev_gmtext[j] += crossover[i].x_val[j]*crossover[i].x_val[j];
          					leg[id1].n_gmtext[j]++;
          					leg[id2].mean_gmtext[j] -= crossover[i].x_val[j];
          					leg[id2].st_dev_gmtext[j] += crossover[i].x_val[j]*crossover[i].x_val[j];
          					leg[id2].n_gmtext[j]++;
          				}
          			}
          		}
           		if (fwrite ((void *)crossover, (size_t)REC_SIZE, (size_t)n_x, fpb2) != (size_t)n_x) {
           			fprintf (stderr, "xremove: Write error on xx_base.b\n");
           			exit (EXIT_FAILURE);
           		}
         		if (internal)
          			leg[id1].n_x_int = n_x;
          		else {
          			leg[id1].n_x_ext += n_x;
          			leg[id2].n_x_ext += n_x;
          		}
          		nrecs += n_x + 1;
          	}
		ok = fread ((void*)header, (size_t)REC_SIZE, (size_t)1, fpb);
	}
	fclose (fpb);
	sprintf (header, "%10ld xx_base.b header", nrecs);
	fseek (fpb2, 0L, SEEK_SET);
	if (fwrite ((void *)header, (size_t)REC_SIZE, (size_t)1, fpb2) != (size_t)1) {
		fprintf (stderr, "xremove: Write error on xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	fclose (fpb2);

	if (verbose) fprintf (stderr, "xremove: New xx_legs.b-file created successfully\n");

	for (i = 0; i < nlegs; i++) {
		if (findleg (leg[i].name)) continue;

		for (j = 0; j < 3; j++) {
			/* Compute statistics for internal crossovers */

			mean = (leg[i].n_gmtint[j]) ? leg[i].mean_gmtint[j] / leg[i].n_gmtint[j] : 0.0;
			leg[i].st_dev_gmtint[j] = (leg[i].n_gmtint[j] > 1) ? 
				sqrt((leg[i].st_dev_gmtint[j]-mean*leg[i].mean_gmtint[j])/(leg[i].n_gmtint[j]-1)) : 0.0;
			leg[i].mean_gmtint[j] = mean;

			/* Same for external crossovers */

			mean = (leg[i].n_gmtext[j]) ? leg[i].mean_gmtext[j] / leg[i].n_gmtext[j] : 0.0;
			leg[i].st_dev_gmtext[j] = (leg[i].n_gmtext[j] > 1) ?
				sqrt((leg[i].st_dev_gmtext[j]-mean*leg[i].mean_gmtext[j])/(leg[i].n_gmtext[j]-1)) : 0.0;
			leg[i].mean_gmtext[j] = mean;
		}
	}
	if (fwrite ((void *)leg, legsize, (size_t)nlegs, fpl2) != (size_t)nlegs) {
		fprintf (stderr, "xremove: Write error for xx_legs.b file\n");
		exit (EXIT_FAILURE);
	}
	fclose (fpl2);
	if (verbose) fprintf (stderr, "xremove: New xx_legs.b-file created successfully\n");
	exit (EXIT_SUCCESS);
}
          
GMT_LONG findleg (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nbadlegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, badlegs[mid]);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (1);
	}
	return (0);
}

GMT_LONG get_id (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nlegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, leg[mid].name);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (mid);
	}
	return (-1);
}
