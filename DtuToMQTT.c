#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <time.h> //time()
#include "MQTTClient.h"

// Ustawienia MQTT
#define ADDRESS         "tcp://192.168.20.121:1883"
#define CLIENTID        "UbuntuClientMQTT"
#define VOLTAGE_CONFIG_TOPIC "homeassistant/sensor/voltage_sensor/config"
#define VOLTAGE_STATE_TOPIC  "homeassistant/sensor/voltage_sensor/state"
#define POWER_CONFIG_TOPIC "homeassistant/sensor/power_sensor/config"
#define POWER_STATE_TOPIC  "homeassistant/sensor/power_sensor/state"
#define CURRENT_CONFIG_TOPIC "homeassistant/sensor/current_sensor/config"
#define CURRENT_STATE_TOPIC  "homeassistant/sensor/current_sensor/state"
#define POWERFACTOR_CONFIG_TOPIC "homeassistant/sensor/power_factor/config"
#define POWERFACTOR_STATE_TOPIC  "homeassistant/sensor/power_factor/state"
#define GRIDEXPORT_CONFIG_TOPIC "homeassistant/sensor/grid_export/config"
#define GRIDEXPORT_STATE_TOPIC  "homeassistant/sensor/grid_export/state"
#define GRIDIMPORT_CONFIG_TOPIC "homeassistant/sensor/grid_import/config"
#define GRIDIMPORT_STATE_TOPIC  "homeassistant/sensor/grid_import/state"

#define QOS             1
#define TIMEOUT         10000L


#define SA struct sockaddr
#define PORT 502
#define MAX 240		//Maksymalny rozmiar buforat

#define TIME_LOOP 15 //frequency measurement

short int transaction_id=0;
 

int get_modbus_data(int sockfd,int id,unsigned char buff[],unsigned short int address,unsigned short int numbers_of_registers)
{
//    unsigned char buff[MAX];
    unsigned char sendbuff[260];
    short int i,b;
//    b = 30000;
//    printf("adres : %i \n",address);

    sendbuff[0]=(unsigned char)(transaction_id>>8);  //transaction id
    sendbuff[1]=(unsigned char)(transaction_id);
    transaction_id++;
    sendbuff[2]=0x00;  //protocol id
    sendbuff[3]=0x00;
    sendbuff[4]=0x00; //length
    sendbuff[5]=0x06;
    sendbuff[6]=id; //id
    sendbuff[7]=0x03; //function code
    sendbuff[8]=(unsigned char)(address>>8); //start adress
    sendbuff[9]=(unsigned char)address;
    sendbuff[10]=(unsigned char)(numbers_of_registers>>8); //ilosc rejestrow
    sendbuff[11]=(unsigned char) numbers_of_registers;
//    for (i=0;i<12;i++) printf (" %2X",sendbuff[i]);
    write(sockfd,sendbuff,12);
//    write(sockfd,send buff,12sizeof(sendbuff));
//    printf("\nCzekam na dane\n:");
    b=read(sockfd, buff, MAX);

//    printf("Bajtow : %i %i\n",sizeof(*buff),b);
//    for (i=0;i<b;i++) printf (" %2X",buff[i]);
//    printf("\nEND OF PROC \n");

    return (b);
}




int main()
{
    int sockfd, b;
    int chint_addres = 0x6000;
    short int j,i,k=0;
    int *ptr; 
    struct sockaddr_in servaddr;
    time_t t;
    struct tm * timeinfo;

    unsigned char buff_pv[MAX];
    unsigned char buffer[MAX];

    struct timeval tv;
    int stat;
	    char string_value[14]; 

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;


    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Dodaj dane uwierzytelniające
    conn_opts.username = "mosquitto";
    conn_opts.password = "Haslo123";




	tv.tv_sec = 4;
	tv.tv_usec = 0;
    for (;;)
    {
	sleep(1); 
    // Inicjalizacja klienta MQTT
    MQTTClient_create(&client, ADDRESS, CLIENTID,MQTTCLIENT_PERSISTENCE_NONE, NULL);

        // Połączenie z brokerem MQTT
        if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
	    printf("Nie udało się połączyć z brokerem MQTT, kod błędu: %d\n", rc);
    	    continue; 
	}
	printf("Połączono z brokerem MQTT\n");

        // Wysłanie konfiguracji encji GridVoltage
	const char* voltage_config_payload = 
    	    "{\"name\": \"GridVoltage\", \"state_topic\": \"homeassistant/sensor/voltage_sensor/state\", "
	    "\"unit_of_measurement\": \"V\", \"device_class\": \"voltage\", \"state_class\": \"measurement\" }";

	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)voltage_config_payload;
	pubmsg.payloadlen = (int)strlen(voltage_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, VOLTAGE_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji napięcia, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja napięcia wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}


        // Wysłanie konfiguracji encji PowerGrid
	const char* power_config_payload  = 
    	    "{\"name\": \"PowerGrid\", \"state_topic\": \"homeassistant/sensor/power_sensor/state\", "
	    "\"unit_of_measurement\": \"W\", \"device_class\": \"power\", \"state_class\": \"measurement\" }";

	//MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)power_config_payload;
	pubmsg.payloadlen = (int)strlen(power_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	//MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, POWER_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji mocy, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja mocy wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}


        // Wysłanie konfiguracji encji CurrentGrid
	const char* current_config_payload  = 
    	    "{\"name\": \"GridCurrent\", \"state_topic\": \"homeassistant/sensor/current_sensor/state\", "
	    "\"unit_of_measurement\": \"mA\", \"device_class\": \"current\", \"state_class\": \"measurement\" }";

	//MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)current_config_payload;
	pubmsg.payloadlen = (int)strlen(current_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	//MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, CURRENT_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji prądu, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja prądu wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}


        // Wysłanie konfiguracji encji PowerFactor
	const char* powerfactor_config_payload  = 
    	    "{\"name\": \"PowerFactor\", \"state_topic\": \"homeassistant/sensor/power_factor/state\", "
	    "\"unit_of_measurement\": \" \", \"device_class\": \"power_factor\", \"state_class\": \"measurement\" }";

	//MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)powerfactor_config_payload;
	pubmsg.payloadlen = (int)strlen(powerfactor_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	//MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, POWERFACTOR_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji prądu, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja prądu wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}



        // Wysłanie konfiguracji encji GridExport
	const char* gridexport_config_payload  = 
    	    "{\"name\": \"DTSU_GridExport\", \"state_topic\": \"homeassistant/sensor/grid_export/state\", "
	    "\"unit_of_measurement\": \"kWh \", \"device_class\": \"energy\", \"state_class\": \"total_increasing\" }";

	//MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)gridexport_config_payload;
	pubmsg.payloadlen = (int)strlen(gridexport_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	//MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, GRIDEXPORT_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji exportu, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja exportu wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}


        // Wysłanie konfiguracji encji GridImport
	const char* gridimport_config_payload  = 
    	    "{\"name\": \"DTSU_GridImport\", \"state_topic\": \"homeassistant/sensor/grid_import/state\", "
	    "\"unit_of_measurement\": \"kWh \", \"device_class\": \"energy\", \"state_class\": \"total_increasing\" }";

	//MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = (void*)gridimport_config_payload;
	pubmsg.payloadlen = (int)strlen(gridimport_config_payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 1; // Retained na 1 dla konfiguracji

	//MQTTClient_deliveryToken token;
	rc = MQTTClient_publishMessage(client, GRIDIMPORT_CONFIG_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
    	    printf("Nie udało się wysłać konfiguracji importu, kod błędu: %d\n", rc);
	    continue;
	    
	} else {
    	    printf("Konfiguracja importu wysłana.\n");
    	    MQTTClient_waitForCompletion(client, token, TIMEOUT);
	}



        t=time(NULL);

	for(;;)
	{
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
    	    printf("socket creation failed...\n");
	    continue;
	//    exit(0);
	}
	else
    	    printf("Socket successfully created..\n");
	stat=setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("192.168.20.11");
	servaddr.sin_port = htons(PORT);

        // connect the client socket to server socket
	if (stat=connect(sockfd, (SA*)&servaddr, sizeof(servaddr))!= 0) 
	{
    	    printf("connection with the server failed...\n");
	    continue;
	    //    exit(0);
	}
	else
    	    printf("connected to the server..\n");
	    ptr=buff_pv+9;
	    while (stat==0)
	    {
		if (stat==0)  b=get_modbus_data(sockfd,2,buff_pv,chint_addres,2);
		//	printf("Search\n",(*(buff_pv+9)));
    		if (b<0) break;
		if (*ptr==0x4713c010) 
		{	
		    break;
		}
		else 
		{
		    chint_addres+=0x10;
		    if (chint_addres>=0x7000) {chint_addres = 0x6000;}
		}
	    }
//		printf("Znaleziono\n");
	    if (b<0) break;
	    b=get_modbus_data(sockfd,2,buff_pv,chint_addres+0x20,64);
	    if (b<0) break;
//    		for (i=0;i<b;i++) printf (" %2X",buff_pv[i]);
//    		printf("\nEND OF PROC \n");

	    ptr=buff_pv+0x50+9;
	    printf("Napięcie Ua : %i\n",*ptr);
	    // Wysłanie wartości napięcia
		 sprintf(string_value,"%3d.%1d",*ptr/1000,(*ptr/100 % 10)); // Przykładowe napięcie
	    printf("lan: %s \n",string_value);
	    pubmsg.payload = (void*)string_value;
	    pubmsg.payloadlen = (int)strlen(string_value);
	    pubmsg.retained = 0; // Wartości zwykle nie są retained

	    rc = MQTTClient_publishMessage(client, VOLTAGE_STATE_TOPIC, &pubmsg, &token);
	    if (rc != MQTTCLIENT_SUCCESS) {
        	    printf("Nie udało się wysłać wartości napięcia, kod błędu: %d\n", rc);
		    break;
	    } else {
    		printf("Wartość napięcia wysłana.\n");
    		MQTTClient_waitForCompletion(client, token, TIMEOUT);
	    }
	    short int direction;
	    ptr=buff_pv+0x18+9;
	    printf("Moc P : %i\n",*ptr);
	    if (*ptr<0)
	    {
		direction=+1;
		sprintf(string_value,"%4d.%1d",abs(*ptr/10),abs(*ptr % 10));
	    }
	    else
	    { 
		direction=-1;
		sprintf(string_value,"-%4d.%1d",abs(*ptr/10),abs(*ptr % 10));
	    }
	    // Wysłanie Wartości mocy
	    printf("lan: %s \n",string_value);
	    pubmsg.payload = (void*)string_value;
	    pubmsg.payloadlen = (int)strlen(string_value);
	    pubmsg.retained = 0; // Wartości zwykle nie są retained

	    rc = MQTTClient_publishMessage(client, POWER_STATE_TOPIC, &pubmsg, &token);
	    if (rc != MQTTCLIENT_SUCCESS) {
        	    printf("Nie udało się wysłać wartości mocy chwilowej, kod błędu: %d\n", rc);
		    break;
	    } else {
    		printf("Wartość mocy chwilowej wysłana.\n");
    		MQTTClient_waitForCompletion(client, token, TIMEOUT);
	    }

	    ptr=buff_pv+0x5c+9;
	    printf("Prąd Ia : %i\n",*ptr);
	    // Wysłanie wartości prądu
	    if (direction==-1)
		 sprintf(string_value,"-%d",*ptr);
	    else sprintf(string_value,"%d",*ptr);
	    printf("lan: %s \n",string_value);
	    pubmsg.payload = (void*)string_value;
	    pubmsg.payloadlen = (int)strlen(string_value);
	    pubmsg.retained = 0; // Wartości zwykle nie są retained

	    rc = MQTTClient_publishMessage(client, CURRENT_STATE_TOPIC, &pubmsg, &token);
	    if (rc != MQTTCLIENT_SUCCESS) {
        	    printf("Nie udało się wysłać wartości prądu, kod błędu: %d\n", rc);
		    break;
	    } else {
    		printf("Wartość prądu wysłana.\n");
    		MQTTClient_waitForCompletion(client, token, TIMEOUT);
	    }
	


	    ptr=buff_pv+0x68+9;
	    printf("cos  : %i\n",*ptr);
	    // Wysłanie wartości prądu
	    if (*ptr<0)
		 sprintf(string_value,"-%1d.%3d",*ptr/1000,abs(*ptr % 1000)); //
	    else
		 sprintf(string_value,"%1d.%3d",*ptr/1000,abs(*ptr % 1000)); // 
	    printf("lan: %s \n",string_value);
	    pubmsg.payload = (void*)string_value;
	    pubmsg.payloadlen = (int)strlen(string_value);
	    pubmsg.retained = 0; // Wartości zwykle nie są retained

	    rc = MQTTClient_publishMessage(client, POWERFACTOR_STATE_TOPIC, &pubmsg, &token);
	    if (rc != MQTTCLIENT_SUCCESS) {
        	    printf("Nie udało się wysłać wartości cos, kod błędu: %d\n", rc);
		    break;
	    } else {
    		printf("Wartość cos wysłana.\n");
    		MQTTClient_waitForCompletion(client, token, TIMEOUT);
	    }


	    if (k==0)
	    {
		k=4;
		
		ptr=buff_pv+0x38+9;
		printf("Pimp  : %i\n",*ptr);
		// Wysłanie wartości prądu
		sprintf(string_value,"%1d.%3d",*ptr/1000,abs(*ptr % 1000)); //
	        printf("Pimp: %s \n",string_value);
		pubmsg.payload = (void*)string_value;
		pubmsg.payloadlen = (int)strlen(string_value);
		pubmsg.retained = 0; // Wartości zwykle nie są retained

	    	rc = MQTTClient_publishMessage(client, GRIDIMPORT_STATE_TOPIC, &pubmsg, &token);
	        if (rc != MQTTCLIENT_SUCCESS) {
    		    printf("Nie udało się wysłać wartości importu, kod błędu: %d\n", rc);
		    break;
		} else {
    		    printf("Wartość importu wysłana.\n");
    		    MQTTClient_waitForCompletion(client, token, TIMEOUT);
		}

		ptr=buff_pv+0x28+9;
		printf("Pexp  : %i\n",*ptr);
		sprintf(string_value,"%1d.%3d",*ptr/1000,abs(*ptr % 1000)); //
	        printf("Pexp: %s \n",string_value);
		pubmsg.payload = (void*)string_value;
		pubmsg.payloadlen = (int)strlen(string_value);
		pubmsg.retained = 0; // Wartości zwykle nie są retained

	    	rc = MQTTClient_publishMessage(client, GRIDEXPORT_STATE_TOPIC, &pubmsg, &token);
	        if (rc != MQTTCLIENT_SUCCESS) {
    		    printf("Nie udało się wysłać wartości EXPORTU, kod błędu: %d\n", rc);
		    break;
		} else {
    		    printf("Wartość EXPORTU wysłana.\n");
    		    MQTTClient_waitForCompletion(client, token, TIMEOUT);
		}
	    }



	    k--;

		//time ( &t );`
	    timeinfo = localtime ( &t );



//		printf("r:%f Czas %d",DTSU_AVG.R,time(NULL));
	    fflush(stdout);
	
	    while (t+TIME_LOOP>time(NULL))
	    {
		sleep(1); 
		printf ("Timeloop");
	    };
	    t+=TIME_LOOP;
	close(sockfd);

	}
	printf ("KONIECPETLI");
	// close the socket
        // Rozłączenie klienta
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	printf("Klient MQTT rozłączony.\n");

    }
}



