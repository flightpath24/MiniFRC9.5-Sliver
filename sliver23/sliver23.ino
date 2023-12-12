/**
 * Code for the sliver robot in 2023 using an ESP32 and NoU2 controlled with the keyboard from AlfredoConnect.
 * The NoU2 library can be found at https://github.com/AlfredoElectronics/Alfredo-NoU2.
 *
 * Detailed instructions can be found at https://github.com/AlfredoElectronics/AlfredoConnect-Receive/.
 */

#include <AlfredoConnect.h>
#include <BluetoothSerial.h>
#include <Alfredo_NoU2.h>

BluetoothSerial bluetooth;

// Include motors and servos here
  // Drivetrain
NoU_Motor frontLeftMotor(4);
NoU_Motor frontRightMotor(1);
NoU_Motor rearLeftMotor(3);
NoU_Motor rearRightMotor(2);

  // Arm
NoU_Servo armServo(1);
//NoU_Motor armMotor(5);
NoU_Servo wristServo(4);

  // Intake
NoU_Servo intakeRightServo(2);
NoU_Servo intakeLeftServo(3);

// This creates the drivetrain object, you shouldn't have to mess with this
NoU_Drivetrain drivetrain(&frontLeftMotor, &frontRightMotor, &rearLeftMotor, &rearRightMotor);

void setup() {
// Boots the robot up
    bluetooth.begin("Sliverbot 23");
    AlfredoConnect.begin(bluetooth);
    bluetooth.println("Sliver code started");

// If a motor in the drivetrain is spinning the wrong way, change the value for it here from 'false' to 'true'
    frontLeftMotor.setInverted(false);
    frontRightMotor.setInverted(true);
    rearLeftMotor.setInverted(false);
    rearRightMotor.setInverted(true);

// No need to mess with this code
    RSL::initialize();
    RSL::setState(RSL_ENABLED);
}

void loop() {
// Here we define the variables we use in the loop
    int throttle = 0;
    int rotation = 0;
    int armServoAngle = 0;
    int intakeRightServoAngle = 90;
    int intakeLeftServoAngle = 90;
    int wristServoAngle = 0;

// Drivetrain controls
    if (AlfredoConnect.keyHeld(Key::S)) {
        throttle = 1;
    } else if (AlfredoConnect.keyHeld(Key::W)) {
        throttle = -1;
    }
    if (AlfredoConnect.keyHeld(Key::D)) {
        rotation = -1;
    } else if (AlfredoConnect.keyHeld(Key::A)) {
        rotation = 1;
    }

// Main arm pivot controls
    if (AlfredoConnect.keyHeld(Key::Q)) {
        armServoAngle = 120;
    }
    else if (AlfredoConnect.keyHeld(Key::E)) {
        armServoAngle = 180;
    }
    else {
        armServoAngle = 0;
    }

// Intake controls
    if (AlfredoConnect.keyHeld(Key::Z)) {
        intakeRightServoAngle = 30;
        intakeLeftServoAngle = 150;
    }
    else {
        intakeRightServoAngle = 90;
        intakeLeftServoAngle = 90;
    }

// Wrist controls
    if (AlfredoConnect.keyHeld(Key::T)) {
        wristServoAngle = 90;
    }
    else if (AlfredoConnect.keyHeld(Key::G)) {
        wristServoAngle = 180;
    }
    else {
        wristServoAngle = 0;
    }

// Push angles and speeds to the robot
    drivetrain.arcadeDrive(throttle, rotation);
    armServo.write(armServoAngle);
    intakeRightServo.write(intakeRightServoAngle);
    intakeLeftServo.write(intakeLeftServoAngle);
    wristServo.write(wristServoAngle);

// No need to mess with this code
    AlfredoConnect.update();
    RSL::update();
}