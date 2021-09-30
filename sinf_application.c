#include "sinf_interfaces.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <postgresql/libpq-fe.h>

/*
 * In order to add or remove rooms from this working version, two changes need to be made.
 * 
 * 1) Change the definition NUMBER_ROOMS to the desired value, e.g. 2 if you only want to rooms
 * 2) Change the global variable temp_threshold with the intended threasholds. The number of thresholds
 * should match the NUMBER_ROOMS, e.g. int temp_threshold[NUMBER_ROOMS] = {8,17};
 * 
 * After these changes, you only need to compile, build and execute your code.
 * 
 * */


#define MAX_NAME_SZ 256
#define NUMBER_ROOMS 1
#define NUMBER_CELLS 5

int temp_threshold[NUMBER_ROOMS] = {9999}; //Default value to be overwritten

int print_values = 0; // Change to 1 to print in stdin, 0 to disable prints


int main() {	

    PGconn *conn;
    
    const char *dbconn;
    
    dbconn = "host='db.fe.up.pt' dbname='sinf2021a42' user='sinf2021a42' password='HhuSzRsb'";
    
    conn = PQconnectdb(dbconn);
	
	if (PQstatus(conn) == CONNECTION_BAD) {
		
		if(print_values) printf("Unable to connect\n");
		
		exit(-1);
		
	}
	
	else {
		
		if(print_values) printf("Able to connect!\n");
		
	}
	
	PQexec(conn, "SET SEARCH_PATH TO sprint2;");
	
	/////////////////////////////////////////
	
	struct Measurements sensors[NUMBER_ROOMS];
	
	struct Actuators actuators[NUMBER_ROOMS];

	int max_temp=0, min_temp=9999, max_light=0, min_light=9999, max_hum=0, min_hum=9999;
	
	char str[MAX_NAME_SZ], location[40], query_string[4000];
	
	int prev=0; //Variable to store previous value of the heater power state
	
	fgets(str, MAX_NAME_SZ, stdin);
	
	PGresult *query;

    query = PQexec(conn, "SELECT * FROM rules");
    
    if (PQresultStatus(query) == PGRES_TUPLES_OK){
		
		if(print_values) printf("Tuples OK!\n");
			
		int nt = PQntuples(query);
		
		if(print_values) printf("nt = %d\n", nt);
			
		if(nt != 0){
					
			if(print_values) printf("%d\n", atoi(PQgetvalue(query, 0, 2)));
				
			temp_threshold[0] = atoi(PQgetvalue(query, 0, 2));
		
		}
		
		else {
				
			if(print_values) printf("No rows to fetch! \n");
			
		}
		
	}
	
	else {
			
		if(print_values) printf("DB query call not ok!\n");
			
	}
		
	PQclear(query);
	
	while(1){
		
		fgets(str, MAX_NAME_SZ, stdin);
		
		readSensors(sensors,str);

        strcpy(location, "Ambiente");

        sprintf(query_string, "INSERT INTO mote(num, location) VALUES (%d, '%s');", sensors[0].mote_id, location);

        PQexec(conn, query_string);

        sprintf(query_string, "INSERT INTO sensor_quantities(cod_s, num, type, value) VALUES (%d, %d, 'temperature', %d);", 1, sensors[0].mote_id, sensors[0].temperature);

        PQexec(conn, query_string);
        
        if(max_temp < sensors[0].temperature)
        
			max_temp = sensors[0].temperature;
			
		if(min_temp > sensors[0].temperature)
        
			min_temp = sensors[0].temperature;	
        
        sprintf(query_string, "INSERT INTO sensor_history(cod_hs , cod_s, type, maximum_value, minimum_value) VALUES (%d, %d, 'temperature', %d, %d);", 1, 1, max_temp, min_temp);

        PQexec(conn, query_string);

        sprintf(query_string, "INSERT INTO sensor_quantities(cod_s, num, type, value) VALUES (%d, %d, 'humidity', %d);", 2, sensors[0].mote_id, sensors[0].humidity);
        
        PQexec(conn, query_string);
        
        if(max_hum < sensors[0].humidity)
        
			max_hum = sensors[0].humidity;
			
		if(min_hum > sensors[0].humidity)
        
			min_hum = sensors[0].humidity;

        sprintf(query_string, "INSERT INTO sensor_history(cod_hs , cod_s, type, maximum_value, minimum_value) VALUES (%d, %d, 'humidity', %d, %d);", 1, 1, max_hum, min_hum);    
        
        PQexec(conn, query_string);

        sprintf(query_string, "INSERT INTO sensor_quantities(cod_s, num, type, value) VALUES (%d, %d, 'light', %d);", 4, sensors[0].mote_id, sensors[0].light);
        
        PQexec(conn, query_string);
        
        if(max_light < sensors[0].light)
        
			max_light = sensors[0].light;
			
		if(min_light > sensors[0].light)
        
			min_light = sensors[0].light;
        
        sprintf(query_string, "INSERT INTO sensor_history(cod_hs , cod_s, type, maximum_value, minimum_value) VALUES (%d, %d, 'light', %d, %d);", 1, 1, max_light, min_light);

        PQexec(conn, query_string);
		
		prev = actuators[0].heater_on;
		
		checkRules(sensors,actuators,NUMBER_ROOMS,temp_threshold);

        sprintf(query_string, "INSERT INTO actuators(cod_a, type, power, num) VALUES (%d, 'heater', %d, %d);", 1, actuators[0].heater_on, sensors->mote_id);
		
		PQexec(conn, query_string);

        sprintf(query_string, "INSERT INTO actuator_history(cod_ha, cod_a, type, previous_value) VALUES (%d, %d, 'heater', %d);", 1, 1, prev);
        
        PQexec(conn, query_string);
		
		if (print_values) {
			
			printf("Temperature: %d\n", sensors[0].temperature);
			
			printf("Humidity: %d\n", sensors[0].humidity);
			
			printf("Light: %d\n", sensors[0].light);
			
			printf("Mote: %d\n", sensors[0].mote_id);
			
		}
		
		else {
			
			writeRGBMatrix(sensors,actuators,NUMBER_ROOMS,temp_threshold,NUMBER_CELLS);	
			
		}
		
	}

    PQfinish(conn);
	
	return 0;
}
