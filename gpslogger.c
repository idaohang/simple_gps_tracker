#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <libpq-fe.h>
#include <gps.h>
#include "gpsdclient.h"

static char*  progname;
static struct fixsource_t source;

static PGconn*   connection  ;
static PGresult* query_result;
static int       query_result_status;

static struct gps_data_t gpsdata;
static bool   intrack = false;
static time_t timeout = 5; /* seconds */
static double minmove = 0; /* meters */

static void print_fix(struct gps_data_t *gpsdata, double time){
	static char sbuf[140              ];
	static char tbuf[CLIENT_DATE_MAX+1];
	sprintf(sbuf, "insert into tracker_tracker(date, point) values ('%s', ST_GeomFromText('POINT(%f %f)',4326));", gpsdata->fix.latitude, gpsdata->fix.longitude, unix_to_iso8601(time, tbuf, sizeof(tbuf)));
	fprintf(stdout, "%s\n", sbuf);
	if((query_result = PQexec( connection, sbuf )) == NULL){
		fprintf(stderr, "%s: error inserting gps record\n", progname, PQerrorMessage( connection ));
	}

	if((query_result_status = PQresultStatus(query_result)) != PGRES_COMMAND_OK){
		fprintf(stderr, "%s: sql command failed :%s for %s\n", progname, PQresStatus(query_result_status), sbuf);
	}
}

static void conditionally_log_fix(struct gps_data_t *gpsdata){
	static double int_time, old_int_time;
	static double old_lat , old_lon;
	static bool   first = true;

	int_time = gpsdata->fix.time;
	if ((int_time == old_int_time) || gpsdata->fix.mode < MODE_2D)
		return;

	if (minmove>0 && !first && earth_distance(gpsdata->fix.latitude, gpsdata->fix.longitude, old_lat, old_lon) < minmove)
		return;

	old_int_time = int_time;
	if (minmove > 0) {
		old_lat = gpsdata->fix.latitude;
		old_lon = gpsdata->fix.longitude;
	}
	print_fix(gpsdata, int_time);
}

static void quit_handler(int signum){
	if (signum != SIGINT)
		fprintf(stderr, "exiting, signal %d received", signum);
	gps_close(&gpsdata);
	exit(0);
}

int main(int argc, char **argv){
	progname = argv[0];
	if(argc != 3){
		fprintf(stderr, "usage : gpslogger IP PORT\n");
		exit(1);
	}

	signal(SIGINT , quit_handler);
	signal(SIGTERM, quit_handler);
	signal(SIGQUIT, quit_handler);


	if (daemon(0, 0) != 0){
		 fprintf(stderr,"demonization failed: %s\n", strerror(errno));
		 exit(1);
	}

	if (gps_open(argv[1], argv[2], &gpsdata) != 0) {
		fprintf(stderr, "%s: no gpsd running or network error: %d, %s\n", progname, errno, gps_errstr(errno));
		exit(1);
	}

	connection = PQconnectdb("dbname=tracking user=postgres");
	if(connection == NULL){
		fprintf(stderr, "%s: unable to allocate database connection\n", progname);
		exit(1);
	}

	if( PQstatus( connection ) != CONNECTION_OK ){
		fprintf(stderr, "%s: database connection error : %s\n", progname, PQerrorMessage( connection ));
		exit(1);
	}

	gps_stream(&gpsdata, WATCH_ENABLE, source.device);
	gps_mainloop(&gpsdata, 5000000, conditionally_log_fix);
	gps_close(&gpsdata);
	PQfinish( connection);

	exit(0);
}