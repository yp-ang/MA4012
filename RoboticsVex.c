//EVERYTHING DISTANCE RELATED IN mm!
//Motor Definitions

#define LEFT_MOT_PORT port3
#define RIGHT_MOT_PORT port2
#define DOOR_MOT_PORT port4
#define DOOR_MOT_SPEED 50
#define SPEED_OFFSET 0.0
#define SLOW_SPEED 20.0
#define OP_SPEED 127.0
//Limit Switch Definitions

#define DOOR_LSWITCH_PORT dgtl2
#define BALL_LSWITCH_PORT dgtl1
//Distance IR Sensor Definitions

#define BOT_DIST_IR_TYPE ORANGE_YELLOW
#define BOT_DIST_IR_PORT in1
#define TOP_DIST_IR_TYPE ORANGE_GREEN
#define TOP_DIST_IR_PORT in2
#define DOOR_DIST_IR_TYPE ORANGE_BLUE
#define DOOR_DIST_IR_PORT in3
#define DOOR_DIST_IR_THRESHOLD 130.0 //in mm
#define DIST_IR_MAX_DIFF 68.0 //in mm
#define DIST_IR_MIN_DIFF 30.0 //in mm

//Reflective Sensor Definitions
#define LEFT_REFLECTIVE_IR_PORT in4
#define RIGHT_REFLECTIVE_IR_PORT in5
#define LEFT_REFLECTIVE_IR_THRESHOLD 1100
#define RIGHT_REFLECTIVE_IR_THRESHOLD 1100

//Compass Definitions
#define NORTH_PORT dgtl3
#define EAST_PORT dgtl4
#define WEST_PORT dgtl5
#define SOUTH_PORT dgtl6

//Utils
#define WIFI_DEBUGGING 0
#define UART_PORT uartOne


enum ir_sensor{ORANGE_PINK, ORANGE_GREEN, ORANGE_YELLOW, ORANGE_BLUE, NUMBER_OF_IR_SENSORS};
enum direction{STOP, REVERSE, STRAIGHT, CLOCKWISE, CCLOCKWISE};
enum bearing{NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, ERROR};
enum mode{MOVE_TO_CENTER, SEARCH_FOR_BALL, MOVE_TO_BALL, RETURN_BALL};
enum return_ball_seq{ALIGN_BEARING, RETURN_TO_BASE, DEPOSIT_BALL};
int ir_sensor_max_value[(int) NUMBER_OF_IR_SENSORS] = {1853, 1828, 1884, 1978};
int ir_sensor_min_value[(int) NUMBER_OF_IR_SENSORS] = {539, 384, 386, 307};

/*
MA4012
Group Project
YuPin edit 22/2/2022

*/
mode machine_mode = MOVE_TO_CENTER;
mode machine_return_ball_seq = ALIGN_BEARING;
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

bool check_within_range(float value, float min, float max)
{
	return value <= min || value >= max ?  false : true;
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

void movement(direction directionMode, int speed){
	switch (directionMode)
	{
	case STOP:
		motor[LEFT_MOT_PORT] = 0;
		motor[RIGHT_MOT_PORT] = 0;
		break;
	case STRAIGHT:
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = speed;
		break;
	case REVERSE:
		motor[LEFT_MOT_PORT] = speed;
		motor[RIGHT_MOT_PORT] = -speed;
		break;
	case CLOCKWISE:
		motor[LEFT_MOT_PORT] = speed;
		motor[RIGHT_MOT_PORT] = speed;
		break;
	case CCLOCKWISE:
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = -speed;
		break;
	default:
		motor[LEFT_MOT_PORT] = 0;
		motor[RIGHT_MOT_PORT] = 0;
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

bool detect_ball_deposit()
{
	if (SensorValue(DOOR_LSWITCH_PORT) == 1)
	{
		return (SensorValue(DOOR_DIST_IR_PORT) > DOOR_DIST_IR_THRESHOLD);
	}
	return false;
}

bool detect_ball_field(){
	float bottom_ir_distance = convert_ir_reading_to_distance(BOT_DIST_IR_TYPE, SensorValue(BOT_DIST_IR_PORT));
	float top_ir_distance = convert_ir_reading_to_distance(TOP_DIST_IR_TYPE, SensorValue(TOP_DIST_IR_TYPE));
	if (check_within_range(bottom_ir_distance, ir_sensor_min_value[BOT_DIST_IR_TYPE], ir_sensor_max_value[BOT_DIST_IR_TYPE]))
	{
		//Something detected at bottom sensor
		if (check_within_range(top_ir_distance, ir_sensor_min_value[TOP_DIST_IR_TYPE], ir_sensor_max_value[TOP_DIST_IR_TYPE]))
		{
			//Something detected at top sensor
			if (top_ir_distance - bottom_ir_distance > DIST_IR_MIN_DIFF)
			{
				//ball detetcted, robot detected
			}
		//bottom sensor detect, top sensor no detect, only ball
		}
	}
	// nothing detected, if bottom did not detect anything, nothing should be at the top
	return false;
}

bool detect_ball_collector(){
	return (SensorValue(BALL_LSWITCH_PORT) == 1);
}

enum bearing get_heading()
{
	byte heading = 0x00;
	heading & SensorValue(NORTH_PORT) & (SensorValue(EAST_PORT) << 1) & (SensorValue(SOUTH_PORT) << 2) & (SensorValue(WEST_PORT) << 3);
	switch (heading)
{	case 1:
		return NORTH;
		break;
	case  3:
		return NORTHEAST;
		break;
	case 2:
		return EAST;
		break;
	case 6:
		return SOUTHEAST;
		break;
	case 4:
		return SOUTH;
		break;
	case 12:
		return SOUTHWEST;
		break;
	case 8:
		return WEST;
		break;
	case 9:
		return NORTHWEST;
		break;
	default:
		return ERROR;
	}
}

void move_to_ball(/*distance*/){
	//
}

void start_process(){
	//movement to middle of field
}


void align_orientation(){
	//compass value
}

void move_to_collection(){
	movement(REVERSE, slow_speed);
	//release ball
	start_process();
}

void keep_door_closed()
{
	motor[DOOR_MOT_PORT] = DOOR_MOT_SPEED * (SensorValue[DOOR_LSWITCH_PORT] == 0);
}

void run_machine()
{
	switch (machine_mode)
	{
		case MOVE_TO_CENTER:
		break;
		case SEARCH_FOR_BALL:
		break;
		case MOVE_TO_BALL:
		break;
		case RETURN_BALL:
			switch (machine_return_ball_seq)
			{
				case ALIGN_BEARING:
				break;
				case RETURN_TO_BASE:
					movement(REVERSE, OP_SPEED);
					if (detect_ball_deposit())
					{
						machine_return_ball_seq = DEPOSIT_BALL;
					}
				break;
				case DEPOSIT_BALL:
					motor[DOOR_MOT_PORT] = DOOR_MOT_SPEED;
					delay(300);
					machine_mode = MOVE_TO_CENTER;
				break;
			}
		break;
		default:
		break;
	}
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

		detect_boundary(SensorValue[LEFT_REFLECTIVE_IR_PORT],SensorValue[RIGHT_REFLECTIVE_IR_PORT]);

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
