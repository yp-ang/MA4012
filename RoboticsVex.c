#pragma config(Sensor, dgtl7, Floor_frontLeft, sensorDigitalIn)
#pragma config(Sensor, dgtl8, Floor_frontRight, sensorDigitalIn)


//Motor Definitions

#define LEFT_MOTOR port3
#define RIGHT_MOTOR port2
#define SPEED_OFFSET 0.0
#define SLOW_SPEED 20.0
//Limit Switch Definitions

#define DOOR_LSWITCH_PORT dgtl2
#define BALL_LSWITCH_PORT dgtl1
//Distance IR Sensor Definitions

#define LEFT_DISTANCE_IR_TYPE ORANGE_YELLOW
#define LEFT_DISTANCE_IR_PORT in1
#define RIGHT_DISTANCE_IR_TYPE ORANGE_GREEN
#define RIGHT_DISTANCE_IR_PORT in2
#define DOOR_DISTANCE_IR_TYPE ORANGE_BLUE
#define DOOR_DISTANCE_IR_PORT in3

//Reflective Sensor Definitions
#define LEFT_REFLECTIVE_IR_PORT in4
#define RIGHT_REFLECTIVE_IR_PORT in5

//Utils
#define WIFI_DEBUGGING 0
#define UART_PORT uartOne


enum ir_sensor{ORANGE_PINK, ORANGE_GREEN, ORANGE_YELLOW, ORANGE_BLUE};
enum direction{STOP, REVERSE, STRAIGHT, CLOCKWISE, CCLOCKWISE};
enum bearing{NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, ERROR};

/*
MA4012
Group Project
YuPin edit 22/2/2022

*/

const int slow_speed = 20;

void send_debug_msg(char* msg, int size)
{
#ifdef WIFI_DEBUGGING
	for (int i = 0; i < size; i ++)
	{
		sendChar(UART_PORT, msg[i]);
	}
#endif
}

float convert_ir_reading_to_distance(ir_sensor sensor, int analog)
{
	switch(sensor)
	{
	case ORANGE_BLUE:
		return 2316.4 * pow(analog, -0.998);
		break;
	case ORANGE_GREEN:
		return 49865 * pow(analog, -1.416);
		break;
	case ORANGE_PINK:
		return 166922 * pow(analog, -1.604);
		break;
	case ORANGE_YELLOW:
		return 44924 * pow(analog, -1.394);
		break;
	default:
		return -1;
	}
}

void start_process(){
	//movement to middle of field
}

void movement(direction directionMode, int speed){
	switch (directionMode)
	{
	case STOP:
		motor[LEFT_MOTOR] = 0;
		motor[RIGHT_MOTOR] = 0;
		break;
	case STRAIGHT:
		motor[LEFT_MOTOR] = -speed;
		motor[RIGHT_MOTOR] = speed;
		break;
	case REVERSE:
		motor[LEFT_MOTOR] = speed;
		motor[RIGHT_MOTOR] = -speed;
		break;
	case CLOCKWISE:
		motor[LEFT_MOTOR] = speed;
		motor[RIGHT_MOTOR] = speed;
		break;
	case CCLOCKWISE:
		motor[LEFT_MOTOR] = -speed;
		motor[RIGHT_MOTOR] = -speed;
		break;
	default:
		motor[LEFT_MOTOR] = 0;
		motor[RIGHT_MOTOR] = 0;
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
	return (SensorValue(BALL_LSWITCH_PORT) == 1);
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
#ifdef WIFI_DEBUGGING
	setBaudRate(UART_PORT, baudRate9600);
#endif
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
