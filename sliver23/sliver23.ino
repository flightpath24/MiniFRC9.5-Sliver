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
NoU_Motor armMotor(5);
NoU_Servo wristServo(4);

  // Intake
NoU_Servo intakeRightServo(2);
NoU_Servo intakeLeftServo(3);

  // Position variables
int throttle = 0;
int rotation = 0;
int armServoAngle = 0;
int intakeRightServoAngle = 90;
int intakeLeftServoAngle = 90;
int wristServoAngle = 15;
int armSpeed = 0;

  // Auto and setpoint variables
int autoSequence = 0;
int autoStartTime = 0;
int autoTimer = 0;
bool autoStarted = false;
int sequenceStartTime = 0;
int currentTime = 0;
int setpoint = -1;
int localTime = 0;

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
    throttle = 0;
    rotation = 0;

    currentTime = millis();
    autoTimer = currentTime - autoStartTime;

// Drivetrain controls
    if (AlfredoConnect.keyHeld(Key::W)) {
        throttle = 1;
    } else if (AlfredoConnect.keyHeld(Key::S)) {
        throttle = -1;
    }
    if (AlfredoConnect.keyHeld(Key::D)) {
        rotation = -1;
    } else if (AlfredoConnect.keyHeld(Key::A)) {
        rotation = 1;
    }

// Main arm pivot controls
    if (AlfredoConnect.keyHeld(Key::O)) {
        armServoAngle = 120;
    }
    else if (AlfredoConnect.keyHeld(Key::L)) {
        armServoAngle = 180;
    }
    else {
        armServoAngle = 0;
    }

// Arm extender controls
    if (AlfredoConnect.keyHeld(Key::Y)) {
        armSpeed = 1;
    } else if (AlfredoConnect.keyHeld(Key::H)) {
        armSpeed = -1;
    } else {
      armSpeed = 0;
    }

// Intake controls
    if (AlfredoConnect.keyHeld(Key::Z)) {
        intakeRightServoAngle = 30;
        intakeLeftServoAngle = 180;
    }
    else {
        intakeRightServoAngle = 105;
        intakeLeftServoAngle = 115;
    }

// Wrist controls
    if (AlfredoConnect.keyHeld(Key::V)) {
        wristServoAngle = 90;
    }
    else if (AlfredoConnect.keyHeld(Key::B)) {
        wristServoAngle = 180;
    }
    else {
        wristServoAngle = 15;
    }

// Trigger autos
    if (AlfredoConnect.keyHeld(Key::Digit1)) {
      autoSequence = 1;
    } else if (AlfredoConnect.keyHeld(Key::Digit2)) {
      autoSequence = 3;
    } else if (AlfredoConnect.keyHeld(Key::Digit3)) {
      autoSequence = 2;
    } else if (AlfredoConnect.keyHeld(Key::Digit4)) {
      autoSequence = 4;
    }

// Setpoints
    if (AlfredoConnect.keyHeld(Key::G)) {
      setpoint = 1;
    } else if (AlfredoConnect.keyHeld(Key::U)) {
      setpoint = 4;
    } else if (AlfredoConnect.keyHeld(Key::J)) {
      setpoint = 3;
    } else if (AlfredoConnect.keyHeld(Key::N)) {
      setpoint = 2;
    } else if (AlfredoConnect.keyHeld(Key::T)) {
      setpoint = 6;
    }
    
    smartControl();
// Push angles and speeds to the robot
    //bluetooth.println(armSpeed);
    drivetrain.curvatureDrive(throttle, rotation);
    armServo.write(armServoAngle);
    intakeRightServo.write(intakeRightServoAngle);
    intakeLeftServo.write(intakeLeftServoAngle);
    wristServo.write(wristServoAngle);
    armMotor.set(armSpeed);

// No need to mess with this code
    AlfredoConnect.update();
    RSL::update();
}

//
void smartControl() {
  switch(setpoint) {
    case 0:
      stow();
      return;
    case 1:
      groundFront();
      return;
    case 2:
      lowRow();
      break;
    case 3:
      midRow();
      break;
    case 4:
      highRow();
      return;
    case 5:
      //doubleSubFront();
      break;
    case 6:
      doubleSubRear();
      break;
    default:
      // Do Nothing
      break;
  }

  switch(autoSequence) {
    case 1:
      // Mobility only
      mobilityAuto();
      return;
    case 2:
      midPieceMobilityAuto();
      break;
    case 3:
      midPieceAuto();
      break;
    case 4:
      midPieceMobilityDockAuto();
      break;
    case 5:
      //highPiecePickupAuto();
      break;
    default:
      // Teleop; do nothing
      break;
  }
}

void mobilityAuto() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
  } else if(autoTimer < 1000) {
    throttle = 1;
    return;
  } else {
    autoSequence = 0;
    autoStarted = false;
  }
}

void midPieceAuto() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 1000) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1500) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (autoTimer < 2100) {
    armServoAngle = 105;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if (autoTimer > 2100) {
    autoStarted = false;
    autoSequence = 0;
    return;
  }
}

void midPieceMobilityAuto() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 1000) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1500) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (autoTimer < 2100) {
    armServoAngle = 105;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if(autoTimer < 2200) { //stow
    armServoAngle = 30;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 3100) {
    armServoAngle = 30;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = -1;
    return;
  } else if(autoTimer < 3300) {
    armServoAngle = 0;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if(autoTimer < 4300) { // drive
    throttle = 1;
    return;
  } else if (autoTimer > 4300) {
    autoStarted = false;
    autoSequence = 0;
    return;
  }
}

void midPieceMobilityDockAuto() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 1000) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1500) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (autoTimer < 2100) {
    armServoAngle = 105;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if(autoTimer < 2200) { //stow
    armServoAngle = 30;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 3100) {
    armServoAngle = 30;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = -1;
    return;
  } else if(autoTimer < 3300) {
    armServoAngle = 0;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if(autoTimer < 4600) { //drive
    throttle = 1;
    return;
  } else if(autoTimer < 5300) {
    throttle = -1;
    return;
  } else if (autoTimer > 5000) {
    autoStarted = false;
    autoSequence = 0;
    return;
  }
}

void groundFront() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 100) {
    armServoAngle = 10;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 150) {
    armServoAngle = 10;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 90;
    armSpeed = 1;
    return;
  } else if(AlfredoConnect.keyHeld(Key::E)) {
    armServoAngle = 10;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 180;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (150 > currentTime - localTime && currentTime - localTime > 100) {
    setpoint = 0;
    autoStarted = false;
    return;
  } else if (currentTime - localTime > 150) {
    armServoAngle = 10;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 180;
    armSpeed = 0;
    return;
  }
}

void doubleSubRear() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 500) {
    armServoAngle = 80;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 75;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1000) {
    armServoAngle = 150;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 75;
    armSpeed = 0;
    return;
  } else if(AlfredoConnect.keyHeld(Key::E)) {
    armServoAngle = 150;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 75;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (150 > currentTime - localTime && currentTime - localTime > 100) {
    setpoint = 0;
    autoStarted = false;
    return;
  } else if (currentTime - localTime > 150) {
    armServoAngle = 150;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 105;
    armSpeed = 0;
    return;
  }
}

void highRow() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 200) {
    armServoAngle = 85;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 350) {
    armServoAngle = 85;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 1;
    return;
  } else if(autoTimer < 500) {
    armServoAngle = 95;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 600) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 700) {
    armServoAngle = 115;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1100) {
    armServoAngle = 125;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 130;
    armSpeed = 0;
    return;
  } else if(AlfredoConnect.keyHeld(Key::E) && localTime == 0) {
    armServoAngle = 125;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 130;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (900 > currentTime - localTime && currentTime - localTime > 500) {
    armServoAngle = 90;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if (localTime != 0 && currentTime - localTime > 900) {
    autoStarted = false;
    setpoint = 0;
    return;
  } else if (currentTime - localTime > 900 && localTime == 0) {
    armServoAngle = 125;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 130;
    armSpeed = 0;
    return;
  } else {
    armServoAngle = 125;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 130;
    armSpeed = 0;
    return;
  }
}

void midRow() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 100) {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(AlfredoConnect.keyHeld(Key::E) && localTime == 0) {
    armServoAngle = 105;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 90;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (localTime != 0 && currentTime - localTime < 600) {
    armServoAngle = 105;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if (localTime != 0 && currentTime - localTime > 600) {
    autoStarted = false;
    setpoint = 0;
    return;
  } else {
    armServoAngle = 105;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    return;
  }
}

void lowRow() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 100) {
    armServoAngle = 90;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if(AlfredoConnect.keyHeld(Key::E) && localTime == 0) {
    armServoAngle = 90;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    localTime = currentTime;
    return;
  } else if (localTime != 0 && currentTime - localTime < 600) {
    armServoAngle = 90;
    intakeRightServoAngle = 30;
    intakeLeftServoAngle = 180;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else if (localTime != 0 && currentTime - localTime > 600) {
    autoStarted = false;
    setpoint = 0;
    return;
  } else {
    armServoAngle = 90;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    return;
  }
}

void stow() {
  if(autoStarted == false) {
    autoStarted = true;
    autoStartTime = currentTime;
    localTime = 0;
  } else if(autoTimer < 100) {
    armServoAngle = 15;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = 0;
    return;
  } else if(autoTimer < 1000) {
    armServoAngle = 15;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 90;
    armSpeed = -1;
    return;
  } else if(autoTimer < 1200) {
    armServoAngle = 0;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    armSpeed = 0;
    return;
  } else {
    armServoAngle = 0;
    intakeRightServoAngle = 105;
    intakeLeftServoAngle = 115;
    wristServoAngle = 15;
    armSpeed = 0;
    setpoint = -1;
    autoStarted = false;
    return;
  }
}













