// EVERYTHING DISTANCE RELATED IN mm!
// Motor Definitions

#define LEFT_MOT_PORT port3
#define RIGHT_MOT_PORT port5
#define DOOR_MOT_PORT port4
#define ROLLER_MOT_PORT port2
#define DOOR_MOT_SPEED 50
#define SPEED_OFFSET 0.0
#define SLOW_SPEED 20.0
#define OP_SPEED 127.0

// Limit Switch Definitions
#define DOOR_LSWITCH_PORT dgtl11 // switch pressed down is 1
#define BALL_LSWITCH_PORT dgtl12 //switch pressed down is 1
#define BACK_L_LSWITCH_PORT dgtl9 //switch pressed down is 0
#define BACK_R_LSWITCH_PORT dgtl10 //switch pressed down is 0

// Start Switch Definitions
#define START_SWITCH_PORT dgtl8 // switch on is 0

// Distance IR Sensor Definitions
#define RIGHT_DIST_IR_TYPE ORANGE_GREEN
#define TOP_DIST_IR_PORT in1
#define LEFT_DIST_IR_TYPE ORANGE_YELLOW
#define BTM_DIST_IR_PORT in2
#define CENTER_DIST_IR_TYPE ORANGE_BLUE
#define CENTER_DIST_IR_PORT in3
#define DOOR_DIST_IR_THRESHOLD 130.0 //in mm
#define DIST_IR_MAX_DIFF 68.0 //in mm
#define DIST_IR_MIN_DIFF 150.0 //in mm
#define LEFT_DIST_IR_OFFSET 70.0//

//Reflective Sensor Definitions
#define FRT_LFT_REFL_IR_PORT in4
#define FRT_RGT_REFL_IR_PORT in5
#define FRT_LFT_REFL_IR_THRESHOLD 1100
#define FRT_RGT_REFL_IR_THRESHOLD 1100
#define BCK_LFT_REFL_IR_PORT in7
#define BCK_RGT_REFL_IR_PORT in8
#define BCK_LFT_REFL_IR_THRESHOLD 1100
#define BCK_RGT_REFL_IR_THRESHOLD 1100

//Compass Definitions
#define NORTH_PORT dgtl3
#define EAST_PORT dgtl4
#define SOUTH_PORT dgtl5
#define WEST_PORT dgtl6
#define COMPASS_SWING_DURATION 3000

// Utils
#define WIFI_DEBUGGING 0

//Only allow CALIBRATION or TESTING one at a time
//#define CALIBRATION 0
//#define TESTING 0
#define UART_PORT uartOne
#define BALL_DIAMETER 63.0
#define ANGLE_SLICE 0.16
#define TIMER T1

float lowest_reading = 9999;
//float lowest_reading_1 = 9999;

enum ir_sensor
{
	ORANGE_PINK,
	ORANGE_GREEN,
	ORANGE_YELLOW,
	ORANGE_BLUE,
	NUMBER_OF_IR_SENSORS
};
enum direction
{
	STOP,
	REVERSE,
	STRAIGHT,
	CLOCKWISE,
	CCLOCKWISE
};
enum bearing
{
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST,
	ERROR
};
enum mode
{
	MOVE_TO_CENTER,
	SEARCH_FOR_BALL,
	MOVE_TO_BALL,
	RETURN_THE_BALL
};
enum search_ball_seq
{
	ROTATE,
	MOVE_TO_NEW_POSITION
};

enum move_to_ball_seq
{
	OFFSET,
	MOVE_TOWARDS_BALL
};
enum return_ball_seq
{
	ALIGN_BEARING,
	RETURN_TO_BASE,
	MOVE_FORWARD,
	TURN_RIGHT,
	MOVE_FORWARD_RIGHT,
	DEPOSIT_BALL
};

/*Thresholds are at 10-60cm as the anything above 60cm for the 10-80cm sensors is just too bad
For 4-30cm, threshold taken at 30cm
*/
int ir_sensor_max_value[(int) NUMBER_OF_IR_SENSORS] = {2000, 2000, 2000, 1978};
int ir_sensor_min_value[(int) NUMBER_OF_IR_SENSORS] = {508, 472, 476, 307};

mode machine_mode = MOVE_TO_CENTER;
mode machine_return_ball_seq = ALIGN_BEARING;
search_ball_seq search_ball = ROTATE;
move_to_ball_seq move_ball = OFFSET;
return_ball_seq return_ball = ALIGN_BEARING;
bearing machine_heading = ERROR;
direction machine_direction = STOP;
int machine_speed = 0;

const int slow_speed = 20;

void send_debug_msg(char *msg, int size)
{
#ifdef WIFI_DEBUGGING
	for (int i = 0; i < size; i++)
	{
		sendChar(UART_PORT, msg[i]);
	}
#endif
}

bool check_within_range(float value, float min, float max)
{
	return value <= min || value >= max ? false : true;
}

float convert_ir_reading_to_distance(ir_sensor sensor, int analog)
{
	analog /= 4;
	switch (sensor)
	{
	case ORANGE_BLUE:
		return 2316.4 * pow(analog, -0.998) * 10;
		break;
	case ORANGE_GREEN:
		return 49865 * pow(analog, -1.416) * 10;
		break;
	case ORANGE_PINK:
		return 166922 * pow(analog, -1.604) * 10;
		break;
	case ORANGE_YELLOW:
		return 44924 * pow(analog, -1.394) * 10;
		break;
	default:
		return -1;
	}
}

void movement(direction directionMode, int speed = 0)
{
	int speed1 = abs(speed * 1.02);
	switch (directionMode)
	{
	case STOP:
		motor[LEFT_MOT_PORT] = 0;
		motor[RIGHT_MOT_PORT] = 0;
		break;
	case REVERSE:
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = speed1;
		break;
	case STRAIGHT:
		motor[LEFT_MOT_PORT] = speed;
		motor[RIGHT_MOT_PORT] = -speed1;
		break;
	case CCLOCKWISE:
		motor[LEFT_MOT_PORT] = speed;
		motor[RIGHT_MOT_PORT] = speed1;
		break;
	case CLOCKWISE:
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = -speed1;
		break;
	default:
		motor[LEFT_MOT_PORT] = 0;
		motor[RIGHT_MOT_PORT] = 0;
		break;
	}
}

bool detect_ball_deposit()
{
	return SensorValue(BACK_L_LSWITCH_PORT) == 0 && SensorValue(BACK_R_LSWITCH_PORT) == 0;
}

int take_average(tSensors sensor_port, int readings)
{
	float average = 0;
	for (int i = 0; i < readings; i++)
	{
		average += SensorValue(sensor_port);
	}
	return average / readings;
}

float clamp(float d, float min, float max)
{
	const float t = d < min ? min : d;
	return t > max ? max : t;
}

bool detect_ballv4()
{
	int left_analog = take_average(BTM_DIST_IR_PORT, 1);
	float left_distance = convert_ir_reading_to_distance(LEFT_DIST_IR_TYPE, left_analog);
	bool left_curr_in_range =
	check_within_range(left_distance, 0, 450);
	lowest_reading = left_distance;
	int right_analog = take_average(TOP_DIST_IR_PORT, 1);
	float right_distance = convert_ir_reading_to_distance(RIGHT_DIST_IR_TYPE, right_analog);
	bool right_curr_in_range =
	check_within_range(right_distance, 0, 450);

#ifdef WIFI_DEBUGGING
	char buf3[8] = "Value: ";
	char buf4[8] = "\n";
	send_debug_msg(buf3, sizeof(buf3));
	char buf[64];
	snprintf(buf, sizeof(buf), "%f, %f", left_distance, right_distance);
	send_debug_msg(buf, sizeof(buf));
	send_debug_msg(buf4, sizeof(buf4));
#endif
	if (!left_curr_in_range && !right_curr_in_range)
	{
#ifdef WIFI_DEBUGGING
		char buf[64];
		snprintf(buf, sizeof(buf), "left and right not in range", left_distance, right_distance);
		send_debug_msg(buf, sizeof(buf));
#endif
		return false;
	}
	if ((left_curr_in_range) && !right_curr_in_range)
	{
		if (!right_curr_in_range)
		{
#ifdef WIFI_DEBUGGING
			char buf[64];
			snprintf(buf, sizeof(buf), "left in range, right not", left_distance, right_distance);
			send_debug_msg(buf, sizeof(buf));
#endif
			return true;
		}
		if ((right_distance - left_distance) <= 90)
		{
#ifdef WIFI_DEBUGGING
			char buf[64];
			snprintf(buf, sizeof(buf), "left and right small diff", left_distance, right_distance);
			send_debug_msg(buf, sizeof(buf));
#endif
			return true;
		}
	}
#ifdef WIFI_DEBUGGING
	snprintf(buf, sizeof(buf), "left and right big diff", left_distance, right_distance);
	send_debug_msg(buf, sizeof(buf));
#endif
	return false;
}

//bool detect_ballv3()
//{
//	const int array_size = 10;
//	static float dist_array[array_size] = {1800.0, 1800.0, 1800.0, 1800.0,
//		1800.0, 1800.0, 1800.0, 1800.0, 1800.0, 1800.0};

//	int lowest_reading_index = 0;
//	int count = 0;
//	int left_analog = take_average(BTM_DIST_IR_PORT, 1);
//	bool curr_in_range =
//	check_within_range(left_analog, ir_sensor_min_value[LEFT_DIST_IR_TYPE], ir_sensor_max_value[LEFT_DIST_IR_TYPE]);
//	float ir_distance = convert_ir_reading_to_distance(LEFT_DIST_IR_TYPE, left_analog);
//	lowest_reading = ir_distance;
//	for (int i = 0; i < array_size - 1; i++)
//	{
//		dist_array[i] = dist_array[i+1];
//	}
//	dist_array[array_size - 1] = ir_distance;
//#ifdef WIFI_DEBUGGING
//	char buf1[8] = "Start: ";
//	char buf2[8] = "\n";
//	send_debug_msg(buf1, sizeof(buf1));
//	for (int i = 0; i < array_size; i++)
//	{
//		char buf[16];
//		snprintf(buf, sizeof(buf), "%d, ", (int)dist_array[i]);
//		send_debug_msg(buf, sizeof(buf));
//	}
//	send_debug_msg(buf2, sizeof(buf2));
//#endif
//	for (int i = 0; i < array_size; i++)
//	{
//		if (dist_array[i] < lowest_reading)
//		{
//			lowest_reading = dist_array[i];
//			lowest_reading_index = i;
//		}
//	}

//	if (lowest_reading == 0  ||
//		check_within_range(lowest_reading, ir_sensor_min_value[LEFT_DIST_IR_TYPE],
//	ir_sensor_max_value[LEFT_DIST_IR_TYPE]))
//	{
//		return false;
//	}

//	float ratio_thresh = -0.005 * lowest_reading + 3.8;

//	//right search
//	bool nir_found;
//	bool nir_1=false;
//	bool nir_2=false;
//	for (int i = lowest_reading_index; i < array_size; i++)
//	{
//		float ratio;
//		ratio = dist_array[i]/lowest_reading;
//		if (ratio <= ratio_thresh)
//		{
//			count = count + 1;
//		}
//		else
//		{
//			nir_1 = true;
//			goto end;

//		}
//	}
//end:
//	for (int i = lowest_reading_index; i >= 0; i--)
//	{
//		float ratio;
//		ratio = dist_array[i]/lowest_reading;
//		if (ratio <= ratio_thresh)
//		{
//			count = count + 1;
//		}
//		else
//		{
//			nir_2 = true;
//			goto end2;

//		}
//	}
//end2:
//	nir_found = nir_1 && nir_2;
//	//int count_thresh = -0.0079 * lowest_reading + 5.6499;
//	//int count_thresh = 0.00002 * pow(lowest_reading, 2) -0.0259 * lowest_reading + 9.373;
//	int count_thresh_low = 0.000005 * pow(lowest_reading, 2) - 0.0102 * lowest_reading + 4.995
//	int count_thresh = 0.00003 * pow(lowest_reading, 2) -0.0332* lowest_reading + 11.435;
//	count = count-1;
//#ifdef WIFI_DEBUGGING
//	char buf3[8] = "Value: ";
//	char buf4[8] = "\n";
//	send_debug_msg(buf3, sizeof(buf1));
//	char buf[64];
//	snprintf(buf, sizeof(buf), "%f, %d, %d, %f ", ratio_thresh, count_thresh, count, lowest_reading);
//	send_debug_msg(buf, sizeof(buf));
//	send_debug_msg(buf4, sizeof(buf2));
//#endif

//	if (count >= (count_thresh_low) && count <= (count_thresh) && count != 0 && nir_found)
//	{
//#ifdef WIFI_DEBUGGING
//		char buf[32];
//		snprintf(buf, sizeof(buf), "DETECTED!!!!!!!!!!!");
//		send_debug_msg(buf, sizeof(buf));
//#endif
//		for (int i = 0; i < array_size; i++)
//		{
//			dist_array[i] = 1800.0;
//		}
//		return true;

//	}
//	return false;
//}

//int correction_time(float reading)
//{

//	return reading / 3 + 200;
//}

bool detect_ball_collector()
{
	return (SensorValue(BALL_LSWITCH_PORT) == 1);
}

enum bearing get_heading()
{
	byte heading = 0x00;
	heading &SensorValue(NORTH_PORT) & (SensorValue(EAST_PORT) << 1) & (SensorValue(SOUTH_PORT) << 2) & (SensorValue(WEST_PORT) << 3);
	switch (heading)
	{
	case 1:
		return NORTH;
		break;
	case 3:
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

void move_to_ball(/*distance*/)
{
	//
}

void keep_door_closed()
{
	motor[DOOR_MOT_PORT] = DOOR_MOT_SPEED * (SensorValue[DOOR_LSWITCH_PORT] == 0);
}

void goto_movetocenter()
{
	machine_direction = STRAIGHT;
	machine_speed = 127;
	motor[ROLLER_MOT_PORT] = 127;
	clearTimer(TIMER);
}

void goto_searchforball()
{
	send_debug_msg("Going to SEARCH_FOR_BALL\n", 32);
	machine_mode = SEARCH_FOR_BALL;
	search_ball = ROTATE;
	machine_direction = CLOCKWISE;
	machine_speed = 24;
	motor[ROLLER_MOT_PORT] = 127;
	clearTimer(TIMER);
}

bool determine_rotation_direction()
{
	bearing current_heading = get_heading();
	if (current_heading == machine_heading)
	{
		machine_direction = REVERSE;
		machine_speed = 127;
		return_ball = RETURN_TO_BASE;
		clearTimer(TIMER);
		return true;
	}
	else
	{
		if ((((int)current_heading + (8 - (int)machine_heading)) % 8) >= 4)
		{
			machine_direction = CLOCKWISE;
			machine_speed = 127;
			return_ball = ALIGN_BEARING;
			clearTimer(TIMER);
		}
		else
		{
			machine_direction = CCLOCKWISE;
			machine_speed = 127;
			return_ball = ALIGN_BEARING;
			clearTimer(TIMER);
		}
	}
	return false;
}

void goto_returnball()
{
	send_debug_msg("Going to RETURN_BALL\n", 32);
	machine_mode = RETURN_THE_BALL;
	determine_rotation_direction();
}

void run_machine()
{
#ifdef WIFI_DEBUGGING
	char buf[16];
	snprintf(buf, sizeof(buf), "Time: %ld\n", time1[TIMER]);
	send_debug_msg(buf, sizeof(buf));
#endif
	switch (machine_mode)
	{
	case MOVE_TO_CENTER:
		if (time1[TIMER] > 3000)
		{
			goto_searchforball();
		}
		if (detect_ball_collector())
		{
			goto_returnball();
			break;
		}
		break;
	case SEARCH_FOR_BALL:
		motor[ROLLER_MOT_PORT] = 127;
		if (detect_ball_collector())
		{
			goto_returnball();
			break;
		}
		if (detect_ballv4())
		{
			//goto
			//machine_mode = MOVE_TO_BALL;
			//clearTimer(TIMER);
		}
		switch (search_ball)
		{
		case ROTATE:
			if (time1[TIMER] > 4000)
			{
				send_debug_msg("Changing to MOVE_TO_NEW_POSITION", 48);
				movement(STRAIGHT, 100);
				search_ball = MOVE_TO_NEW_POSITION;
				clearTimer(TIMER);

			}
			break;
		case MOVE_TO_NEW_POSITION:
			if (time1[TIMER] > 4000 )
			{
				send_debug_msg("Changing to ROTATE", 48);
				movement(CLOCKWISE, 24);
				search_ball = ROTATE;
				clearTimer(TIMER);
			}
			break;
		}
		break;
	case MOVE_TO_BALL:
		if (detect_ball_collector())
		{
			goto_returnball();
			break;
		}
		switch (move_ball)
		{
		case OFFSET:
			movement(CCLOCKWISE, 24);
			//if (time1[TIMER] > correction_time(lowest_reading))
			//{
			//	move_ball = MOVE_TOWARDS_BALL;
			//	clearTimer(TIMER);
			//}
			break;
		case MOVE_TOWARDS_BALL:
			movement(STRAIGHT, 100);
			//if (time1[TIMER] > correction_time(lowest_reading) && !(detect_ball_collector()) ) //*****
			//{
			//	machine_mode = SEARCH_FOR_BALL;
			//	clearTimer(TIMER);
			//}
			break;
		}
		break;
	case RETURN_THE_BALL:
		motor[ROLLER_MOT_PORT] = -40;
		switch (machine_return_ball_seq)
		{
		case ALIGN_BEARING:
			if (determine_rotation_direction())
			{
				machine_direction = REVERSE;
				machine_speed = 127;
				return_ball = RETURN_TO_BASE;
				clearTimer(TIMER);
			}
			break;
		case RETURN_TO_BASE:
			if (time1[TIMER] > 4000 )
			{
				machine_direction = STRAIGHT;
				machine_speed = 127;
				return_ball = MOVE_FORWARD;
				clearTimer(TIMER);
			}
			if (detect_ball_deposit())
			{
				machine_direction = STOP;
				machine_speed = 0;
				return_ball = DEPOSIT_BALL;
				clearTimer(TIMER);
			}
			break;
		case DEPOSIT_BALL:
			motor[DOOR_MOT_PORT] = DOOR_MOT_SPEED;
			if (time1[TIMER] > 200)
			{
				goto_movetocenter();
			}
			break;
		case MOVE_FORWARD:
			if (time1[TIMER] > 2000 )
			{
				machine_direction = CCLOCKWISE;
				machine_speed = 24;
				return_ball = TURN_RIGHT;
				clearTimer(TIMER);
			}
			break;
		case TURN_RIGHT:
			if (time1[TIMER] > 2000 )
			{
				machine_direction = STRAIGHT;
				machine_speed = 127;
				return_ball = MOVE_FORWARD_RIGHT;
				clearTimer(TIMER);
			}
			break;
		case MOVE_FORWARD_RIGHT:
			if (time1[TIMER] > 2000 )
			{
				goto_returnball();
			}
			break;
		}
		break;
	default:
		break;
	}
	movement(machine_direction, machine_speed);
}

task runMachine()
{
	while (1)
	{
		run_machine();
		wait1Msec(20);
	}
}

task edgeDetection()
{
	const int reverse_speed = 100;
	const int rotate_speed = 100;
	while (1)
	{
		bool front_left_detected = SensorValue(FRT_LFT_REFL_IR_PORT) <= FRT_LFT_REFL_IR_THRESHOLD;
		bool front_right_detected = SensorValue(FRT_RGT_REFL_IR_PORT) <= FRT_RGT_REFL_IR_THRESHOLD;
		bool back_left_detected = SensorValue(BCK_LFT_REFL_IR_PORT) <= BCK_LFT_REFL_IR_THRESHOLD;
		bool back_right_detected = SensorValue(BCK_RGT_REFL_IR_PORT) <= BCK_RGT_REFL_IR_THRESHOLD;
		if (front_left_detected)
		{
			movement(REVERSE, reverse_speed);
			delay(300);
			movement(CLOCKWISE, rotate_speed);
			delay(300);
			send_debug_msg("front left detected", 32);
		}
		else if (front_right_detected)
		{
			movement(REVERSE, reverse_speed);
			delay(300);
			movement(CCLOCKWISE, rotate_speed);
			delay(300);
			send_debug_msg("front right detected", 32);
		}
		else if (back_left_detected)
		{
			movement(STRAIGHT, reverse_speed);
			delay(300);
			movement(CLOCKWISE, rotate_speed);
			delay(300);
		}
		else if (back_right_detected)
		{
			movement(STRAIGHT, reverse_speed);
			delay(300);
			movement(CCLOCKWISE, rotate_speed);
			delay(300);
		}
		wait1Msec(200);
	}
}

//////////////////////////////////////////////////////////
task main()
{
#ifdef WIFI_DEBUGGING
	setBaudRate(UART_PORT, baudRate115200);
	send_debug_msg("Vex Started\n", 13);
	writeDebugStreamLine("%d", nImmediateBatteryLevel);
#endif
#ifdef CALIBRATION
	while (true)
	{
		char bearing_buf[16];
		switch(get_heading())
		{
		case 1:
			bearing_buf = "NORTH";
			break;
		case 3:
			bearing_buf = "NORTHEAST";
			break;
		case 2:
			bearing_buf = "EAST";
			break;
		case 6:
			bearing_buf = "SOUTHEAST";
			break;
		case 4:
			bearing_buf = "SOUTH";
			break;
		case 12:
			bearing_buf = "SOUTHWEST";
			break;
		case 8:
			bearing_buf = "WEST";
			break;
		case 9:
			bearing_buf = "NORTHWEST";
			break;
		default:
			bearing_buf = "ERROR";
		}
		char buf[256];
		snprintf(buf, sizeof(buf), "Btm Dist IR: %d\n Top Dist IR: %d\n Center Dist IR: %d\n"
		"Front Left IR: %d\n Front Right IR: %d\n Back Left IR: %d\n Back Right IR: %d\n Current Bearing: %s\n"
		"Door Limit Switch: %d\n Ball Limit Switch: %d\n Back Left Limit Switch: %d\n Back Right Limit Switch: %d\n",
		SensorValue(BTM_DIST_IR_PORT), SensorValue(TOP_DIST_IR_PORT), SensorValue(CENTER_DIST_IR_PORT),
		SensorValue(FRT_LFT_REFL_IR_PORT), SensorValue(FRT_RGT_REFL_IR_PORT), SensorValue(BCK_LFT_REFL_IR_PORT), SensorValue(BCK_RGT_REFL_IR_PORT),bearing_buf,
		SensorValue(DOOR_LSWITCH_PORT), SensorValue(BALL_LSWITCH_PORT), SensorValue(BCK_LFT_REFL_IR_PORT), SensorValue(BCK_RGT_REFL_IR_PORT));
		writeDebugStreamLine(buf);
#ifdef WIFI_DEBUGGING
		send_debug_msg(buf, sizeof(buf));
#endif
		delay(500);
	}
#elif defined(TESTING)
	while (1)
	{
	}
#else
	while (true)
	{
		if (SensorValue(START_SWITCH_PORT) == 0)
		{
			send_debug_msg("Beginning Task\n", 24);

			//Initalise values and start tasks clear timer must always be before start run machine task
			machine_mode = MOVE_TO_CENTER;
			while (machine_heading == ERROR)
			{
				machine_heading = get_heading();
			}
			goto_movetocenter();
			startTask(runMachine, kDefaultTaskPriority + 1);
			startTask(edgeDetection);
			break;
		}
	}
	while (1)
	{
	}
#endif
	// while (true)
	//{

	//	//detect_ball_field();
	//	movement(direction1, 24);

	//	lowest_reading = 9999;
	//	if (detect_ballv3())
	//	{
	//		movement(CCLOCKWISE, 24);
	//		delay(correction_time(lowest_reading));
	//		movement(STRAIGHT, 100);
	//		motor[port2] = 127;
	//		delay(1000);
	//		movement(STOP);
	//		direction1 = STOP;
	//	}

	//	if (getChar(UART_PORT) == '1')
	//	{
	//		direction1 = CLOCKWISE;
	//	}
	//	else if (getChar(UART_PORT) == '0'){
	//		direction1 = STOP;
	//	}
	//	delay(50);
	//}

	// Don't remove
	//while (1)
	//{
	//detect_ball_field();
	//	movement(direction1, 24);
	//	//lowest_reading = 9999;
	//	lowest_reading_1 = 9999;
	//	if (detect_ballv4())
	//	{
	//		movement(CLOCKWISE, 24);
	//		delay(500);
	//		movement(STRAIGHT, 100);
	//		motor[port2] = 127;
	//		delay(lowest_reading / 100 * 1.5 * 1000);
	//		movement(STOP);
	//		direction1 = STOP;
	//	}
	//	if (getChar(UART_PORT) == '1')
	//	{
	//		direction1 = CLOCKWISE;
	//	}
	//	else if (getChar(UART_PORT) == '0'){
	//		direction1 = STOP;
	//	}
	//	delay(50);
	//}
}
