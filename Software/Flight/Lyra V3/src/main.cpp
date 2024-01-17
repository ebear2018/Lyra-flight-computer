#include <Arduino.h>

#include "mpcore.h"

MPCORE MP;

bool dataismoved = false;


void setup() { // main core setup
    MP.setuppins();
    MP.beep();
    MP.setled(BLUE);
    MP.initperipherials();

    
    MP.flashinit();
    MP.logdata();
    


    if (NAV._sysstate.r.errorflag * MP._sysstate.r.errorflag != 1)
    {
        MP.ledcolor = BLUE;
        Serial.println("core error");
    }
    else if (NAV._sysstate.r.errorflag * MP._sysstate.r.errorflag < 0){
        MP.ledcolor = RED;
    }
    else{
        MP.ledcolor = GREEN;
        Serial.println("cores good");
    }
    

    MP._sysstate.r.uptime = millis();


    Serial.print("MP boot complete error code: ");
    Serial.println(MP._sysstate.r.errorflag);
    
    Serial.print("NAV boot complete, error code :");
    Serial.println(NAV._sysstate.r.errorflag);

    MP.beep(10000,200);
    Serial.println("mpcore out of setup");
}

void setup1() { // nav core setup
    // NAV.handshake();
    delay(6000);
    NAV.initi2c();
    NAV.sensorinit();
    NAV.getpadoffset();
    NAV.KFinit();
    NAV.getsensordata();
}

void loop() { // main core loop
    int eventsfired = 0;
    MP.changestate();

    if (millis() - MP.prevtime.logdata >= MP.intervals[MP._sysstate.r.state].logdata)
    {
        if (MP.sendserialon && MP.sendtoteleplot)
        {
            Serial.print(">shouldlog: 1 \n");
        }
    }

    if (millis() - MP.prevtime.logdata >= MP.intervals[MP._sysstate.r.state].logdata)
    {
        //uint32_t prevlogmicros = micros();
        MP.logdata();
        if (MP.sendserialon && MP.sendtoteleplot)
        {
            Serial.printf(">lograte: %f \n",1000/float((millis()-MP.prevtime.logdata)));
        }
        MP.prevtime.logdata = millis();
        eventsfired += 2;
        //Serial.printf("logging  took: %d \n",micros() - prevlogmicros);
    }

    if (MP.sendserialon & millis() - MP.prevtime.serial >= MP.intervals[MP._sysstate.r.state].serial)
    {
        port.senddata(MP._sysstate,NAV._sysstate);
        MP.prevtime.serial = millis();
        //eventsfired += 20;
    }
    


    if (millis()- MP.prevtime.led >= MP.intervals[MP._sysstate.r.state].led)
    {
        MP.ledstate ? MP.setled(MP.ledcolor) : MP.setled(OFF);
        MP.ledstate =! MP.ledstate;
        MP.prevtime.led = millis();
        
    }

    if (millis()- MP.prevtime.beep >= MP.intervals[MP._sysstate.r.state].beep)
    {
        MP.beep(MP._sysstate.r.state,false);
        MP.prevtime.beep = millis();
        eventsfired += 10;
    }
    
    
    if (Serial.available())
    {
        char buf = Serial.read(); 
        int i;
        Serial.printf("echo: %c dec: %d \n",buf,buf);
        MP.parsecommand(buf);

    }

    
    if ((millis() - MP.prevtime.sendtelemetry >= MP.intervals[MP._sysstate.r.state].sendtelemetry) && MP._sysstate.r.errorflag %19 != 0)
    {
        uint32_t prevtelemmicros = micros();
        MP.sendtelemetry();
        MP.prevtime.sendtelemetry = millis();
        eventsfired += 4;
        //Serial.printf("telemetry sending took: %d \n",micros() - prevtelemmicros);
    }

    if (Serial1.available())
    {
        int buf = Serial1.read();
        Serial.printf("recived %x from radio",buf);
        MP.parsecommand(buf);
        
    }
    
    // MP._sysstate.r.uptime = millis();
    // if (MP.sendserialon && MP.sendtoteleplot)
    // {
    //     Serial.printf(">looptime: %f \n", 1/(float(micros() - MP.prevtime.loop)/1e6));
    //     Serial.printf(">eventstranspired: %d \n", eventsfired);
    //     Serial.print(">shouldlog: 0 \n");
    // }
    
    MP.prevtime.loop = micros();
    MP._sysstate.r.state >= 1 ? MP.missionelasped = millis() - MP.liftofftime : MP.missionelasped = 0, MP.landedtime = millis();

    // if (Serial1.available())
    // {
    //     Serial.println("newradiomessage");
    //     while (Serial1.available() > 0)
    //     {
    //         uint8_t readbuf = Serial1.read();
    //         Serial.printf("0x%x, ",readbuf);
    //         printBin(readbuf);
    //         Serial.printf(", %d\n",readbuf);
    //     }
        
    // }
    
}


void loop1() { // nav core loop
    MP._sysstate.r.state = 0 ? NAV.useaccel = true : NAV.useaccel = false;
    
    NAV.prevtime.getdata = micros();
    NAV.getsensordata();
    if (MP.sendserialon && MP.sendtoteleplot)
    {
            Serial.printf(">sensordatatime: %f \n",float(micros()-NAV.prevtime.getdata)/1000);
            NAV.prevtime.predictkf = micros();
    }
    NAV.KFpredict();
    //Serial.printf(">kfpredicttime: %f \n",float(micros()-NAV.prevtime.predictkf)/1000);

    if (millis() - NAV.prevtime.kfupdate >= 100)
    {
        NAV.prevtime.updatekf = micros();
        NAV.KFupdate();
        NAV.prevtime.kfupdate = millis();
        //Serial.printf(">kfupdatetime: %f \n",float(micros()-NAV.prevtime.updatekf)/1000);
    }
    
    if (MP.sendserialon && MP.sendtoteleplot)
    {
        Serial.printf(">navlooprate: %f \n", 1/(float(micros() - NAV.prevtime.looptime)/1e6));
        NAV.prevtime.looptime = micros();
    }
    
    NAV._sysstate.r.uptime = millis();
}