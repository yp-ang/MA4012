#pragma config(Sensor, dgtl7, Floor_frontLeft, sensorDigitalIn)
#pragma config(Sensor, dgtl8, Floor_frontRight, sensorDigitalIn)

#define left_motor port3
#define right_motor port2
#define speedOffset 0

enum direction{STOP, REVERSE, STRAIGHT, CLOCKWISE, CCLOCKWISE};
enum bearing{NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, ERROR};

/*
MA4012
Group Project
YuPin edit 22/2/2022

*/


//port2 motor goes to 9/10
//port3 motor goes to 11/12
const int slow_speed = 20;
//const int speedOffset = 5;


void start_process(){
	//movement to middle of field
}

void movement(direction directionMode, int speed){
	switch (directionMode)
	{
	case STOP:
		motor[left_motor] = 0;
		motor[right_motor] = 0;
		break;
	case STRAIGHT:
		motor[left_motor] = -speed;
		motor[right_motor] = speed;
		break;
	case REVERSE:
		motor[left_motor] = speed;
		motor[right_motor] = -speed;
		break;
	case CLOCKWISE:
		motor[left_motor] = speed;
		motor[right_motor] = speed;
		break;
	case CCLOCKWISE:
		motor[left_motor] = -speed;
		motor[right_motor] = -speed;
		break;
	default:
		motor[left_motor] = 0;
		motor[right_motor] = 0;
		break;
	}
}

void movement_t(direction directionMode, int speed, int duration){
	//duration in miliseconds
	clearTimer(T1);
	while(true){
		if (time1[T1] < duration){
			movement(directionMode, speed);
		}
	}
}

void detect_boundary(int left_f, int right_f){
	if(left_f==0 && right_f==0){	//detect both front sensors -> turn right full round
		movement_t(-1,slow_speed,1000);
		movement_t(3,slow_speed,4000);
		movement(0,0);
	}
	else if (left_f==0){					//detect left sensor -> turn left
		movement_t(3,slow_speed,2000);
		movement(0,0);
	}
	else if (right_f==0){					//detect right sensor -> turn right
		movement_t(2,slow_speed,2000);
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
	movement(REVERSE, slow_speed);
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
