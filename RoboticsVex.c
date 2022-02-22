#pragma config(Sensor, dgtl7, Floor_frontLeft, sensorDigitalIn)
#pragma config(Sensor, dgtl8, Floor_frontRight, sensorDigitalIn)


/*
	MA4012
	Group Project

*/


//port2 motor goes to 9/10
//port3 motor goes to 11/12
const int slow_speed = 20;
//const int speedOffset = 5;






void start_process(){
	//movement to middle of field
	}
void movement(int directionMode, int speed){
	// 1 =	forward
	//-1 =	reverse
	// 2 =	rotate left
	// 3 = 	rotate right
	// 0 =	stop
	if (directionMode == 1){					//forward
		motor[port2] = -speed;
		motor[port3] = speed;
		}
	else if (directionMode == -1){		//reverse
		motor[port2] = speed;
		motor[port3] = -speed;
		}
	else if (directionMode == 2){			//left
		motor[port2] = -speed;
		motor[port3] = -speed;
		}
	else if (directionMode == 3){			//right
		motor[port2] = speed;
		motor[port3] = speed;
		}
	else if (directionMode == 0){			//stop
		motor[port2] = 0;
		motor[port3] = 0;
		}
	else{															//stop if directionMode is unrecognised
		motor[port2] = 0;
		motor[port3] = 0;
		}
	}


void detect_boundary(int left_f, int right_f){
	clearTimer(T1);
	if(left_f==0 && right_f==0){	//detect both front sensors -> turn right full round
		while (time1[T1] < 3000){
			movement(3,slow_speed);
			}
		movement(0,0);
		}
	else if (left_f==0){					//detect left sensor -> turn left
		while (time1[T1] < 2000){
			movement(3,slow_speed);
			}
		movement(0,0);
		}
	else if (right_f==0){					//detect right sensor -> turn right
		while (time1[T1] < 2000){
			movement(2,slow_speed);
			}
		movement(0,0);
		}
	}

bool detect_ball_field(){
	//sensor
	//return distance value
	}

void move_to_ball(/*distance*/){
	//
	}

bool detect_ball_collector(){
	}

void align_orientation(){
	//compass value
	}

void move_to_collection(){
	movement(-1, slow_speed);
	//release ball
	start_process();
	}

//////////////////////////////////////////////////////////
task main()
{
	start_process();
	while (true)
	{
		movement(1,slow_speed);

		detect_boundary(SensorValue[Floor_frontLeft],SensorValue[Floor_frontRight]);

		if (detect_ball_field()==true)
		{
			move_to_ball(); //move towards possible directions in small step
		}

		if (detect_ball_collector()==true)
		{
			align_orientation();
			move_to_collection();
		}

	}
}
