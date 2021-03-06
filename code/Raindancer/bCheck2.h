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
#ifndef BH_CHECK2_H
#define BH_CHECK2_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "BehaviourTree.h"
#include "UseServices.h"



class TCheck2LeftCoilSignal : public Action {
private:
	int state;
public:

	TCheck2LeftCoilSignal() {
		state = 0;
	}

	virtual void onInitialize(Blackboard& bb) {
		if (state == 0) {
			unsigned long time = millis();
			if (time - srvPerSensoren.lastTimeSignalReceivedL > CONF_PER_SIGNAL_LOST_TIME_OONECOIL) {
				state = 1;
				sprintf(errorHandler.msg, "!03,Signal Left Lost\r\n");
				errorHandler.setInfo();
			}
		}
	}

	virtual NodeStatus onUpdate(Blackboard& bb) {


		if (state != 0) {
			unsigned long time = millis();
			if (time - srvPerSensoren.lastTimeSignalReceivedL < 800ul) {
				state = 0;
				sprintf(errorHandler.msg, "!03,Signal Left Found\r\n");
				errorHandler.setInfo();
			}

			if (state == 1) {
				srvMotor.stopAllMotors();
				state = 2;
				return BH_RUNNING;
			}

			if (state == 2) {
				return BH_RUNNING;
			}

		}
		return BH_FAILURE;

	}
};

class TCheck2RightCoilSignal : public Action {
private:
	int state;
public:

	TCheck2RightCoilSignal() {
		state = 0;
	}

	virtual void onInitialize(Blackboard& bb) {
		if (state == 0) {
			unsigned long time = millis();
			if (time - srvPerSensoren.lastTimeSignalReceivedR > CONF_PER_SIGNAL_LOST_TIME_OONECOIL) {
				state = 1;
				sprintf(errorHandler.msg, "!03,Signal Right Lost\r\n");
				errorHandler.setInfo();
			}
		}
	}

	virtual NodeStatus onUpdate(Blackboard& bb) {


		if (state != 0) {
			unsigned long time = millis();
			if (time - srvPerSensoren.lastTimeSignalReceivedR < 800ul) {
				state = 0;
				sprintf(errorHandler.msg, "!03,Signal Right Found\r\n");
				errorHandler.setInfo();
			}

			if (state == 1) {
				srvMotor.stopAllMotors();
				state = 2;
				return BH_RUNNING;
			}

			if (state == 2) {
				return BH_RUNNING;
			}

		}
		return BH_FAILURE;

	}
};



class TCheck2PerSignal : public Action {
private:
	int state;
public:

	TCheck2PerSignal() {
		state = 0;
	}

	virtual void onInitialize(Blackboard& bb) {
		if (state == 0) {
			unsigned long time = millis();
			if ((time - srvPerSensoren.lastTimeSignalReceivedL > CONF_PER_SIGNAL_LOST_TIME) &&
				(time - srvPerSensoren.lastTimeSignalReceivedR > CONF_PER_SIGNAL_LOST_TIME)/* &&
				(time-bb.perimeterSensoren.lastTimeSignalReceivedB > 1000)*/) {

				state = 1;
				sprintf(errorHandler.msg, "!03,Signal Lost\r\n");
				errorHandler.setInfo();
			}
		}
	}

	virtual NodeStatus onUpdate(Blackboard& bb) {


		if (state != 0) {
			unsigned long time = millis();

			if ((time - srvPerSensoren.lastTimeSignalReceivedL < 800ul) &&
				(time - srvPerSensoren.lastTimeSignalReceivedR < 800ul)/* &&
				(time-bb.perimeterSensoren.lastTimeSignalReceivedB  < 800)*/) {
				state = 0;
				sprintf(errorHandler.msg, "!03,Signal Found\r\n");
				errorHandler.setInfo();
			}



			if (state == 1) {
				srvMotor.stopAllMotors();
				state = 2;
				return BH_RUNNING;
			}

			if (state == 2) {
				return BH_RUNNING;
			}

		}
		return BH_FAILURE;

	}
};



class TCheck2AllCoilsOutside
	: public Action {
private:
	int state;
	unsigned long startTime;
public:

	TCheck2AllCoilsOutside() {
		state = 0;
		startTime = 0;
	}

	virtual void onInitialize(Blackboard& bb) {
		if (state == 0) {
			if (srvPerSensoren.isLeftOutside() && srvPerSensoren.isRightOutside() /*&& bb.perimeterSensoren.isBackOutside()*/) {
				startTime = millis();
				state = 1;
				/*
		  sprintf(errorHandler.msg,"!03,All Coils Outside\r\n");
		  errorHandler.setInfo();
		  sprintf(errorHandler.msg,"!03,ML: %d MR: %d\r\n",bb.perimeterSensoren.magnetudeL , bb.perimeterSensoren.magnetudeR);
		  errorHandler.setInfo();
				*/
			}
		}
	}

	virtual NodeStatus onUpdate(Blackboard& bb) {
		if (state == 1 || state == 2) {
			if (srvPerSensoren.isLeftInside() || srvPerSensoren.isRightInside() /* || bb.perimeterSensoren.isBackInside()*/) {
				//sprintf(errorHandler.msg,"!03,One Coils Inside Again\r\n");
				//errorHandler.setInfo();
				state = 0;
			}
		}

		if (state == 1) {
			// 1 Sekunde warten
			if (millis() - startTime > 1000ul) {
				srvMotor.stopAllMotors();
				state = 2;
				sprintf(errorHandler.msg, "!03,All Coils Outside\r\n");
				errorHandler.setInfo();
				return BH_RUNNING;
			}
			return BH_FAILURE;
		}

		if (state == 2) {
			return BH_RUNNING;
		}

		return BH_FAILURE;

	}
};



class TCheck2CoilSignalAreaX : public Action {
private:
	int state;
	unsigned long waittime;
public:

	TCheck2CoilSignalAreaX() {
		state = 0;
	}

	virtual void onInitialize(Blackboard& bb) {
		if (state == 0 && bb.flagGotoAreaXFirstCall == true) {
			waittime = millis();
			state = 1;
			bb.flagGotoAreaXFirstCall = false;  // bb.flagGotoAreaXFirstCall is set to true in void Blackboard::setBehaviour(enuBehaviour b)
		}
	}


	virtual NodeStatus onUpdate(Blackboard& bb) {


		if (state != 0) {
			unsigned long time = millis();

			if (state == 1) {

				// 2 Sekunden auf Signal warten da gerade eingeschaltet wurde
				if (time - waittime < 2000) {
					return BH_RUNNING;
				}

				// Is right coil outside then rotate 5 degree right further to be sure to really be outside
				if (srvPerSensoren.isRightOutside()) {
					sprintf(errorHandler.msg, "!03,Signal Right Found SignalAreaX1 Outside rotate 5 degree further\r\n");
					errorHandler.setInfo();
					//bb.cruiseSpeed = bb.CRUISE_SPEED_LOW;
					//bb.driveDirection = DD_ROTATECW;
					srvMotor.turnTo(5, bb.CRUISE_SPEED_LOW);
					state = 2;
					return BH_RUNNING;
				}
				// Is right coil inside then rotate 15 degree right further to be sure to really be outside
				else if (srvPerSensoren.isRightInside()) {
					sprintf(errorHandler.msg, "!03,Signal Right Found SignalAreaX1 Inside rotate 15 degree further\r\n");
					errorHandler.setInfo();
					//bb.cruiseSpeed = bb.CRUISE_SPEED_LOW;
					//bb.driveDirection = DD_ROTATECW;
					srvMotor.turnTo(10, bb.CRUISE_SPEED_LOW);
					state = 2;
					return BH_RUNNING;
				}

				// Rotate 10 degree CW because signal is 0
				sprintf(errorHandler.msg, "!03,Signal Right NOT Found SignalAreaX1  rotate 10 degree further\r\n");
				errorHandler.setInfo();
				//bb.cruiseSpeed = bb.CRUISE_SPEED_LOW;
				//bb.driveDirection = DD_ROTATECW;
				srvMotor.turnTo(10, bb.CRUISE_SPEED_LOW);
				state = 2;
				return BH_RUNNING;
			}



			if (state == 2) {

				if (getTimeInNode() > 10000) {
					errorHandler.setError(F("!03,TCheck2CoilSignalAreaX  too long in state\r\n"));
				}

				if (srvMotor.isPositionReached()) {
					state = 3;
					bb.flagGotoAreaXFirstCall = false;
					sprintf(errorHandler.msg, "!03,TCheck2CoilSignalAreaX rotated\r\n");
					errorHandler.setInfo();
					return BH_RUNNING;
				}
				return BH_RUNNING;
			}


			if (state == 3) {

				// 1 Sekunden auf Signal warten da gerade eingeschaltet wurde
				if (time - waittime < 1000) {
					return BH_RUNNING;
				}

				// Is left coil outside then rotate -10 degree left to be sure to really be inside
				if (srvPerSensoren.isLeftOutside()) {
					sprintf(errorHandler.msg, "!03,Signal Left Found SignalAreaX1 Outside rotate 10 degree to left\r\n");
					errorHandler.setInfo();
					//bb.cruiseSpeed = bb.CRUISE_SPEED_LOW;
					//bb.driveDirection = DD_ROTATECW;
					srvMotor.turnTo(-10, bb.CRUISE_SPEED_LOW);
					state = 4;
					return BH_RUNNING;
				}

				state = 4;
				return BH_RUNNING;
			}

			if (state == 4) {

				if (getTimeInNode() > 10000) {
					errorHandler.setError(F("!03,TCheck2CoilSignalAreaX  too long in state\r\n"));
				}

				if (srvMotor.isPositionReached()) {
					state = 0;
					bb.flagGotoAreaXFirstCall = false;
					sprintf(errorHandler.msg, "!03,TCheck2CoilSignalAreaX rotated\r\n");
					errorHandler.setInfo();
					return BH_FAILURE;
				}
				return BH_FAILURE;
			}

		}
		return BH_FAILURE;

	}
};
#endif
