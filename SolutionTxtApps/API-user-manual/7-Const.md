## Available enum's
Part of the namespace:
```using namespace fischertechnik::txt::ids;```
### Output/Motor related
```C
/// <summary>
/// The 4 Motor ouputs
/// </summary>
enum Motor :uint8_t
{
	M1 = 0, M2 = 1, M3 = 2, M4 = 3, NoMotor = 15
};

/// <summary>
/// The 8 Output outputs
/// </summary>
enum Output :uint8_t
{
	O1 = 0, O2, O3, O4, O5, O6, O7, O8, NoOutput = 15
};
/// <summary>
/// The Directions
/// </summary>
enum Direction :int8_t {
	CCW = -1, Halt = 0, CW = 1, NoDirection = 15
};
```
### Input related
```C
/// <summary>
/// The 8 Inputs 
/// </summary>
enum  Input :uint8_t
{
	I1 = 0, I2, I3, I4, I5, I6, I7, I8, NoInput = 15
};

/// <summary>
/// The Universal Input modes
/// </summary>
enum InputMode
{
	MODE_U = 0,   //Voltage 10V
	MODE_R = 1,       //Resistance 10kOhm
	//MODE_R2=2,       //not in use for the TXT
	MODE_ULTRASONIC = 3, //Distance sensor
	MODE_INVALID = 4
};
```
### Counter related
```C
/// <summary>
/// The 4 Counter inputs
/// </summary>
enum Counter :uint8_t {
	C1 = 0, C2, C3, C4, NoCounter = 15
};
```
### IR devices related (fischertechnik IR controllers)
```C
/// <summary>
/// IR controllers 
/// </summary>
enum IrDev :uint8_t {
	IR_ALL_DATA = 0,//  :IR Data for all receivers
	IR_OFF_OFF=1,// IR Data for receivers with SW1=0  SW2=0
	IR_ON_OFF=2,//  IR Data for receivers with SW1=1  SW2=0
	IR_OFF_ON=3,//  IR Data for receivers with SW1=0  SW2=1
	IR_ON_ON=4//   IR Data for receivers with SW1=1  SW2=1
};
/// <summary>
/// IR controller full joystick  axis (0=middle -15..0..+15)
/// </summary>
enum IrAxisF :uint8_t {
	JoyLeftX = 10,            // left Joystick X-Axis  (0=middle -15..0..+15)
	JoyLeftY = 11,                //  left Joystick Y-Axis  (0=middle -15..0..+15)
	JoyRightX = 20,               //  right Joystick X-Axis  (0=middle -15..0..+15)
	JoyRightY=21               // right Joystick Y-Axis  (0=middle -15..0..+15)
};
/// <summary>
/// IR controller partial joystick axis (0=middle..+15) 
/// </summary>
enum IrAxisP :uint8_t {
	JoyLeftXtoLeft = 12,          // left Joystick X-Axis from middle to left maximum (0=middle..+15)
	JoyLeftXtoRight = 13,         // left Joystick X-Axis from middle to right maximum (0=middle..+15)
	JoyLeftYtoForward = 14,       //  left Joystick Y-Axis from middle to forward maximum (0=middle..+15)
	JoyLeftYtoBackwards = 15,     // uleft Joystick Y-Axis from middle to backwards maximum (0=middle..+15)

	JoyRightXtoLeft = 22,         // right Joystick X-Axis from middle to left maximum (0=middle..+15)
	JoyRightXtoRight = 23,        // right Joystick Y-Axis from middle to forward maximum (0=middle..+15)
	JoyRightYtoForward = 24,     //  right Joystick Y-Axis from middle to forward maximum (0=middle..+15)
	JoyRightYtoBackwards = 25    // right Joystick Y-Axis from middle to backwards maximum (0=middle..+15)
};
/// <summary>
/// IR controller button/switches
/// </summary>
enum IrButSwi :uint8_t {
	ButtonOn = 0,                // ON-Switch: 1=pressed 
	ButtonOff,               // OFF-Switch: 1=pressed
	DipSwitch1,              // 1: Switch ON, 0: Switch OFF
	DipSwitch2              // 1: Switch ON, 0: Switch OFF
};
/// <summary>
/// The error's
/// </summary>
enum FtErrors :unsigned int {

	FTLIB_ERR_SUCCESS = 0x00000000L,
	FTLIB_ERR_INVALID_PARAM = 0xE0001018L,
	FTLIB_ERR_FAILED = 0xE0001000L,
	FTLIB_ERR_THREAD_IS_RUNNING = 0xE00012A5L,
	FTLIB_ERR_THREAD_NOT_RUNNING = 0xE00012A6L,
	FTLIB_ERR_UNKNOWN = 0xEFFFFFFFL
};
```

# Document history
Version 1.1.2.1
(c) 2020 TesCaWeb ing. C. van Leeuwen Btw.
