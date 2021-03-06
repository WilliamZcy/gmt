/*
 * Copyright (c) 2012-2018
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_supplements_module.c populates the external array of GMT supplements with
 * module parameters such as name, group, purpose and keys strings.
 * This file also contains the following convenience functions to
 * display all module purposes or just list their names:
 *
 *   void gmt_supplements_module_show_all (struct GMTAPI_CTRL *API);
 *   void gmt_supplements_module_list_all (void *API);
 *
 * These functions may be called by gmt --help and gmt --show-modules
 *
 * Developers of external APIs for accessing GMT modules will use this
 * function indirectly via GMT_Encode_Options to retrieve option keys
 * needed for module arg processing:
 *
 *   char * gmt_supplements_module_keys (void *API, const char *module);
 *
 * DO NOT edit this file directly! Regenerate the file by running
 * 	gmt_make_module_src.sh supplements
 */
#include "gmt.h"
#include "gmt_notposix.h"       /* Non-POSIX extensions */
#define gmt_M_unused(x) (void)(x)
#define GMT_LEN256 256
#include "gmt_supplements_module.h"
#include <string.h>

/* Sorted array with information for all GMT supplements modules */

/* name, library, and purpose for each module */
struct Gmt_moduleinfo {
	const char *name;             /* Program name */
	const char *component;        /* Component (core, supplement, custom) */
	const char *purpose;          /* Program purpose */
	const char *keys;             /* Program option info for external APIs */
};

static struct Gmt_moduleinfo g_supplements_module[] = {
	{"gshhg", "gshhg", "Extract data tables from binary GSHHS or WDBII data files", ">D}"},
	{"img2grd", "img", "Extract a subset from an img file in Mercator or Geographic format", "<G{,GG}"},
	{"pscoupe", "meca", "Plot cross-sections of focal mechanisms", "<D{,>X}"},
	{"psmeca", "meca", "Plot focal mechanisms on maps", "<D{,>X}"},
	{"pspolar", "meca", "Plot polarities on the inferior focal half-sphere on maps", "<D{,>X}"},
	{"pssac", "meca", "Plot seismograms in SAC format on maps", ">X},RG-"},
	{"psvelo", "meca", "Plot velocity vectors, crosses, and wedges on maps", "<D{,>X}"},
	{"mgd77convert", "mgd77", "Convert MGD77 data to other file formats", ""},
	{"mgd77header", "mgd77", "Create MGD77 headers from A77 files", ""},
	{"mgd77info", "mgd77", "Extract information about MGD77 files", ""},
	{"mgd77list", "mgd77", "Extract data from MGD77 files", ">D}"},
	{"mgd77magref", "mgd77", "Evaluate the IGRF or CM4 magnetic field models", "<D{,>D}"},
	{"mgd77manage", "mgd77", "Manage the content of MGD77+ files", ""},
	{"mgd77path", "mgd77", "Return paths to MGD77 cruises and directories", ">D}"},
	{"mgd77sniffer", "mgd77", "Along-track quality control of MGD77 cruises", ""},
	{"mgd77track", "mgd77", "Plot track-line map of MGD77 cruises", ">X}"},
	{"dimfilter", "misc", "Directional filtering of grids in the space domain", "<G{,GG},>DQ"},
	{"earthtide", "potential", "Compute grids or time-series of solid Earth tides", ">D},GG),>DL,>DS"},
	{"gmtflexure", "potential", "Compute flexural deformation of 2-D loads, forces, and bending moments", "ED(,QD(,TD(,>D}"},
	{"gmtgravmag3d", "potential", "Compute the gravity/magnetic anomaly of a 3-D body by the method of Okabe", "TD{,FD(,GG),>D}"},
	{"gpsgridder", "potential", "Interpolate GPS velocity vectors using Green's functions for a thin elastic sheet", "<D{,ND(,TG(,CD)=f,GG}"},
	{"gravfft", "potential", "Spectral calculations of gravity, isostasy, admittance, and coherence for grids", "<G{+,GG},DG(,GDC,GDI"},
	{"grdflexure", "potential", "Compute flexural deformation of 3-D surfaces for various rheologies", "<G{,GG},LD),TD("},
	{"grdgravmag3d", "potential", "Computes the gravity effect of one (or two) grids by the method of Okabe", "<G{+,FD(,GG}"},
	{"grdredpol", "potential", "Compute the Continuous Reduction To the Pole, AKA differential RTP", "<G{,EG(,GG},ZG)"},
	{"grdseamount", "potential", "Create synthetic seamounts (Gaussian, parabolic, cone or disc, circular or elliptical)", "<T{,GG},TD("},
	{"talwani2d", "potential", "Compute geopotential anomalies over 2-D bodies by the method of Talwani", "<D{,ND(,>D}"},
	{"talwani3d", "potential", "Compute geopotential anomalies over 3-D bodies by the method of Talwani", "<D{,ND(,ZG(,G?},GDN"},
	{"pssegyz", "segy", "Plot a SEGY file on a map in 3-D", ">X}"},
	{"pssegy", "segy", "Plot a SEGY file on a map", ">X}"},
	{"segy2grd", "segy", "Converting SEGY data to a GMT grid", "GG}"},
	{"backtracker", "spotter", "Generate forward and backward flowlines and hotspot tracks", "<D{,>D},FD("},
	{"gmtpmodeler", "spotter", "Evaluate a plate motion model at given locations", "<D{,FD(,>D}"},
	{"grdpmodeler", "spotter", "Evaluate a plate motion model on a geographic grid", "<G{,FD(,GG),>DG"},
	{"grdrotater", "spotter", "Finite rotation reconstruction of geographic grid", "<G{,FD(,GG},TD("},
	{"grdspotter", "spotter", "Create CVA image from a gravity or topography grid", "<G{,AG(,DG),LG),GG}"},
	{"hotspotter", "spotter", "Create CVA image from seamount locations", "<D{,GG}"},
	{"originater", "spotter", "Associate seamounts with nearest hotspot point sources", "<D{,FD(,>D}"},
	{"polespotter", "spotter", "Find stage poles given fracture zones and abyssal hills", "AD(,CD),FD(,GG},LD)"},
	{"rotconverter", "spotter", "Manipulate total reconstruction and stage rotations", ">D}"},
	{"rotsmoother", "spotter", "Get mean rotations and covarience matrices from set of finate rotations", "<D{,>D}"},
	{"x2sys_binlist", "x2sys", "Create bin index listing from track data files", ">D}"},
	{"x2sys_cross", "x2sys", "Calculate crossovers between track data files", ">D}"},
	{"x2sys_datalist", "x2sys", "Extract content of track data files", ">D}"},
	{"x2sys_get", "x2sys", "Get track listing from track index database", ">D}"},
	{"x2sys_init", "x2sys", "Initialize a new x2sys track database", ""},
	{"x2sys_list", "x2sys", "Extract subset from crossover data base", ">D}"},
	{"x2sys_merge", "x2sys", "Merge an updated COEs table (smaller) into the main table (bigger)", ">D}"},
	{"x2sys_put", "x2sys", "Update track index database from track bin file", ""},
	{"x2sys_report", "x2sys", "Report statistics from crossover data base", ">D}"},
	{"x2sys_solve", "x2sys", "Determine least-squares systematic correction from crossovers", ">D}"},
	{NULL, NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* Pretty print all GMT supplements module names and their purposes for gmt --help */
void gmt_supplements_module_show_all (void *V_API) {
	unsigned int module_id = 0;
	char message[GMT_LEN256];
	GMT_Message (V_API, GMT_TIME_NONE, "\n===  GMT suppl: The official supplements to the Generic Mapping Tools  ===\n");
	while (g_supplements_module[module_id].name != NULL) {
		if (module_id == 0 || strcmp (g_supplements_module[module_id-1].component, g_supplements_module[module_id].component)) {
			/* Start of new supplemental group */
			snprintf (message, GMT_LEN256, "\nModule name:     Purpose of %s module:\n", g_supplements_module[module_id].component);
			GMT_Message (V_API, GMT_TIME_NONE, message);
			GMT_Message (V_API, GMT_TIME_NONE, "----------------------------------------------------------------\n");
		}
		snprintf (message, GMT_LEN256, "%-16s %s\n",
			g_supplements_module[module_id].name, g_supplements_module[module_id].purpose);
			GMT_Message (V_API, GMT_TIME_NONE, message);
		++module_id;
	}
}

/* Produce single list on stdout of all GMT supplements module names for gmt --show-modules */
void gmt_supplements_module_list_all (void *V_API) {
	unsigned int module_id = 0;
	gmt_M_unused(V_API);
	while (g_supplements_module[module_id].name != NULL) {
		printf ("%s\n", g_supplements_module[module_id].name);
		++module_id;
	}
}

/* Lookup module id by name, return option keys pointer (for external API developers) */
const char *gmt_supplements_module_keys (void *API, char *candidate) {
	int module_id = 0;
	gmt_M_unused(API);

	/* Match actual_name against g_module[module_id].name */
	while (g_supplements_module[module_id].name != NULL &&
	       strcmp (candidate, g_supplements_module[module_id].name))
		++module_id;

	/* Return Module keys or NULL */
	return (g_supplements_module[module_id].keys);
}

/* Lookup module id by name, return group char name (for external API developers) */
const char *gmt_supplements_module_group (void *API, char *candidate) {
	int module_id = 0;
	gmt_M_unused(API);

	/* Match actual_name against g_module[module_id].name */
	while (g_supplements_module[module_id].name != NULL &&
	       strcmp (candidate, g_supplements_module[module_id].name))
		++module_id;

	/* Return Module keys or NULL */
	return (g_supplements_module[module_id].component);
}
