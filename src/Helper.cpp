#include <Helper.hpp>

String readFile(const char *file_path)
{
  Serial.println("Try to read file: " + String(file_path));
  Serial.println(file_path);
  File file = LittleFS.open(file_path, "r");
  String fileContent = file.readString();
  file.close();
  Serial.println("Reading is success: " + String(file_path));
  return fileContent;
}

void writeFile(String content, const char *file_path)
{
  Serial.println("Try to write file: " + String(file_path));
  File file = LittleFS.open(file_path, "w");
  file.print(content);
  file.close();
  Serial.println("Writing is success: " + String(file_path));
}

String timeToString(int *time)
{
  return intToTime(time[0]) + ":" + intToTime(time[1]) + ":" + intToTime(time[2]);
}

String timeToJson(int *time)
{
  return "[\"" + intToTime(time[0]) + "\",\"" + intToTime(time[1]) + "\",\"" + intToTime(time[2]) + "\"]";
}

int *stringToTime(String s)
{
  int hours = s.substring(0, 2).toInt();
  int minutes = s.substring(3, 5).toInt();
  int seconds = s.substring(6, 8).toInt();
  return new int[3]{hours, minutes, seconds};
}
String intToTime(int time)
{
  return (time < 10 ? "0" : "") + String(time);
}
bool isFirstTimeBiggerThanSecond(int *t1, int *t2)
{
  if (t1[0] > t2[0])
  {
    return true;
  }
  else
  {
    if (t1[0] == t2[0] && t1[1] > t2[1])
    {
      return true;
    }
    else
    {
      if (t1[1] == t2[1] && t1[2] > t2[2])
      {
        return true;
      }
    }
  }
  return false;
}

boolean isInteger(String str)
{
  if (str.length())
  {
    for (char i = 0; i < str.length(); i++)
    {
      if (!isDigit(str.charAt(i)))
      {
        return false;
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

String getBoardStatus()
{
  String output = "";
  output += "CPU frequency:" + String(ESP.getCpuFreqMHz()) + "\n";
  output += "FlashChipSize: " + String(ESP.getFlashChipSize()) + "\n";
  output += "FlashChipRealSize: " + String(ESP.getFlashChipRealSize()) + "\n";
  output += "SketchSpace: " + String(ESP.getFreeSketchSpace()) + "\n";
  output += "SketchSize: " + String(ESP.getSketchSize()) + "\n";
  output += "FreeHeap: " + String(ESP.getFreeHeap()) + "\n";
  output += "HeapFragmentation: " + String(ESP.getHeapFragmentation()) + "\n";
  output += "FreeContStack: " + String(ESP.getFreeContStack()) + "\n";
  output += "MaxFreeBlockSize: " + String(ESP.getMaxFreeBlockSize()) + "\n";
  output += "Vcc: " + String(ESP.getVcc()) + "\n";
  output += "";
  return output;
}

