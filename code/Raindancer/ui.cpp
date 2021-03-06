/*
  Robotic Lawn Mower
  Copyright (c) 2017 by Kai WÃ¼rtz



  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

#include "ui.h"

#include "UseServices.h"
#include "cmd.h"

#include "Blackboard.h"
#include "bCreateTree.h"
#include "bt.h"
#include "adcman.h"
#include "bPerimeterTracking.h"



extern bool _controlManuel;
extern bool _printProcessingData;
extern unsigned long maxLoopTime;
//extern unsigned long lastTimeShowError;
extern TfindTriangle findTriangle;

extern Blackboard myBlackboard;
extern TCreateTree myCreateTree;
extern TlineFollow lineFollow;

extern TErrorHandler errorHandler;

extern Trtc srvRtc;
extern TEEPROM srvEeprom;
extern TDHT srvDht;

extern Tgps srvGps;


extern TmotorSensor srvMotorSensorL;
extern TmotorSensor srvMotorSensorR;

extern void FreeMem(void);

extern TShutdown srvShutdown;
extern TUI srvUI;

bool TUI::Run() {
	PT_BEGIN();

	errorHandler.setInfo(F("Raindancer interface emulator\r\n"));
	errorHandler.setInfo(F("=============================\r\n"));
	errorHandler.setInfo(F("Available debug commands: (lines end with CRLF or '\\r')\r\n"));
	errorHandler.setInfo(F("H will print this help message again\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("hello print hello message\r\n"));
	errorHandler.setInfo(F("args,1,2,3 show args 1 2 3\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== MODE SELECTION ===\r\n"));
	errorHandler.setInfo(F("A        //automatic control over actuators\r\n"));
	errorHandler.setInfo(F("M        //manual control over actuators\r\n"));
	errorHandler.setInfo(F("area,12  //drive 12m at perimeter and begin mowing\r\n"));
	errorHandler.setInfo(F("gohome   //drive to docking station. Call again to deactivate\r\n"));
	errorHandler.setInfo(F("tpt      //test perimeter tracking to dock. Mower stands on perimeter\r\n"));

	errorHandler.setInfo(F("poweroff  //shutdown the sytem\r\n"));


	//errorHandler.setInfo(F("rh,3    //restores 3 drive directions of the history\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== ERROR HANDLING ===\r\n"));
	errorHandler.setInfo(F("error //show errormessage\r\n"));
	errorHandler.setInfo(F("reset //reset error and motor faults\r\n"));
	PT_YIELD();


	errorHandler.setInfo(F("\r\n=== BLUETOOTH ===\r\n"));
	errorHandler.setInfo(F("bt.show //try to detect BT module\r\n"));
	errorHandler.setInfo(F("bt.set  //configure BT module\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== I2C/RTC//EEPROM SERVICE ===\r\n"));
	errorHandler.setInfo(F("i2c.scan          //i2c scanner\r\n"));
	errorHandler.setInfo(F("rtc.show          //show rtc values every rtc read (10sec)\r\n"));
	errorHandler.setInfo(F("rtc.config        //show rtc service config\r\n"));
	errorHandler.setInfo(F("rtc.find          //tries to find RTC and show result\r\n"));
	errorHandler.setInfo(F("rtc.set,8,17,3,25,01,2017 //set rtc time=8:17 dayOfWeek=3 date=25.01.2017\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("eep.config        //show EEPROM service config\r\n"));
	errorHandler.setInfo(F("eep.u8t,10        //show uint8_t at address 10\r\n"));
	errorHandler.setInfo(F("eep.s32t,10       //show int32_t at address 10\r\n"));
	errorHandler.setInfo(F("eep.f,10          //show float at address 10\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("eep.set.u8t,10,7     //write value uint8_t=7 to address=10 \r\n"));
	errorHandler.setInfo(F("eep.set.s32t,10,1234 //write value int32_t=1234 to address=10 \r\n"));
	errorHandler.setInfo(F("eep.set.f,10,7.3     //write value float=7.3 to address=10 \r\n"));
	errorHandler.setInfo(F("eep.erase            //erase the eeprom\r\n"));

	PT_YIELD();


	errorHandler.setInfo(F("\r\n=== DRIVE MOTOR CLOSED LOOP CONTROL SERVICE ===\r\n"));
	errorHandler.setInfo(F("clc.config      //show clcL/R config\r\n"));
	errorHandler.setInfo(F("clc.enc         //show encoder values \r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("clc.scl         //show setpoint, currentspeed left\r\n"));
	errorHandler.setInfo(F("clc.scr         //show setpoint, currentspeed right\r\n"));
	errorHandler.setInfo(F("clc.speedl      //show speed left\r\n"));
	errorHandler.setInfo(F("clc.speedr      //show speed right\r\n"));
	errorHandler.setInfo(F("clc.ser         //show call of enableXXXRamping functions\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("clc.v,30        //drives both motors in closed loop with speed of 30%\r\n"));
	errorHandler.setInfo(F("                //value: -100%% to 100%%\r\n"));
	errorHandler.setInfo(F("clc.s           //stop drive motors\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("clc.p,123.34    //sets drive motors proportional term\r\n"));
	errorHandler.setInfo(F("clc.i,123.34    //sets drive motors integral term\r\n"));
	errorHandler.setInfo(F("clc.k,123.34    //sets drive motors kfr term\r\n"));

	PT_YIELD();
	errorHandler.setInfo(F("clc.ag,3.0,1.0  //sets agility setOutputZeroAtRPm=3.0 stopReachedThresholdAtRpm=1.0 in RPM\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("clc.mt,1,150    //direct motor test. run motor=1 with speed=150\r\n"));
	errorHandler.setInfo(F("                //motor: 1=L, 2=R  speed= -255 to 255\r\n"));
	errorHandler.setInfo(F("                //deactivates closed loop control\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("                //end test with: clc.mt,0,0\r\n"));
	errorHandler.setInfo(F("                //value < 100 will to start the motor\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== MOW MOTOR CLOSED LOOP CONTROL SERVICE ===\r\n"));
	errorHandler.setInfo(F("clcm.config       //show clcM config\r\n"));
	errorHandler.setInfo(F("clcm.speed        //show speed 0-255 \r\n"));
	errorHandler.setInfo(F("clcm.accel,2000   //set ramp factor 2000. The higher the slower the acc.\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("clcm.limit,200    //set speedLimit to 200  Values: 0-255\r\n"));
	errorHandler.setInfo(F("z                 //mow motor start\r\n"));
	errorHandler.setInfo(F("t                 //mow motor stop\r\n"));

	PT_YIELD();
	errorHandler.setInfo(F("\r\n=== POSITION CONTROL SERVICE ===\r\n"));
	errorHandler.setInfo(F("pc.config         //show pcL/R config\r\n"));
	errorHandler.setInfo(F("pc.L              //show result after pos reached\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("pc.R               //show result after pos reached\r\n"));
	errorHandler.setInfo(F("pc.tuneup,2.0,1.85 //stopCmBeforeTarget,addCmToTargetPosition\r\n"));
	errorHandler.setInfo(F("pc.a,60,30         //rotate wheel 60 degrees with speed 30\r\n"));
	PT_YIELD();
	//xdes1
	errorHandler.setInfo(F("pc.cm,40,60,30,50  //drives left wheel 40cm at 30%% speed and right 60cm at 50%% speed\r\n"));
	errorHandler.setInfo(F("                  //negative cm drives backward\r\n"));
	errorHandler.setInfo(F("pc.s              //stop Positioning\r\n"));
	errorHandler.setInfo(F("pc.sp             //stop Positioning at perimeter\r\n"));

	PT_YIELD();


	errorHandler.setInfo(F("\r\n=== MOTOR INTERFACE SERVICE ===\r\n"));
	errorHandler.setInfo(F("turnto,60,30         //turn 60 degrees right (-60=>left) with speed 30\r\n"));
	errorHandler.setInfo(F("mot.mfb,40,80        //drive motors from 40%% to 80%% \r\n"));
	errorHandler.setInfo(F("                     //end test with: mot.mfb,0,0\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("mot.mfsp,-60,80      //drive motors from -60%% to 80%%\r\n"));
	errorHandler.setInfo(F("                     //stops first before run to next speed\r\n"));
	errorHandler.setInfo(F("                     //end test with: mot.mfsp,0,0\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("mot.mpfsb,360,80     //rotate both drive motor to 360° and then -360° with 80%% speed \r\n"));
	errorHandler.setInfo(F("                     //stops first before run to next speed\r\n"));
	errorHandler.setInfo(F("                     //end test with: mot.pfsb,0,0\r\n"));

	errorHandler.setInfo(F("mot.ort,20           //overrun test. drives robot until perimeter outside reached with 20%% speed\r\n"));
	errorHandler.setInfo(F("                     //needed to determin the FF values in TOverRun class\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== MOTOR L/R/M CURRENT SERVICE ===\r\n"));
	errorHandler.setInfo(F("mot.config        //show config\r\n"));
	errorHandler.setInfo(F("mot.cur           //show drive motor current\r\n"));
	errorHandler.setInfo(F("mot.curm          //show mow motor current\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("mot.scalel,1.2,1.4 //calculate scalefactor motor L for measured A of 1.2. Current shown with mot.cur = 1.4\r\n"));
	errorHandler.setInfo(F("mot.scaler,1.2,1.4 //calculate scalefactor motor R for measured A of 1.2. Current shown with mot.cur = 1.4\r\n"));
	errorHandler.setInfo(F("mot.scalem,1.2,1.4 //calculate scalefactor motor M for measured A of 1.2. Current shown with mot.curm = 1.4\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== CHARGE SYSTEM SERVICE ===\r\n"));
	errorHandler.setInfo(F("charge.config     //show config\r\n"));
	errorHandler.setInfo(F("charge.show       //show charge sensors\r\n"));
	errorHandler.setInfo(F("charge.relay,1/0  //turn relay on/off\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== BATTERY SERVICE ===\r\n"));
	errorHandler.setInfo(F("bat.config //show config\r\n"));
	errorHandler.setInfo(F("bat.show   //show battery voltage\r\n"));

	errorHandler.setInfo(F("\r\n=== TEMPERATURE SERVICE ===\r\n"));
	errorHandler.setInfo(F("temp.show   //show temperature and humidity\r\n"));


	errorHandler.setInfo(F("\r\n=== RAIN SENSOR SERVICE ===\r\n"));
	errorHandler.setInfo(F("rain.config //show config\r\n"));
	errorHandler.setInfo(F("rain.show   //show sensor value\r\n"));

	PT_YIELD();


	errorHandler.setInfo(F("\r\n=== ADC MANAGER ===\r\n"));
	errorHandler.setInfo(F("adc.config  //show adc config\r\n"));
	errorHandler.setInfo(F("adc.samples //show adc values\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== BHT COMANDS ===\r\n"));
	errorHandler.setInfo(F("set.spiral,0/1  //0=Off 1=On\r\n"));
	errorHandler.setInfo(F("show.distance   //show distance while drving to areaX\r\n"));
	errorHandler.setInfo(F("show.rot        //show rotate values\r\n"));
	errorHandler.setInfo(F("show.hist       //show history\r\n"));
	errorHandler.setInfo(F("show.bht        //show bht node id an names\n"));
	errorHandler.setInfo(F("bht.ln          //show last called node of each BHT run\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== LINEFOLLOWER COMANDS ===\r\n"));
	errorHandler.setInfo(F("set.lfki,0.1    //set line follower ki to 0.1\r\n"));
	errorHandler.setInfo(F("set.lfkp,0.1    //set line follower kp to 0.1\r\n"));
	errorHandler.setInfo(F("set.lfkd,0.1    //set line follower kd to0.1\r\n"));
	errorHandler.setInfo(F("show.lfpid      //show intern PID calculation\r\n"));
	errorHandler.setInfo(F("show.lftune     //show PID tunings\r\n"));
	errorHandler.setInfo(F("show.lf         //show line follower values\r\n"));
	errorHandler.setInfo(F("set.lftuning,1  //tune PID. 0=stop tuning\r\n"));
	errorHandler.setInfo(F("set.lfiterm,3.2   //set iterm limit to 3.2\r\n"));
	errorHandler.setInfo(F("bht.tri         //show states of trinagle finding\r\n"));



	PT_YIELD();


	errorHandler.setInfo(F("\r\n=== PERIMETER SERVICE ===\r\n"));
	errorHandler.setInfo(F("per.config  //show config\r\n"));
	errorHandler.setInfo(F("per.show    //show perimeter service sensor values\r\n"));
	errorHandler.setInfo(F("per.max     //show maximum perimeter value\r\n"));
	errorHandler.setInfo(F("per.adcocl  //show adc l offset corrected\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("per.adcocr  //show adc r offset corrected\r\n"));
	errorHandler.setInfo(F("per.corrl   //show correlation l\r\n"));
	errorHandler.setInfo(F("per.corrr   //show correlation r\r\n"));
	errorHandler.setInfo(F("per.corrsql //show squared correlation l\r\n"));
	errorHandler.setInfo(F("per.corrsqr //show squared correlation r\r\n"));
	PT_YIELD();
	errorHandler.setInfo(F("per.psnrfl  //show psnr array l\r\n"));
	errorHandler.setInfo(F("per.psnrfr  //show psnr array r\r\n"));
	errorHandler.setInfo(F("per.resultl //show matched filter results l\r\n"));
	errorHandler.setInfo(F("per.resultr //show matched filter results r\r\n"));
	errorHandler.setInfo(F("per.fftl    //show matched filter translation l\r\n"));
	errorHandler.setInfo(F("per.fftr    //show matched filter translation r\r\n"));
	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== BUMPER SERVICE ===\r\n"));
	errorHandler.setInfo(F("bumper.config //show config\r\n"));
	errorHandler.setInfo(F("bumper.show   //show bumper sensor event\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== RANGE SERVICE ===\r\n"));
	errorHandler.setInfo(F("range.config //show config\r\n"));
	errorHandler.setInfo(F("range.show   //show range sensor event\r\n"));


	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== GPS SERVICE ===\r\n"));
	errorHandler.setInfo(F("gps.config //show config\r\n"));
	errorHandler.setInfo(F("gps.show   //show calculated gps data\r\n"));

	PT_YIELD();

	errorHandler.setInfo(F("\r\n=== OTHER ===\r\n"));
	errorHandler.setInfo(F("show.mem  //show free memory\r\n"));
	errorHandler.setInfo(F("show.stat //show statistic\n"));
	errorHandler.setInfo(F("h         //hide showing\r\n"));

	PT_YIELD();

	//xdes1
	errorHandler.setInfo(F("\r\n=== Control Center ===\r\n"));
	errorHandler.setInfo(F("set.cco,1/0  //turn output for Control Center on/off\r\n"));

	errorHandler.setInfo(F("show.dur     //print out duration of threads\r\n"));


	PT_END();
}

/********************************************************************************
********************************************************************************
**functions for user commands
** each function represents one user command
********************************************************************************
*********************************************************************************/

void cmd_help(int arg_cnt, char** args) {

	srvUI.Restart();
}



bool checkManualMode() {
	if (_controlManuel != true) {
		errorHandler.setInfo(F("!03,NEED TO BE IN MANUAL MODE!\r\n"));
		return false;
	}

	return true;
}





// Closed loop control comands

void cmd_clc_setKP(int arg_cnt, char** args) {
	float val = cmdStr2Float(args[1]);
	srvClcL.kp = val;
	srvClcR.kp = val;
}

void cmd_clc_setKI(int arg_cnt, char** args) {
	float val = cmdStr2Float(args[1]);
	srvClcL.ki = val;
	srvClcR.ki = val;
}

void cmd_clc_setKFR(int arg_cnt, char** args) {
	float val = cmdStr2Float(args[1]);
	srvClcL.kfr = val;
	srvClcR.kfr = val;
}

void cmd_clc_show_config(int arg_cnt, char** args) {
	srvClcL.showConfig();
	srvClcR.showConfig();
}

void cmd_clc_showSpeedL(int arg_cnt, char** args) {
	srvClcL.flagShowSpeed = !srvClcL.flagShowSpeed;
}

void cmd_clc_showSpeedR(int arg_cnt, char** args) {
	srvClcR.flagShowSpeed = !srvClcR.flagShowSpeed;
}


void cmd_clc_showSetpointCurSpeedL(int arg_cnt, char** args) {
	srvClcL.flagShowSetpointCurrSpeed = !srvClcL.flagShowSetpointCurrSpeed;
}


void cmd_clc_showSetpointCurSpeedR(int arg_cnt, char** args) {
	srvClcR.flagShowSetpointCurrSpeed = !srvClcR.flagShowSetpointCurrSpeed;
}

void cmd_clc_showCallOfEnableXRamping(int arg_cnt, char** args) {
	srvClcL.flagShowEnableRamping = !srvClcL.flagShowEnableRamping;
	srvClcR.flagShowEnableRamping = !srvClcR.flagShowEnableRamping;
}




void cmd_clc_setSpeed(int arg_cnt, char** args) {
	if (checkManualMode()) {
		int val = cmdStr2Num(args[1], 10);
		srvClcL.setSpeed(val);
		srvClcR.setSpeed(val);
	}
}


void cmd_clc_setAgility(int arg_cnt, char** args) {
	float val0 = cmdStr2Float(args[1]);
	float val1 = cmdStr2Float(args[2]);
	srvClcL.setOutputToZeroAtRPm = val0;
	srvClcL.stopReachedThresholdAtRpm = val1;
	srvClcR.setOutputToZeroAtRPm = val0;
	srvClcR.stopReachedThresholdAtRpm = val1;

}



void cmd_clc_driveStop(int arg_cnt, char** args) {
	if (checkManualMode()) {
		//srvMotor.stop();
		srvClcL.stop();
		srvClcR.stop();
	}
}

void cmd_clc_showEncoder(int arg_cnt, char** args) {
	srvClcL.flagShowEncoder = !srvClcL.flagShowEncoder;
	srvClcR.flagShowEncoder = !srvClcR.flagShowEncoder;
}

void cmd_clc_motorTest(int arg_cnt, char** args) {
	int mot = cmdStr2Num(args[1], 10);
	int val = cmdStr2Num(args[2], 10);

	if (checkManualMode()) {

		if (val == 0) {
			srvClcL.flagControldirect = false;
			srvClcR.flagControldirect = false;
			srvClcL.controlDirect(val);
			srvClcR.controlDirect(val);
			srvClcL.stop();
			srvClcR.stop();
			return;
		}
		if (mot == 1) {
			srvClcL.flagControldirect = true;
			motorDriver.resetFault(true);
			srvClcL.controlDirect(val);
		}
		if (mot == 2) {
			srvClcR.flagControldirect = true;
			motorDriver.resetFault(true);
			srvClcR.controlDirect(val);
		}

	}
}


// Runs both motors in closed loop from speedMinTest to speedMaxTest and vice versa
void cmd_motor_motorStepSpeed(int arg_cnt, char** args) {
	if (checkManualMode()) {

		srvMotor.speedMinTest = cmdStr2Num(args[1], 10);
		srvMotor.speedMaxTest = cmdStr2Num(args[2], 10);

		// Turn off test
		if (srvMotor.speedMinTest == 0 && srvMotor.speedMaxTest == 0) {
			srvMotor.stateTest = 99;
		}
		// Turn on test
		else {
			srvMotor.stateTest = 1;
			srvMotor.flagMotorStepSpeed = true;
		}
	}
}

void cmd_motor_motorFSB(int arg_cnt, char** args) {

	if (checkManualMode()) {

		srvMotor.speedMinTest = cmdStr2Num(args[1], 10);
		srvMotor.speedMaxTest = cmdStr2Num(args[2], 10);

		// Turn off test
		if (srvMotor.speedMinTest == 0 && srvMotor.speedMaxTest == 0) {
			srvMotor.stateTest = 99;
		}
		// Turn on test
		else {
			srvMotor.stateTest = 1;
			srvMotor.flagMotorFSB = true;
		}
	}

}


void cmd_motor_motorPFSB(int arg_cnt, char** args) {

	if (checkManualMode()) {

		srvMotor.speedMinTest = cmdStr2Num(args[1], 10); // Angle
		srvMotor.speedMaxTest = cmdStr2Num(args[2], 10); // Speed

		// Turn off test
		if (srvMotor.speedMinTest == 0 && srvMotor.speedMaxTest == 0) {
			srvMotor.stateTest = 99;
		}
		// Turn on test
		else {
			srvMotor.stateTest = 1;
			srvMotor.flagMotorPFSB = true;

		}
	}

}


void cmd_motor_motorOverRunTest(int arg_cnt, char** args) {

	if (checkManualMode()) {

		srvMotor.speedMinTest = cmdStr2Num(args[1], 10); // Angle
		// Turn off test
		if (srvMotor.speedMinTest == 0) {
			srvMotor.stateTest = 99;
		}
		// Turn on test
		else {
			srvMotor.stateTest = 1;
			srvMotor.flagMotorPerOverrun = true;

		}
	}

}

void cmd_clcM_motorTest(int arg_cnt, char** args) {
	int mot = cmdStr2Num(args[1], 10);
	int val = cmdStr2Num(args[2], 10);
	if (checkManualMode()) {

		if (val == 0) {
			srvClcM.Restart();
			mowMotorDriver.motor(1, val);
			srvClcM.stop();
			return;
		}

		if (mot == 1) {
			srvClcM.Stop();
			mowMotorDriver.resetFault(true);
			mowMotorDriver.motor(1, val);
		}

	}
}



void cmd_clcm_show_config(int arg_cnt, char** args) {
	srvClcM.showConfig();
}

void cmd_clcm_showSpeed(int arg_cnt, char** args) {
	srvClcM.flagShowSpeed = !srvClcM.flagShowSpeed;
}

void cmd_clcm_setRamp(int arg_cnt, char** args) {
	int val = cmdStr2Num(args[1], 10);
	srvClcM.motorMowAccel = val;
}

void cmd_clcm_setSpeedLimit(int arg_cnt, char** args) {
	float val = cmdStr2Float(args[1]);
	srvClcM.speedLimit = val;
}

void cmd_pc_setTuneup(int arg_cnt, char** args) {
	srvPcL.stopCmBeforeTarget = cmdStr2Float(args[1]);
	//srvPcL.addCmToTargetPosition = cmdStr2Float(args[2]);

	srvPcR.stopCmBeforeTarget = srvPcL.stopCmBeforeTarget;
	//srvPcR.addCmToTargetPosition = srvPcL.addCmToTargetPosition;
}

void cmd_pos_show_config(int arg_cnt, char** args) {
	srvPcL.showConfig();
	srvPcR.showConfig();
}

void cmd_pos_show_resultsL(int arg_cnt, char** args) {
	srvPcL.flagShowResults = !srvPcL.flagShowResults;
}

void cmd_pos_show_resultsR(int arg_cnt, char** args) {
	srvPcR.flagShowResults = !srvPcR.flagShowResults;
}




void cmd_goHome(int arg_cnt, char** args) {
	myBlackboard.flagGoHome = true;
}

void cmd_testPerimeterTracking(int arg_cnt, char** args) {
	_controlManuel = false;
	myCreateTree.reset();
	myBlackboard.setBehaviour(BH_PERITRACK);
	errorHandler.setInfo(F("Drive to dock\r\n"));
}

void cmd_driveAngle(int arg_cnt, char** args) {
	if (checkManualMode()) {
		float winkel = cmdStr2Float(args[1]);
		float speed = cmdStr2Float(args[2]);
		srvMotor.rotateAngle(winkel, speed);
	}
}

void cmd_driveCM(int arg_cnt, char** args) {
	if (checkManualMode()) {
		float cmL = cmdStr2Float(args[1]);
		float cmR = cmdStr2Float(args[2]);
		float speedL = cmdStr2Float(args[3]);
		float speedR = cmdStr2Float(args[4]);
		srvMotor.rotateCM(cmL, cmR, speedL, speedR);
	}
}

void cmd_turnTo(int arg_cnt, char** args) {
	if (checkManualMode()) {
		float winkel = cmdStr2Float(args[1]);
		float speed = cmdStr2Float(args[2]);
		srvMotor.turnTo(winkel, speed);
	}
}

void cmd_stopPositioning(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvMotor.stopPC();
	}
}

void cmd_stopPositioningAtPer(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvMotor.stopPCAtPerimeter();
	}
}




void cmd_startMowMot(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvMotor.mowMotStart();
		srvMotor.M->uiMotorDisabled = false;
		srvMotor.M->motorDisabled = false;
	}
	else {
		srvMotor.M->uiMotorDisabled = false;
	}
}

void cmd_stopMowMot(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvMotor.mowMotStop();
	}
	else {
		srvMotor.M->uiMotorDisabled = true;
	}
}

void cmd_cntrManuel(int arg_cnt, char** args) {
	_controlManuel = true;;
	myBlackboard.setBehaviour(BH_NONE);
	srvMotor.stopAllMotors();
}

void cmd_cntrAuto(int arg_cnt, char** args) {
	if (myBlackboard.flagEnableMowing == false) {
		myCreateTree.reset();
		myBlackboard.setBehaviour(BH_MOW);
		_controlManuel = false;
	}

}

void cmd_cntrGotoAreaX(int arg_cnt, char** args) {
	myCreateTree.reset();
	long distance = cmdStr2Num(args[1], 10);
	if (CONF_LEAVE_CHARGING_STATION_BACKWARDS) {
		myBlackboard.setBehaviour(BH_LEAVE_HEAD_CS);
	}
	else {
		myBlackboard.setBehaviour(BH_GOTOAREA);
	}

	_controlManuel = false;
	//must set after set behaviour, because behaviour resets the BB
	myBlackboard.areaTargetDistanceInMeter = distance;
	errorHandler.setInfo(F("Drive to area: %l\r\n"), distance);

}


/*
  void cmd_cntrRestoreHistory(int arg_cnt, char **args)
  {
    _controlManuel = false;
    //myCreateTree.reset();
    int num = cmdStr2Num(args[1], 10);
    myBlackboard.setBehaviour(BH_RESTOREHISTORY);
    //must set after set behaviour, because behaviour resets the BB
    myBlackboard.numberToRestoreHist = num;
    errorHandler.setInfo(F("Restore history: %d\r\n"), num);

  }
*/

void cmd_showBattery(int arg_cnt, char** args) {
	//xdes1
	errorHandler.setInfo(F("Battery Voltage: %f sensorValue: %f "), srvBatSensor.voltage, srvBatSensor.sensorValue);
	errorHandler.setInfo(F("aiBATVOLT.read_int32() %d\r\n"), aiBATVOLT.read_int32());
	//errorHandler.setInfo(F("$batV,%f,%f,%d\r\n"), srvBatSensor.voltage, srvBatSensor.sensorValue);
}

void cmd_showRain(int arg_cnt, char** args) {
	srvRainSensor.flagShowRainSensor = !srvRainSensor.flagShowRainSensor;
}

//bber2
void cmd_showTemperature(int arg_cnt, char** args) {

	if (CONF_DISABLE_DHT_SERVICE == true) {
		errorHandler.setInfo(F("Temperature service deactivated\r\n"));
		return;
	}
	//xdes1
	if (checkManualMode()) {
		float temp;
		temp = srvDht.readTemperature();
		errorHandler.setInfo(F("Current Temperature: %f\r\n"), temp);
		errorHandler.setInfo(F("Current Humidity: %f\r\n"), srvDht.readHumidity());
		errorHandler.setInfo(F("Temperature stored in service: %f,%d\r\n"), srvDht.getLastReadTemperature(), srvDht.errorCounter);
		//errorHandler.setInfo(F("$temp, %.1f,%d\r\n"), srvDht.getLastReadTemperature(), srvDht.errorCounter);
	}
	else {
		errorHandler.setInfo(F("Temperature stored in service: %f\r\n"), srvDht.getLastReadTemperature());
		//errorHandler.setInfo(F("$temp, %.1f,%d\r\n"), srvDht.getLastReadTemperature(), srvDht.errorCounter);
	}

}

void cmd_activateControlCenterOutput(int arg_cnt, char** args) {

	int i = atoi(args[1]);
	//xdes1
	if (i == 0) {
		srvGps.flagSendToCC = false;
		srvDht.hide();
		srvBatSensor.hide();
		_printProcessingData = false;
		errorHandler.setInfo(F("Control Center Output Off\r\n"), i);
	}
	else {

		if (CONF_DISABLE_DHT_SERVICE == false) {
			srvDht.show();
		}
		if (CONF_DISABLE_BATTERY_SERVICE == false) {
			srvBatSensor.show();
		}
		if (CONF_DISABLE_GPS == false) {
			srvGps.flagSendToCC = true;
		}
		_printProcessingData = true;
		errorHandler.setInfo(F("Control Center Output On\r\n"), i);
	}
}//ENDFUNC

//----------
void cmd_showMowSensor(int arg_cnt, char** args) {
	srvMowMotorSensor.showValuesOnConsole = !srvMowMotorSensor.showValuesOnConsole;
}


void cmd_showStatistik(int arg_cnt, char** args) {

	errorHandler.setInfo(F("CURRENT:\r\n"));

	unsigned long time = millis() - myBlackboard.timeInMowBehaviour;
	double minutes = (double)time / 60000.0;
	errorHandler.setInfo(F("MOWTIME %fmin\r\n"), minutes);

	float ticks = (srvMotor.L->myEncoder->getAbsTicksCounter() + srvMotor.R->myEncoder->getAbsTicksCounter()) / 2.0f;
	float mowway = srvMotor.getMForCounts(ticks);
	errorHandler.setInfo(F("MOWDIRVENWAY %f\r\n"), mowway);

	errorHandler.setInfo(F("ROTATIONCOUNT %d\r\n"), (int)myBlackboard.numberOfRotations);


	errorHandler.setInfo(F("SAVED:\r\n"));
	int count = srvEeprom.read32t(EEPADR_CHARGINGCOUNT);
	float mowtime = srvEeprom.readFloat(EEPADR_MOWTIME);
	mowway = srvEeprom.readFloat(EEPADR_MOWDIRVENWAY);
	int32_t rotations = srvEeprom.read32t(EEPADR_ROTATIONCOUNT);
	errorHandler.setInfo(F("CHARGINGCOUNT %d\r\n"), count);
	errorHandler.setInfo(F("MOWTIME %fm\r\n"), mowtime * 60.0f);
	errorHandler.setInfo(F("MOWTIME %fh\r\n"), mowtime);
	errorHandler.setInfo(F("MOWDIRVENWAY %fm\r\n"), mowway);
	errorHandler.setInfo(F("ROTATIONCOUNT %d\r\n"), rotations);


}


void cmd_mot_show_config(int arg_cnt, char** args) {
	srvMotorSensorL.showConfig();
	srvMotorSensorR.showConfig();
	srvMowMotorSensor.showConfig();
}


void cmd_showMotorCurrent(int arg_cnt, char** args) {
	srvMotorSensorL.showValuesOnConsole = !srvMotorSensorL.showValuesOnConsole;
	srvMotorSensorR.showValuesOnConsole = !srvMotorSensorR.showValuesOnConsole;
}


void cmd_motorCalculateLScale(int arg_cnt, char** args) {
	float data = cmdStr2Float(args[1]);
	float data2 = cmdStr2Float(args[2]);
	srvMotorSensorL.calculateScale(data, data2);
}
void cmd_motorCalculateRScale(int arg_cnt, char** args) {
	float data = cmdStr2Float(args[1]);
	float data2 = cmdStr2Float(args[2]);
	srvMotorSensorR.calculateScale(data, data2);
}

void cmd_motorCalculateMScale(int arg_cnt, char** args) {
	float data = cmdStr2Float(args[1]);
	float data2 = cmdStr2Float(args[2]);
	srvMowMotorSensor.calculateScale(data, data2);
}



void cmd_showPerimeter(int arg_cnt, char** args) {
	srvPerSensoren.showValuesOnConsole = !srvPerSensoren.showValuesOnConsole;
}

void cmd_showPerimeterMax(int arg_cnt, char** args) {
	errorHandler.setInfo(F("magMax:   %ld\r\n"), srvPerSensoren.magMax);
}

void cmd_showPerAdcOffsetCorrectedL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showADCWithoutOffset = !srvPerSensoren.coilL.showADCWithoutOffset;
}

void cmd_showPerAdcOffsetCorrectedR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showADCWithoutOffset = !srvPerSensoren.coilR.showADCWithoutOffset;
}


void cmd_showPerCorrelationL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showCorrelation = !srvPerSensoren.coilL.showCorrelation;
}

void cmd_showPerCorrelationR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showCorrelation = !srvPerSensoren.coilR.showCorrelation;
}


void cmd_showPerCorrelationSQL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showCorrelationSQ = !srvPerSensoren.coilL.showCorrelationSQ;
}

void cmd_showPerCorrelationSQR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showCorrelationSQ = !srvPerSensoren.coilR.showCorrelationSQ;
}


void cmd_showPerPsnrFL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showPSNRFunction = !srvPerSensoren.coilL.showPSNRFunction;
}

void cmd_showPerPsnrFR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showPSNRFunction = !srvPerSensoren.coilR.showPSNRFunction;
}


void cmd_showPerResultL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showValuesResults = !srvPerSensoren.coilL.showValuesResults;
}

void cmd_showPerResultR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showValuesResults = !srvPerSensoren.coilR.showValuesResults;
}


void cmd_showFFTL(int arg_cnt, char** args) {
	srvPerSensoren.coilL.showMatchedFilter = !srvPerSensoren.coilL.showMatchedFilter;
}

void cmd_showFFTR(int arg_cnt, char** args) {
	srvPerSensoren.coilR.showMatchedFilter = !srvPerSensoren.coilR.showMatchedFilter;
}



void cmd_showRTC(int arg_cnt, char** args) {
	//bber11
	//srvRtc.flagShowRTCRead = !srvRtc.flagShowRTCRead;

	//if (srvRtc.flagShowRTCRead)
	//{
	errorHandler.setInfo(F("millis():   %lu\r\n"), millis());

	//errorHandler.setInfo(F("Current RTC:\r\n"));  //here i suppose it's the time in the due and not updated by the srvRtc
	//srvRtc.showImmediately();

	errorHandler.setInfo(F("Read from RTC:\r\n"));
	srvRtc.readDS1307();
	srvRtc.showImmediately();
	//}
	//errorHandler.setInfo(F( "micros():   %lu\r\n",micros());
	//errorHandler.setInfo(F( "micros64(): %llu\r\n",micros64());
	//errorHandler.setInfo(F( "millis64(): %llu\r\n",millis64());
	//errorHandler.setInfo(F( "Sekunden(): %llu\r\n",micros64()/1000000ULL);
	//errorHandler.setInfo(F( "Minuten():  %llu\r\n",micros64()/1000000ULL/60ULL);
}

void cmd_showRTCfind(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvRtc.findDS1307();
	}
}

void cmd_showRTCconfig(int arg_cnt, char** args) {
	srvRtc.showConfig();
}




void cmd_setRTC(int arg_cnt, char** args) {
	if (checkManualMode()) {
		srvRtc.hour = cmdStr2Num(args[1], 10);;
		srvRtc.minute = cmdStr2Num(args[2], 10);;
		srvRtc.dayOfWeek = cmdStr2Num(args[3], 10);;
		srvRtc.day = cmdStr2Num(args[4], 10);;
		srvRtc.month = cmdStr2Num(args[5], 10);;
		srvRtc.year = cmdStr2Num(args[6], 10);;
		srvRtc.showImmediately();
		errorHandler.setInfo(F("saving...\r\n"));
		srvRtc.setDS1307();
		errorHandler.setInfo(F("saved\r\n"));
		delay(500);
		errorHandler.setInfo(F("reading from srvRtc...\r\n"));
		srvRtc.readDS1307();
		srvRtc.showImmediately();
		errorHandler.setInfo(F("read\r\n"));

	}
}


void cmd_showEEPROM_Erase(int arg_cnt, char** args) {
	srvEeprom.erase();
}

void cmd_showEEPROMCconfig(int arg_cnt, char** args) {
	srvEeprom.showConfig();
}

void cmd_setEEPROMbyte(int arg_cnt, char** args) {
	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);
		uint8_t data = cmdStr2Num(args[2], 10);

		srvEeprom.writeu8t(address, data);
		errorHandler.setInfo(F("saved\r\n"));
	}

}

void cmd_setEEPROM32t(int arg_cnt, char** args) {
	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);
		int32_t data = cmdStr2Num(args[2], 10);

		srvEeprom.write32t(address, data);
		errorHandler.setInfo(F("saved\r\n"));
	}

}

void cmd_setEEPROMfloat(int arg_cnt, char** args) {
	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);
		float data = cmdStr2Float(args[2]);

		srvEeprom.writeFloat(address, data);
		errorHandler.setInfo(F("saved\r\n"));
	}

}

void cmd_showEEPROMbyte(int arg_cnt, char** args) {
	uint8_t data;

	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);

		data = srvEeprom.readu8t(address);
		errorHandler.setInfo(F("Value: %d\r\n"), data);

	}
}


void cmd_showEEPROM32t(int arg_cnt, char** args) {
	int32_t data;

	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);

		data = srvEeprom.read32t(address);
		errorHandler.setInfo(F("Value: %d\r\n"), data);

	}
}

void cmd_showEEPROMfloat(int arg_cnt, char** args) {
	float data;

	if (checkManualMode()) {
		int16_t address = cmdStr2Num(args[1], 10);
		data = srvEeprom.readFloat(address);
		errorHandler.setInfo(F("Value: %f\r\n"), data);

	}
}


void cmd_showi2c(int arg_cnt, char** args) {
	if (checkManualMode()) {
		i2cInOut::I2C_scan();
	}
}


void cmd_range_show_config(int arg_cnt, char** args) {
	srvRangeSensor.showConfig();
}

void cmd_showRange(int arg_cnt, char** args) {
	srvRangeSensor.flagShowRange = !srvRangeSensor.flagShowRange;
}

void cmd_gps_show_config(int arg_cnt, char** args) {
	srvGps.showConfig();
}

void cmd_showGPS(int arg_cnt, char** args) {
	if (CONF_DEACTIVATE_GPS_CALCULATION) {
		errorHandler.setInfo(F("GPS calculation deactivated\r\n"));
	}
	else {
		srvGps.flagShowGPS = !srvGps.flagShowGPS;
	}
}

void cmd_bumper_show_config(int arg_cnt, char** args) {
	srvBumperSensor.showConfig();
}

void cmd_showBumper(int arg_cnt, char** args) {
	srvBumperSensor.flagShowBumper = !srvBumperSensor.flagShowBumper;
}

void cmd_showChargeSystem(int arg_cnt, char** args) {
	srvChargeSystem.flagShowChargeSystem = !srvChargeSystem.flagShowChargeSystem;
}

void cmd_charge_show_config(int arg_cnt, char** args) {
	srvChargeSystem.showConfig();
}


void cmd_bat_show_config(int arg_cnt, char** args) {
	srvBatSensor.showConfig();
}


void cmd_rain_show_config(int arg_cnt, char** args) {
	srvRainSensor.showConfig();
}

void cmd_per_show_config(int arg_cnt, char** args) {
	srvPerSensoren.showConfig();
}



void cmd_printError(int arg_cnt, char** args) {
	if (checkManualMode()) {
		errorHandler.print();
		errorHandler.printError();
	}

}

void cmd_resetError(int arg_cnt, char** args) {
	errorHandler.resetError();
	//lastTimeShowError = 0;
	motorDriver.resetFault(true);
	mowMotorDriver.resetFault(true);
	maxLoopTime = 0;
}



void cmd_showMem(int arg_cnt, char** args) {
	FreeMem();
}


void cmd_showADCPrint(int arg_cnt, char** args) {
	ADCMan.printInfo();
}

void cmd_showADCValues(int arg_cnt, char** args) {
	if (checkManualMode()) {
		ADCMan.showValuesOnConsole = !ADCMan.showValuesOnConsole;
	}
}


void cmd_showGotoAreaDistance(int arg_cnt, char** args) {
	srvMotor.flagShowDistance = true;
}


void cmd_showRotate(int arg_cnt, char** args) {

	myBlackboard.flagShowRotateX = !myBlackboard.flagShowRotateX;
	errorHandler.setInfo(F("showRot\r\n"));
}

void cmd_showHistory(int arg_cnt, char** args) {
	if (checkManualMode()) {

		for (int x = HISTROY_BUFSIZE - 1; x > -1; x--) {
			myBlackboard.printHistoryEntry(x);
		}

		return;
	}

	myBlackboard.flagShowHistory = !myBlackboard.flagShowHistory;
	errorHandler.setInfo(F("showHist\r\n"));
}


void cmd_showBHTTriangle(int arg_cnt, char** args) {
	findTriangle.flagShowFindTriangleStates = !findTriangle.flagShowFindTriangleStates;
}

void cmd_showBHTShowLastNode(int arg_cnt, char** args) {
	myBlackboard.flagBHTShowLastNode = !myBlackboard.flagBHTShowLastNode;
}



void cmd_hideShowing(int arg_cnt, char** args) {
	srvClcL.flagShowSpeed = false;
	srvClcR.flagShowSpeed = false;
	srvClcL.flagShowEncoder = false;
	srvClcR.flagShowEncoder = false;
	srvMowMotorSensor.showValuesOnConsole = false;
	srvPerSensoren.showValuesOnConsole = false;
	srvRangeSensor.flagShowRange = false;
	srvGps.flagShowGPS = false;
	srvBumperSensor.flagShowBumper = false;
	srvChargeSystem.flagShowChargeSystem = false;
	srvMotor.flagShowDistance = false;

	srvRainSensor.flagShowRainSensor = false;

	ADCMan.showValuesOnConsole = false;
	srvRtc.flagShowRTCRead = false;

	srvClcL.flagShowSetpointCurrSpeed = false;
	srvClcR.flagShowSetpointCurrSpeed = false;
	srvClcL.flagShowEnableRamping = false;
	srvClcR.flagShowEnableRamping = false;

	srvPcL.flagShowResults = false;
	srvPcR.flagShowResults = false;
	srvClcM.flagShowSpeed = false;
	srvMotorSensorL.showValuesOnConsole = false;
	srvMotorSensorR.showValuesOnConsole = false;

	srvPerSensoren.coilL.showADCWithoutOffset = false;
	srvPerSensoren.coilR.showADCWithoutOffset = false;
	srvPerSensoren.coilL.showCorrelation = false;
	srvPerSensoren.coilR.showCorrelation = false;
	srvPerSensoren.coilL.showCorrelationSQ = false;
	srvPerSensoren.coilR.showCorrelationSQ = false;
	srvPerSensoren.coilL.showPSNRFunction = false;
	srvPerSensoren.coilR.showPSNRFunction = false;
	srvPerSensoren.coilL.showValuesResults = false;
	srvPerSensoren.coilR.showValuesResults = false;
	srvPerSensoren.coilL.showMatchedFilter = false;
	srvPerSensoren.coilR.showMatchedFilter = false;

	findTriangle.flagShowFindTriangleStates = false;
	myBlackboard.flagBHTShowLastNode = CONF_bb_flagBHTShowLastNode;
	myBlackboard.flagShowRotateX = false;
	myBlackboard.flagShowHistory = false;

	lineFollow.pid.flagShowValues = false;
	lineFollow.flagShowValues = false;

	errorHandler.setInfo(F("HIDE\r\n"));

}


void cmd_setLineFollowerKi(int arg_cnt, char** args) {

	lineFollow.pid.SetKi(cmdStr2Float(args[1]));
	errorHandler.setInfo(F("KI: %f\r\n"), lineFollow.pid.GetKi());

}

void cmd_setLineFollowerKp(int arg_cnt, char** args) {

	lineFollow.pid.SetKp(cmdStr2Float(args[1]));
	errorHandler.setInfo(F("KP: %f\r\n"), lineFollow.pid.GetKp());

}


void cmd_setLineFollowerKd(int arg_cnt, char** args) {

	lineFollow.pid.SetKd(cmdStr2Float(args[1]));
	errorHandler.setInfo(F("KD: %f\r\n"), lineFollow.pid.GetKd());

}

void cmd_showLineFollowerPID(int arg_cnt, char** args) {

	lineFollow.pid.flagShowValues = !lineFollow.pid.flagShowValues;


}


void cmd_showLineFollowerTunings(int arg_cnt, char** args) {
	errorHandler.setInfo(F("KP: %f\r\n"), lineFollow.pid.GetKp());
	errorHandler.setInfo(F("KI: %f\r\n"), lineFollow.pid.GetKi());
	errorHandler.setInfo(F("KD: %f\r\n"), lineFollow.pid.GetKd());;
	errorHandler.setInfo(F("iTermLimit: %f\r\n"), lineFollow.iTermLimit);;
}


void cmd_showLineFollower(int arg_cnt, char** args) {

	lineFollow.flagShowValues = !lineFollow.flagShowValues;


}

void cmd_setLineFollowerPIDTuning(int arg_cnt, char** args) {

	int i = cmdStr2Num(args[1], 10);

	if (i == 0) {
		lineFollow.tunePID = false;
		errorHandler.setInfo(F("LF PID tuning  disabled i: %d\r\n"), i);
	}
	else {
		lineFollow.tunePID = true;
		errorHandler.setInfo(F("LF PID tuning  enabled i: %d\r\n"), i);
	}
}


void cmd_setLineFolloweriTermLimit(int arg_cnt, char** args) {

	float i = cmdStr2Float(args[1]);
	lineFollow.iTermLimit = i;
	errorHandler.setInfo(F("LF iTermLimit set to: %f\r\n"), i);
}


void cmd_setChargeRelay(int arg_cnt, char** args) {
	int i = cmdStr2Num(args[1], 10);

	if (i == 0) {
		srvChargeSystem.deactivateRelay();
		errorHandler.setInfo(F("Relay disabled i: %d\r\n"), i);
	}
	else {
		srvChargeSystem.activateRelay();
		errorHandler.setInfo(F("Relay enabled: %d\r\n"), i);
	}
}





void cmd_checkBTModule(int arg_cnt, char** args) {
	if (checkManualMode()) {
		BluetoothConfig bt;
		bt.checkModule(true);
	}
}

void cmd_setBTBodule(int arg_cnt, char** args) {
	if (checkManualMode()) {
		BluetoothConfig bt;
		bt.setParams(F("Ardumower"), 1234, CONF_BT_SERIAL_SPEED, true);
	}
}

void cmd_setCruiseSpiral(int arg_cnt, char** args) {
	int i = cmdStr2Num(args[1], 10);

	if (i == 0) {
		myBlackboard.flagCruiseSpiral = false;
		errorHandler.setInfo(F("CruiseSpiral disabled i: %d\r\n"), i);
	}
	else {
		myBlackboard.flagCruiseSpiral = true;
		errorHandler.setInfo(F("CruiseSpiral enabled: %d\r\n"), i);
	}
}


void cmd_showBHT(int arg_cnt, char** args) {
	if (checkManualMode()) {
		myCreateTree.print();
	}
}



//xdes1
void cmd_PowerOff(int arg_cnt, char** args) {
	srvShutdown.Restart();
}

// Print "hello world" when called from the command line.
//
// Usage:
// hello
void cmd_hello(int arg_cnt, char** args) {
	errorHandler.setInfo(F("Hello world.\r\n"));
}

void cmd_showDuration(int arg_cnt, char** args) {
	//controller.showDuration();
}

// Display the contents of the args string array.
//
// Usage:
// args 12 34 56 hello gothic baby
//
// Will display the contents of the args array as a list of strings
// Output:
// Arg 0: args
// Arg 1: 12
// Arg 2: 34
// Arg 3: 56
// Arg 4: hello
// Arg 5: gothic
// Arg 6: baby
void cmd_arg_display(int arg_cnt, char** args) {
	for (int i = 0; i < arg_cnt; i++) {
		errorHandler.setInfo(F("Arg %i: %s\r\n"), i, args[i]);
	}
}

/********************************************************************************
********************************************************************************
**  setup function for user commands
**
********************************************************************************
*********************************************************************************/


void cmd_setup() {


	cmdAdd((char*)"hello", cmd_hello);

	cmdAdd((char*)"args", cmd_arg_display);
	cmdAdd((char*)"H", cmd_help);

	//xdes1
	cmdAdd((char*)"poweroff", cmd_PowerOff);


	// Mode Selection
	cmdAdd((char*)"M", cmd_cntrManuel);
	cmdAdd((char*)"A", cmd_cntrAuto);
	cmdAdd((char*)"area", cmd_cntrGotoAreaX);
	cmdAdd((char*)"gohome", cmd_goHome);
	cmdAdd((char*)"tpt", cmd_testPerimeterTracking);
	//cmdAdd((char *)"rh", cmd_cntrRestoreHistory);



	cmdAdd((char*)"rtc.show", cmd_showRTC);
	cmdAdd((char*)"rtc.config", cmd_showRTCconfig);
	cmdAdd((char*)"rtc.find", cmd_showRTCfind);
	cmdAdd((char*)"rtc.set", cmd_setRTC);
	cmdAdd((char*)"eep.config", cmd_showEEPROMCconfig);
	cmdAdd((char*)"eep.u8t", cmd_showEEPROMbyte);
	cmdAdd((char*)"eep.s32t", cmd_showEEPROM32t);
	cmdAdd((char*)"eep.f", cmd_showEEPROMfloat);
	cmdAdd((char*)"eep.set.u8t", cmd_setEEPROMbyte);
	cmdAdd((char*)"eep.set.s32t", cmd_setEEPROM32t);
	cmdAdd((char*)"eep.set.f", cmd_setEEPROMfloat);
	cmdAdd((char*)"eep.erase", cmd_showEEPROM_Erase);



	// closed loop control service
	//------------------------------
	cmdAdd((char*)"clc.config", cmd_clc_show_config);
	cmdAdd((char*)"clc.enc", cmd_clc_showEncoder);


	cmdAdd((char*)"clc.scl", cmd_clc_showSetpointCurSpeedL);
	cmdAdd((char*)"clc.scr", cmd_clc_showSetpointCurSpeedR);
	cmdAdd((char*)"clc.ser", cmd_clc_showCallOfEnableXRamping);

	cmdAdd((char*)"clc.speedl", cmd_clc_showSpeedL);
	cmdAdd((char*)"clc.speedr", cmd_clc_showSpeedR);
	cmdAdd((char*)"clc.v", cmd_clc_setSpeed);
	cmdAdd((char*)"clc.s", cmd_clc_driveStop);
	cmdAdd((char*)"clc.p", cmd_clc_setKP);
	cmdAdd((char*)"clc.i", cmd_clc_setKI);
	cmdAdd((char*)"clc.k", cmd_clc_setKFR);

	cmdAdd((char*)"clc.ag", cmd_clc_setAgility);


	cmdAdd((char*)"clc.mt", cmd_clc_motorTest);


	// Mow Motor CLCM
	cmdAdd((char*)"clcm.config", cmd_clcm_show_config);
	cmdAdd((char*)"clcm.speed", cmd_clcm_showSpeed);
	cmdAdd((char*)"clcm.accel", cmd_clcm_setRamp);
	cmdAdd((char*)"clcm.limit", cmd_clcm_setSpeedLimit);

	cmdAdd((char*)"z", cmd_startMowMot);
	cmdAdd((char*)"t", cmd_stopMowMot);

	// Position Control Service
	//------------------------------
	cmdAdd((char*)"pc.config", cmd_pos_show_config);
	cmdAdd((char*)"pc.L", cmd_pos_show_resultsL);
	cmdAdd((char*)"pc.R", cmd_pos_show_resultsR);
	cmdAdd((char*)"pc.tuneup", cmd_pc_setTuneup);
	cmdAdd((char*)"pc.a", cmd_driveAngle);
	cmdAdd((char*)"pc.cm", cmd_driveCM);
	cmdAdd((char*)"pc.s", cmd_stopPositioning);
	cmdAdd((char*)"pc.sp", cmd_stopPositioningAtPer);





	// motor current services
	//------------------------------
	cmdAdd((char*)"mot.config", cmd_mot_show_config);
	cmdAdd((char*)"mot.cur", cmd_showMotorCurrent);
	cmdAdd((char*)"mot.curm", cmd_showMowSensor);
	cmdAdd((char*)"mot.scalel", cmd_motorCalculateLScale);
	cmdAdd((char*)"mot.scaler", cmd_motorCalculateRScale);
	cmdAdd((char*)"mot.scalem", cmd_motorCalculateMScale);
	cmdAdd((char*)"mot.mfb", cmd_motor_motorStepSpeed);
	cmdAdd((char*)"mot.mfsp", cmd_motor_motorFSB);
	cmdAdd((char*)"mot.mpfsb", cmd_motor_motorPFSB);
	cmdAdd((char*)"mot.ort", cmd_motor_motorOverRunTest);



	// charge system services
	//------------------------------
	cmdAdd((char*)"charge.config", cmd_charge_show_config);
	cmdAdd((char*)"charge.show", cmd_showChargeSystem);
	cmdAdd((char*)"charge.relay", cmd_setChargeRelay);


	// battery voltage services
	//------------------------------
	cmdAdd((char*)"bat.config", cmd_bat_show_config);
	cmdAdd((char*)"bat.show", cmd_showBattery);

	// Temperature services
	//------------------------------
	cmdAdd((char*)"temp.show", cmd_showTemperature);

	// rain sensor
	cmdAdd((char*)"rain.config", cmd_rain_show_config);
	cmdAdd((char*)"rain.show", cmd_showRain);

	// ADC manager
	//------------------------------
	cmdAdd((char*)"adc.config", cmd_showADCPrint);
	cmdAdd((char*)"adc.samples", cmd_showADCValues);


	//  Perimeter manager
	//------------------------------

	cmdAdd((char*)"per.config", cmd_per_show_config);
	cmdAdd((char*)"per.show", cmd_showPerimeter);
	cmdAdd((char*)"per.max", cmd_showPerimeterMax);
	cmdAdd((char*)"per.adcocl", cmd_showPerAdcOffsetCorrectedL);
	cmdAdd((char*)"per.adcocr", cmd_showPerAdcOffsetCorrectedR);
	cmdAdd((char*)"per.corrl", cmd_showPerCorrelationL);
	cmdAdd((char*)"per.corrr", cmd_showPerCorrelationR);
	cmdAdd((char*)"per.corrsql", cmd_showPerCorrelationSQL);
	cmdAdd((char*)"per.corrsqr", cmd_showPerCorrelationSQR);
	cmdAdd((char*)"per.psnrfl", cmd_showPerPsnrFL);
	cmdAdd((char*)"per.psnrfr", cmd_showPerPsnrFR);
	cmdAdd((char*)"per.resultl", cmd_showPerResultL);
	cmdAdd((char*)"per.resultr", cmd_showPerResultR);
	cmdAdd((char*)"per.fftl", cmd_showFFTL);
	cmdAdd((char*)"per.fftr", cmd_showFFTR);




	cmdAdd((char*)"set.lfki", cmd_setLineFollowerKi);
	cmdAdd((char*)"set.lfkp", cmd_setLineFollowerKp);
	cmdAdd((char*)"set.lfkd", cmd_setLineFollowerKd);
	cmdAdd((char*)"show.lfpid", cmd_showLineFollowerPID);

	cmdAdd((char*)"show.lftune", cmd_showLineFollowerTunings);
	cmdAdd((char*)"show.lf", cmd_showLineFollower);
	cmdAdd((char*)"set.lftuning", cmd_setLineFollowerPIDTuning);
	cmdAdd((char*)"set.lfiterm", cmd_setLineFolloweriTermLimit);

	cmdAdd((char*)"set.spiral", cmd_setCruiseSpiral);

	cmdAdd((char*)"turnto", cmd_turnTo);


	cmdAdd((char*)"bumper.config", cmd_bumper_show_config);
	cmdAdd((char*)"bumper.show", cmd_showBumper);

	cmdAdd((char*)"range.config", cmd_range_show_config);
	cmdAdd((char*)"range.show", cmd_showRange);

	cmdAdd((char*)"gps.config", cmd_gps_show_config);
	cmdAdd((char*)"gps.show", cmd_showGPS);

	cmdAdd((char*)"i2c.scan", cmd_showi2c);









	cmdAdd((char*)"show.mem", cmd_showMem);
	cmdAdd((char*)"show.distance", cmd_showGotoAreaDistance);
	cmdAdd((char*)"show.rot", cmd_showRotate);
	cmdAdd((char*)"show.hist", cmd_showHistory);
	cmdAdd((char*)"bht.tri", cmd_showBHTTriangle);
	cmdAdd((char*)"bht.ln", cmd_showBHTShowLastNode);

	cmdAdd((char*)"show.stat", cmd_showStatistik);

	cmdAdd((char*)"show.bht", cmd_showBHT);



	cmdAdd((char*)"h", cmd_hideShowing);


	// Bluetooth
	cmdAdd((char*)"bt.show", cmd_checkBTModule);
	cmdAdd((char*)"bt.set", cmd_setBTBodule);
	// error handling
	cmdAdd((char*)"error", cmd_printError);
	cmdAdd((char*)"reset", cmd_resetError);

	//xdes1
	cmdAdd((char*)"set.cco", cmd_activateControlCenterOutput);  //enable: $T,1 disable: $T,0

	cmdAdd((char*)"show.dur", cmd_showDuration);


}



