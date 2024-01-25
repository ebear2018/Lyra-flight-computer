#if !defined(MPCOREHEADER)
#define MPCOREHEADER

#include <navcore.h>
#include <pyrobatt.h>
#include <RPi_Pico_TimerInterrupt.h>

//fs::File logtofile;

NAVCORE NAV;
SERIALPORT port;
RADIO telemetryradio;

PYROCHANNEL P1(1);
PYROCHANNEL P2(2);
PYROCHANNEL P3(3);
PYROCHANNEL P4(4);

RPI_PICO_Timer PyroTimer0(0);

bool checkfirepyros(struct repeating_timer *t)
{ 
  (void) t;

    P1.checkfire();
    P2.checkfire();
    P3.checkfire();
    P4.checkfire();
    //Serial.println("checking pyros");

  return true;
}





class MPCORE{
    

    public:
        

        mpstate _sysstate;
        int detectiontime = 0;
        uint32_t landedtime = 0;
        bool datamoved = false;
        uint32_t landingdetectiontime = 0;
        uint32_t liftofftime = 0;
        uint32_t missionelasped = 0;
        uint32_t P1firedtime = 0;
        uint32_t P2firedtime = 0;
        



        MPCORE();

        //int32_t _sysstate.errorflag = 1;
        /*
            1 = no error
            3 = handshake fail
            5 = serial init failure
            7 = sd init fail
            11 = flash init fail
            13 = no packet recived
            17 = bad telemetry packet
            19 = radio init fail
            23 = bad telemetry packet
        */
        bool sendserialon = false;
        bool sendtoteleplot = true;

        int ledcolor;

        int freqs[7] = {3000,5000,5000,5000,5000,5000,8000};


        struct timings{
            uint32_t logdata;
            uint32_t led;
            uint32_t serial;
            uint32_t sendtelemetry;
            uint32_t beep;
            uint32_t loop;
        };
        timings intervals[7] = {
            {2000,1000,50,1000,10000}, // ground idle
            {10,200,100, 200,800}, // launch detect // DEPRECATED
            {10,500,100, 200,800}, // powered ascent
            {10,500,100,200,800}, // unpowered ascent
            {10,500,100,200,800}, // ballistic descent
            {10,800,100,200,800}, //ready to land
            {1000,1500,100,200,500} // landed
        };
        timings prevtime;
        bool ledstate = false;



        void setuppins();
        int initperipherials();

        void beep();
        void beep(int freq);
        void beep(int freq, unsigned int duration);

        void setled(int color);

        int logdata();
        int erasedata();
        int flashinit();
        int dumpdata();

        int changestate();
        int parsecommand(char input);
        int sendtelemetry();
        int checkforpyros();
        

};

MPCORE::MPCORE(){
    _sysstate.r.state = 0;
    _sysstate.r.errorflag = 1;
    _sysstate.r.pyrosfired = 0;
};

void MPCORE::setuppins(){
    pinMode(LEDRED,OUTPUT);
    pinMode(LEDGREEN,OUTPUT);
    pinMode(LEDBLUE,OUTPUT);
    pinMode(BUZZERPIN,OUTPUT);


    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDBLUE, HIGH);

    // adc.setuppins();
    return;
}


void MPCORE::beep(){
    tone(BUZZERPIN,2000,200);
    return;
}

void MPCORE::beep(int freq){
    tone(BUZZERPIN,freq,200);
    return;
}

void MPCORE::beep(int freq, unsigned int duration){
    tone(BUZZERPIN,freq,duration);
return;
}


void MPCORE::setled(int color){
    switch (color)
    {
    case OFF:
        digitalWrite(LEDRED, HIGH);
        digitalWrite(LEDGREEN, HIGH);
        digitalWrite(LEDBLUE, HIGH);
        break;

    case RED:
        digitalWrite(LEDRED, LOW);
        digitalWrite(LEDGREEN, HIGH);
        digitalWrite(LEDBLUE, HIGH);
        break;
    
    case GREEN:
        digitalWrite(LEDRED, HIGH);
        digitalWrite(LEDGREEN, LOW);
        digitalWrite(LEDBLUE, HIGH);
        break;

    case BLUE:
        digitalWrite(LEDRED, HIGH);
        digitalWrite(LEDGREEN, HIGH);
        digitalWrite(LEDBLUE, LOW);
        break;
    
    default:
        break;
    }
}


int MPCORE::initperipherials(){
    port.init();
    int error = telemetryradio.init();
    // adc.setuppins();

    if (PyroTimer0.attachInterruptInterval(100 * 1000, checkfirepyros))
    {
      Serial.print(F("Starting ITimer0 OK, millis() = ")); Serial.println(millis());
    }
    else
        Serial.println(F("Can't set ITimer0. Select another freq. or timer"));

    
    return 0;
}



int MPCORE::logdata(){
    uint32_t openingtime = micros();
    // adc.readbatt();
    // _sysstate.r.batterystate = adc.battvoltage;

    logpacket datatolog = preplogentry(_sysstate,NAV._sysstate);
    //Serial.print(datatolog.r.checksum2);

    fs::File logfile = LittleFS.open("/log.csv", "a+");
    //Serial.printf("opening file took %d \n",micros()-openingtime);
    openingtime = micros();
    if (!logfile){
        return 1;
        _sysstate.r.errorflag % 11 == 0 ? _sysstate.r.errorflag = _sysstate.r.errorflag : _sysstate.r.errorflag *= 11;
    };
    //Serial.printf("checking file took %d \n",micros()-openingtime);
    openingtime = micros();
    int j = 0;
    for (int i = 0; i < sizeof(logpacket); i++)
    {
        logfile.write(datatolog.data[j]);
        j++;
    }

    
    openingtime = micros();
    logfile.close();
    //Serial.printf("closing file took %d \n\n",micros()-openingtime);
    return 0;
}

int MPCORE::erasedata(){
    int error = LittleFS.remove("/log.csv");
    if (error != 1)
    {
        Serial.println("file erase fail");
        return 1;
    }
    Serial.println("file erase success");
    return 0;
    
}

int MPCORE::dumpdata(){
    Serial.println("dumping data to serial");
    Serial.println("index, checksum,uptime mp,uptime nav,  errorflag mp,errorflag NAV,  accel x, accel y, accel z, accelworld x, accelworld y, accelworld z, accelhighg x, accelhighg y, accelhighg z, gyro x, gyro y, gyro z, euler x, euler y, euler z, quat w, quat x, quat y, quat z, altitude, presusre, verticalvel,filtered vvel, maxalt, altitudeagl, filtered alt, imutemp, barotemp,state,battstate,pyros fired checksum2");
    
    fs::File readfile = LittleFS.open("/log.csv", "r");
    uint32_t entrynum = 0;

    while (readfile.available() > 0)
    {
        logpacket readentry;
        uint8_t buf[sizeof(logpacket)];
        readfile.read(buf,sizeof(logpacket));
        int j = 0;
        for (int i = 0; i < sizeof(logpacket); i++)
        {
            readentry.data[j] = buf[j];
            j++;
        }
        if (readentry.r.checksum1 != 0xAB || readentry.r.checksum2 != 0xCD)
        {
            uint32_t starttime = millis();
            while (millis() - starttime < 5000 && readfile.available() > 0)
            {   
                int thisbyte = readfile.read();
                if (thisbyte == 0xAB)
                {
                    //Serial.printf("found start of next entry at pos %d\n",readfile.position());
                    readfile.seek(readfile.position() - 1);
                    break;
                }
                //Serial.printf("waiting for start of next entry, exp 0xAB got %x \n", thisbyte);
                
            }
            
        }

        Serial.printf(
        "%d, 101,"// index checksum,
        "%d,%d,"//uptimes
        "%d,%d,"//errorflag
        "%f,%f,%f," // accel
        "%f,%f,%f," // accel world
        "%f,%f,%f," // high g accel
        "%f,%f,%f," // gyro
        "%f,%f,%f," // orientation euler"
        "%f,%f,%f,%f," // orientation quat"
        "%f,%f," //altitude, presusre
        "%f,%f," //verticalvel,filtered vvel,
        "%f,%f,%f," // max alt, altitudeagl, filtered alt
        "%f,%f," // temps, imu baro
        "%d,%f,%d, 202\n", //state, battstate, pyros
        entrynum,
        readentry.r.MPstate.r.uptime, 
        readentry.r.navsysstate.r.uptime,

        readentry.r.MPstate.r.errorflag, 
        readentry.r.navsysstate.r.errorflag,

        readentry.r.navsysstate.r.imudata.accel.x, 
        readentry.r.navsysstate.r.imudata.accel.y, 
        readentry.r.navsysstate.r.imudata.accel.z,

        readentry.r.navsysstate.r.accelworld.x, 
        readentry.r.navsysstate.r.accelworld.y, 
        readentry.r.navsysstate.r.accelworld.z,

        readentry.r.navsysstate.r.adxldata.accel.x, 
        readentry.r.navsysstate.r.adxldata.accel.y, 
        readentry.r.navsysstate.r.adxldata.accel.z,

        readentry.r.navsysstate.r.imudata.gyro.x*(180/M_PI),
        readentry.r.navsysstate.r.imudata.gyro.y*(180/M_PI),
        readentry.r.navsysstate.r.imudata.gyro.z*(180/M_PI),

        readentry.r.navsysstate.r.orientationeuler.x*(180/M_PI), 
        readentry.r.navsysstate.r.orientationeuler.y*(180/M_PI), 
        readentry.r.navsysstate.r.orientationeuler.z*(180/M_PI),

        readentry.r.navsysstate.r.orientationquat.w, 
        readentry.r.navsysstate.r.orientationquat.x,
        readentry.r.navsysstate.r.orientationquat.y, 
        readentry.r.navsysstate.r.orientationquat.z,

        readentry.r.navsysstate.r.barodata.altitude, 
        readentry.r.navsysstate.r.barodata.pressure, 

        readentry.r.navsysstate.r.barodata.verticalvel, 
        readentry.r.navsysstate.r.filtered.vvel, 

        readentry.r.navsysstate.r.barodata.maxrecordedalt, 
        readentry.r.navsysstate.r.barodata.altitudeagl,
        readentry.r.navsysstate.r.filtered.alt,

        readentry.r.navsysstate.r.imudata.temp,
        readentry.r.navsysstate.r.barodata.temp,

        readentry.r.MPstate.r.state,
        readentry.r.MPstate.r.batterystate,
        readentry.r.MPstate.r.pyrosfired
        );
        entrynum++;
    }

    //Serial.println("done");
    return 0;
}

int MPCORE::flashinit(){
        //Serial.println("flash init start");

        int error = LittleFS.begin();

        if (error = 0)
        {
            Serial.printf("filesystem mount fail %d\n",error);
            _sysstate.r.errorflag *= 11;
            rp2040.resumeOtherCore();
            return 1;
        }


        FSInfo64 *info;
        error = LittleFS.info64(*info);

        if (error != 1)
        {
            Serial.printf("filesystem info fail %d\n", error);
            _sysstate.r.errorflag *= 11;
            rp2040.resumeOtherCore();
            return 1;
        }
        

        uint32_t total = info->totalBytes;
        uint32_t used = info->usedBytes;
        uint32_t avail = total - used;

        Serial.printf("FS info: total %d, used %d, avail %d\n",total,used,avail);

        LittleFS.remove("/f.txt");

        fs::File testfile = LittleFS.open("/f.txt","w+");

        if (!testfile)
        {
            Serial.println("file open failed");
            _sysstate.r.errorflag *= 11;
            rp2040.resumeOtherCore();
            return 2;
        }

        //Serial.println("file opened");
        int testnum = 1;

        testfile.print(testnum);
        //Serial.print("file written");
        testfile.close();
        testfile = LittleFS.open("/f.txt","r");

        int readnum = testfile.read() - 48;

        //Serial.println("file read");

        if (readnum != testnum)
        {
            Serial.printf("read fail, expected %d, got %d\n",testnum,readnum);
            _sysstate.r.errorflag *= 11;
            rp2040.resumeOtherCore();
            return 3;
        }
        
        //Serial.printf("read success, expected %d, got %d\n",testnum,readnum);

        
        testfile.close();
        //Serial.println("flash init complete");
        rp2040.resumeOtherCore();
        return 0;
}




int MPCORE::changestate(){

    Vector3d accelvec = vectorfloatto3(NAV._sysstate.r.imudata.accel);
    Vector3d gyrovec  = vectorfloatto3(NAV._sysstate.r.imudata.gyro);
    float accelmag = accelvec.norm();
    if (_sysstate.r.state == 0) // detect liftoff
    {
        
        
        NAV._sysstate.r.filtered.alt > 5 && NAV._sysstate.r.filtered.vvel > 5 ? detectiontime = detectiontime : detectiontime = millis();
        if (millis() - detectiontime >= 400)
        {
            _sysstate.r.state = 2;
            detectiontime = millis();
            ebyte.setPower(Power_27,true);
            Serial.println("liftoff");
        }
        
    }

    else if (_sysstate.r.state == 2) // detect burnout
    {
        
        
        accelvec.z() < 8 ? detectiontime = detectiontime : detectiontime = millis();
        if (millis() - detectiontime >= 200)
        {
            _sysstate.r.state = 3;
            detectiontime = millis();
            Serial.println("burnout");
        }
    }

    else if (_sysstate.r.state == 3) // detect appogee
    {
        NAV._sysstate.r.filtered.alt < NAV._sysstate.r.filtered.maxalt*0.95 ?  detectiontime = detectiontime : detectiontime = millis();

        if (millis() - detectiontime >= 100)
        {
            _sysstate.r.state = 4;
            detectiontime = millis();
            Serial.println("appogee");
        }
    }

    else if (_sysstate.r.state == 4) // detect chute opening
    {
        

        accelmag > 7 ? detectiontime = detectiontime : detectiontime = millis();
        if (millis() - detectiontime >= 300)
        {
            _sysstate.r.state = 5;
            detectiontime = millis();
            Serial.println("chutes out");
        }
    }

    else if (_sysstate.r.state == 5) // detect landing
    {   

        if (abs(NAV._sysstate.r.barodata.verticalvel) < 0.3 && accelvec.norm() < 20 &&  accelvec.norm() > 5  && gyrovec.norm() < 0.5)
        {
                detectiontime = detectiontime;
        }
        else{
            detectiontime = millis();
        }

        if (millis() - detectiontime >= 3000)
        {
            _sysstate.r.state = 6;
            detectiontime = millis();
            landedtime = millis();
            Serial.println("landed");
        }
    }

    else if (_sysstate.r.state == 6) // reset
    {   

        if (abs(NAV._sysstate.r.filtered.vvel) < 3 && accelvec.norm() < 20 &&  accelvec.norm() > 5  && gyrovec.norm() < 0.5)
        {
                detectiontime = detectiontime;
        }
        else{
            detectiontime = millis();
        }

        if (millis() - detectiontime >= 20000)
        {
            _sysstate.r.state = 0;
            detectiontime = millis();
            landedtime = 0;
            Serial.println("reseting");
        }
    }
    

    if (_sysstate.r.state > 1 && _sysstate.r.state != 6 && abs(NAV._sysstate.r.barodata.verticalvel) < 0.3 && accelvec.norm() < 20 &&  accelvec.norm() > 5  && gyrovec.norm() < 0.5)
    {
        landingdetectiontime = landingdetectiontime;
    }
    else{
        landingdetectiontime = millis();
    }
    if (millis() - landingdetectiontime >= 30000)
        {
            _sysstate.r.state = 6;
            detectiontime = millis();
            landedtime = millis();
            Serial.println("randoland");
    }

    return 0;
}



// 0 = ground idle 1 = deprecated 2 = powered ascent 3 = unpowered ascent 4 = ballisitic decsent 5 = under chute 6 = landed
int MPCORE::checkforpyros(){

    if (NAV._sysstate.r.filtered.alt < NAV._sysstate.r.filtered.maxalt && _sysstate.r.state >= 4)
    {
        _sysstate.r.pyrosfired =  _sysstate.r.pyrosfired || 00000001;
    }

    if (NAV._sysstate.r.filtered.alt < 400 && _sysstate.r.state >= 4)
    {
        _sysstate.r.pyrosfired = _sysstate.r.pyrosfired || 00000010;
    }

    P1.checkfire();
    P2.checkfire();
    P3.checkfire();
    P4.checkfire();

    uint8_t pyrocont = 0;
    pyrocont = pyrocont || P1.getcont();
    pyrocont = pyrocont || P2.getcont();
    pyrocont = pyrocont || P3.getcont();
    pyrocont = pyrocont || P4.getcont();
    _sysstate.r.pyroscont = pyrocont;
    return 0;
}


int MPCORE::parsecommand(char input){
    if (int(input) == 0)
    {
        return 1;
    }
    Serial.println(int(input));
    
    char channel;

    if (input == 'l' && _sysstate.r.state < 2){
        Serial.println("put into launch mode");
        _sysstate.r.state = 2;
        detectiontime = millis();
        return 0;
    }

    else if (input == 'a' && (_sysstate.r.state < 4 || _sysstate.r.state >= 6 )){
        _sysstate.r.state = 0;
        ebyte.setPower(Power_21,true);
        return 0;
    }

    switch (input)
    {
    case 's':
        Serial.println("printing data to teleplot");
        sendserialon = !sendserialon;
        port.sendtoplot = true;
        break;

    case 'w':
        Serial.println("printing data to processing");
        sendserialon = !sendserialon;
        port.sendtoplot = false;
        break;
    
    case 'X':
        Serial.println("getting new offsets for adxl");
        adxl.getnewoffsets();
        break;

    case 'D':

        dumpdata();
        break;
    
    case 'o':
        NAV.getpadoffset();
        break;
    
    case 'P':
        if (Serial.read() != 'A')
        {
            return 0;
        }
        channel = Serial.read();
        Serial.print("firing pyro ");
        Serial.println(channel);
        switch (channel)
        {
        case '1':

            P1.fire();
            break;
        
        case '2':
            P2.fire();
            break;
        
        case '3':
            P3.fire();
            break;
        
        case '4':
            P4.fire();
            break;
        
        default:
            break;
        }
        break;
    
    case 'e':
        Serial.println("erasing flash?");
        if (Serial.read() == 'e')
        {
            Serial.println("erasing flash");
            uint32_t erasestarttime = millis();
            uint32_t messagetime = millis();
            while (millis()-erasestarttime < 10000)
            {
                if (millis()- messagetime > 500)
                {
                    Serial.println("ERASING FLASH ------ ERASING FLASH ------ ERASING FLASH ------ TO ABORT ERASION DISCONNECT POWER OR TYPE A CHARACTER");
                }
                if (Serial.available())
                {
                    return 1;
                }
                
            }
            
            erasedata();
        }
        
        break;

    
    default:
        break;
    }
    
    
    return 0;
}

int MPCORE::sendtelemetry(){
    telepacket packettosend;
    uint8_t databufs[32];

    packettosend = statetopacket(_sysstate,NAV._sysstate);
    telemetryradio.sendpacket(packettosend);

    
    return 0;
    
}



#endif // MPCOREHEADER


