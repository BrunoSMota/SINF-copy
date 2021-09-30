#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define YELLOW "[255,255,0]" //Luz
#define ORANGE "[255,128,0]" //Temperatura
#define BLUE "[0,0,254]"	 //Humidade
#define WHITE "[255,255,255]"//Lampada
#define GREY "[128,128,128]" //Ar condicionado
#define GREEN "[0,254,0]"	 //Humidificador
#define RED "[254,0,0]"		 //Forno
#define BLACK "[0,0,0]"		 //Desligado ou abaixo/acima do valor limite

/*SECTION1:

    T=25ºC
    L=300 lx
    H= 50%

  SECTION2:
   
    T=30ºC
    L=300 lx
    H= 50%

*/

/*Matriz 4x4 para 2 motes com 3 sensores cada um + 6 atuadores (Estores1, Ar condicionado, Humificador1, Estores2, Forno, Humidificador2)
 
	Luz1			Ar_cond 		Humidade2 		NOT_USED
	
	Temperatura1	Humidificador1	Lampada2		NOT_USED
	
	Humidade1		Luz2			Forno			NOT_USED
	 
	Lampada1		Temperatura2	Humidificador2	NOT_USED
 
 */
void removeSpaces(char *str) { // Function to remove all spaces from a given string
	
    int count = 0; // To keep track of non-space character count
  
    
    for (int i = 0; str[i]; i++) // Traverse the given string. If current character is not space, then place it at index 'count++' 
    
        if (str[i] != ' ')
        
            str[count++] = str[i]; // here count is incremented
            
    str[count] = '\0'; 
    
}

void delchar(char *x,int a, int b) { //Delete n characters from string x, starting at character number b
	
	if ((a+b-1) <= strlen(x)) {
		
		strcpy(&x[b-1],&x[a+b-1]);
		
	}
	
}

typedef struct {
	
	float visible_light;
	
	float temperature;
	
	float humidity;
	
} mote;

int main(int argc, char **argv)
{
	
	char string[100], stringaux[100], msg[1000], msg2[1000], prince[100], king[100];
	
	int f = 0, flag[6] = {0}; //Vetor flag: toma valor 1 na posição 0 se for para ligar a lampada1, 0 para desligar
						  //			toma valor 1 na posição 1 se for para ligar o ar condicionado, 0 para desligar
						  //			toma valor 1 na posição 2 se for para ligar o humidificador1, 0 para desligar
						  //			toma valor 1 na posição 3 se for para ligar a lampada2, 0 para desligar
						  //			toma valor 1 na posição 4 se for para ligar o forno, 0 para desligar
						  //			toma valor 1 na posição 5 se for para ligar o humidificador2, 0 para desligar
	float aux;
	
	mote sensors[2];
	
	FILE *fp;
	
	FILE *fq;
	
	FILE *fr;
	
	while(1) {
	
		fgets(string, sizeof(string), stdin);
		
		string[strcspn(string, "\n")] = '\0';
		
		removeSpaces(string);
		
		
		if(memcmp(string, "7E45", 4) == 0) {
		
			delchar(string, 10, 0);
			
			strncpy(stringaux, string, 4);
			
			f = strtol(stringaux,NULL,16)-1; //Convert Mote ID to long int
			
			delchar(string, 14, 0);
			
			strncpy(stringaux, string, 4);
			
			sensors[f].visible_light = strtol(stringaux,NULL,16); //Convert Raw Visible Light to long int
			
			sensors[f].visible_light = 0.625*1000000*(sensors[f].visible_light/4096)*1.5/100000*1000;
			
			delchar(string, 8, 0);
			
			strncpy(stringaux, string, 4);
			
			sensors[f].temperature = strtol(stringaux,NULL,16); //Convert Raw Temperature to long int
			
			sensors[f].temperature = -39.6 + 0.01 * sensors[f].temperature;
			
			delchar(string, 4, 0);
			
			strncpy(stringaux, string, 4);
			
			sensors[f].humidity = strtol(stringaux,NULL,16); //Convert Raw Humidity to long int
			
			aux=sensors[f].humidity;
			
			sensors[f].humidity = -2.0468 + 0.0367 * sensors[f].humidity - 1.5955 *pow(10,-6) * sensors[f].humidity * sensors[f].humidity;
			
			sensors[f].humidity = (sensors[f].temperature - 25) *(0.01 + 0.00008 * aux) + sensors[f].humidity;
			
			printf("\n\nLeituras do mote %d:\nLuz Visível: %f lx\nTemperatura: %f ºC\nHumidade Relativa: %f %%\n", f+1, sensors[f].visible_light, sensors[f].temperature, sensors[f].humidity);
			
			sprintf(msg,"-n 1 -l 200 -f 1 -c 1 -s [1,3,4] -d [[");
			
			fp = fopen("/tmp/ttyV12", "w+");
			
			if(sensors[f].visible_light < 200.0 && f == 0) { //Se estiver escuro, a luz é ligada
				
				flag[0] = 1;
			
				fprintf(fp, "%s,","["YELLOW);
				sprintf(prince,"'U',300.0,300.0,1.0],[");
				strcat(msg,prince);
			}
			
			else {
				
				flag[0] = 0;
				
				fprintf(fp, "%s,","["BLACK);
				sprintf(prince,"'U',300.0,300.0,1.0],[");
				strcat(msg,prince);
				
				
			}	
			
			if(sensors[f].temperature > 25.0 && f == 0) {
			
				flag[1] = 1;
			
				fprintf(fp, "%s,",ORANGE);
				sprintf(prince,"'L',24.5,25.5,-1],[");
				strcat(msg,prince);
			}
			
			else {
				
				flag[1] = 0;
				
				fprintf(fp, "%s,",BLACK);
				sprintf(prince,"'L',24.5,25.5,1],[");
				strcat(msg,prince);
			}
			
			if(sensors[f].humidity < 50.0 && f == 0) {
			
				flag[2] = 1;
			
				fprintf(fp, "%s,",BLUE);
				sprintf(prince,"'L',49.5,50.5,1]] -i 1");
				strcat(msg,prince);
			}
			
			else {
				
				flag[2] = 0;
				
				fprintf(fp, "%s,",BLACK);
				sprintf(prince,"'L',49.5,50.5,-1]] -i 1");
				strcat(msg,prince);
			}
			
			
			
			sprintf(msg2,"-n 1 -l 200 -f 1 -c 1 -s [1,3,4] -d [[");
			
			
			if(flag[0]) {
			
				fprintf(fp, "%s,",WHITE);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			if(flag[1]) {
			
				fprintf(fp, "%s,",GREY);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			if(flag[2]) {
			
				fprintf(fp, "%s,",GREEN);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			if(sensors[f].visible_light < 200.0 && f == 1) {
			
				flag[3] = 1;
			
				fprintf(fp, "%s,",YELLOW);
				sprintf(king,"'U',300.0,300.0,1.0],[");
				strcat(msg2,king);
				
			}
			
			else {
				
				flag[3] = 0;
			
				fprintf(fp, "%s,",BLACK);
				sprintf(king,"'U',300.0,300.0,1.0],[");
				strcat(msg2,king);
				
				
			}
			
			if(sensors[f].temperature < 30.0 && f == 1) {
			
				flag[4] = 1;
			
				fprintf(fp, "%s,",ORANGE);
				sprintf(king,"'L',29.5,30.5,1],[");
				strcat(msg2,king);
			}
			
			else {
				
				flag[4] = 0;
			
				fprintf(fp, "%s,",BLACK);
				sprintf(king,"'L',29.5,30.5,-1],[");
				strcat(msg2,king);
			}
			
			if(sensors[f].humidity < 50.0 && f == 1) {
			
				flag[5] = 1;
			
				fprintf(fp, "%s,",BLUE);
				sprintf(king,"'L',49.5,50.5,1]] -i 2");
				strcat(msg2,king);
			}
			
			else {
				
				flag[5] = 0;
			
				fprintf(fp, "%s,",BLACK);
				
				sprintf(king,"'L',49.5,50.5,-1]] -i 2");
				
				strcat(msg2,king);
				
			}
			
			
			if(flag[3]) {
			
				fprintf(fp, "%s,",WHITE);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			if(flag[4]) {
			
				fprintf(fp, "%s,",RED);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			if(flag[5]) {
			
				fprintf(fp, "%s,",GREEN);
				
			}
			
			else {
				
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			for(int i = 0; i < 3; i++) {
				
				fprintf(fp, "%s,",BLACK);
				
			}
			
			fprintf(fp, "%s",BLACK"]\n");
			
			fclose(fp);
			
			sleep(1); //Acompanhar a frequência do msg creator
			
			if(strcmp(msg, "") != 0) {
				
				fq = fopen("MsgCreatorConf.txt", "w+");
				
				fprintf(fq, "%s",msg);
				
				fclose(fq);
				
			}
			
			if(strcmp(msg2, "") != 0) {
				
				fr = fopen("MsgCreatorConf2.txt", "w+");
				
				fprintf(fr, "%s",msg2);
				
				fclose(fr);
				
			}	
			
		}
		
	}	
	
	return 0;
}
