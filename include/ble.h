#pragma once
void setupBLE();
void sendCharge(int value);
void sendLeftBelt(int value);
void sendRightBelt(int value);
void sendServo(int value);
void bleTask();