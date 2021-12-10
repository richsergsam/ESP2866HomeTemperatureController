#include <Arduino.h>
#include "LittleFS.h"

String timeToString(int *time);
String timeToJson(int *time);
int *stringToTime(String s);
bool isFirstTimeBiggerThanSecond(int *t1, int *t2);
String intToTime(int time);
String readFile(const char *file_path);
void writeFile(String content, const char *file_path);
boolean isInteger(String str);
String getBoardStatus();