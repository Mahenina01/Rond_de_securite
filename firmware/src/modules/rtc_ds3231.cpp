#include "./hal/rtc_ds3231.h"

#include <Wire.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace
{

  SemaphoreHandle_t rtcMutex = nullptr;
  bool rtcReady = false;

  uint8_t bcdToDec(uint8_t value)
  {
    return static_cast<uint8_t>(((value >> 4) * 10) + (value & 0x0F));
  }

  uint8_t decToBcd(uint8_t value)
  {
    return static_cast<uint8_t>(((value / 10) << 4) | (value % 10));
  }

  bool lockBus()
  {
    return rtcMutex != nullptr &&
           xSemaphoreTake(rtcMutex, pdMS_TO_TICKS(1000)) == pdTRUE;
  }

  void unlockBus()
  {
    xSemaphoreGive(rtcMutex);
  }

  bool isLeapYear(int year)
  {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  int daysInMonth(int year, int month)
  {
    static const uint8_t monthLengths[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12)
      return 0;

    if (month == 2 && isLeapYear(year))
      return 29;

    return monthLengths[month - 1];
  }

  int64_t daysFromCivil(int year, unsigned month, unsigned day)
  {
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
    const unsigned adjustedMonth = month > 2 ? month - 3 : month + 9;
    const unsigned dayOfYear =
        (153 * adjustedMonth + 2) / 5 + day - 1;
    const unsigned dayOfEra =
        yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;

    return static_cast<int64_t>(era) * 146097 + dayOfEra - 719468;
  }

  void civilFromDays(int64_t days, int &year, unsigned &month, unsigned &day)
  {
    days += 719468;
    const int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned dayOfEra =
        static_cast<unsigned>(days - era * 146097);
    const unsigned yearOfEra =
        (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 -
         dayOfEra / 146096) /
        365;

    year = static_cast<int>(yearOfEra) + static_cast<int>(era) * 400;
    const unsigned dayOfYear =
        dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    const unsigned monthPrime = (5 * dayOfYear + 2) / 153;

    day = dayOfYear - (153 * monthPrime + 2) / 5 + 1;
    month = monthPrime + (monthPrime < 10 ? 3 : -9);
    year += month <= 2;
  }

  bool rtcIsValidUnlocked()
  {
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    if (Wire.endTransmission() != 0)
      return false;

    if (Wire.requestFrom(
            static_cast<uint8_t>(DS3231_I2C_ADDR),
            static_cast<uint8_t>(1)) != 1)
      return false;

    return (Wire.read() & DS3231_OSF_BIT) == 0;
  }

  bool readDateTimeUnlocked(DateTime &dateTime)
  {
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_TIME);
    if (Wire.endTransmission() != 0)
      return false;

    if (Wire.requestFrom(
            static_cast<uint8_t>(DS3231_I2C_ADDR),
            static_cast<uint8_t>(7)) != 7)
      return false;

    const uint8_t second = bcdToDec(Wire.read() & 0x7F);
    const uint8_t minute = bcdToDec(Wire.read() & 0x7F);
    const uint8_t hourRegister = Wire.read();
    Wire.read();
    const uint8_t day = bcdToDec(Wire.read() & 0x3F);
    const uint8_t month = bcdToDec(Wire.read() & 0x1F);
    const uint16_t year = 2000 + bcdToDec(Wire.read());
    uint8_t hour;

    if (hourRegister & 0x40)
    {
      hour = bcdToDec(hourRegister & 0x1F);
      if (hour < 1 || hour > 12)
        return false;
      hour = static_cast<uint8_t>((hour % 12) +
                                  ((hourRegister & 0x20) ? 12 : 0));
    }
    else
    {
      hour = bcdToDec(hourRegister & 0x3F);
    }

    if (year < 2000 || year > 2099 ||
        month < 1 || month > 12 ||
        day < 1 || day > daysInMonth(year, month) ||
        hour > 23 || minute > 59 || second > 59)
      return false;

    dateTime.year = year;
    dateTime.month = month;
    dateTime.day = day;
    dateTime.hour = hour;
    dateTime.minute = minute;
    dateTime.second = second;
    dateTime.valid = true;

    return true;
  }

} // namespace

bool rtc_init(int sdaPin, int sclPin)
{
  if (rtcMutex == nullptr)
    rtcMutex = xSemaphoreCreateMutex();

  if (rtcMutex == nullptr || !lockBus())
    return false;

  if (sdaPin >= 0 && sclPin >= 0)
    Wire.begin(sdaPin, sclPin);
  else
    Wire.begin();

  Wire.setClock(100000);
  Wire.beginTransmission(DS3231_I2C_ADDR);
  rtcReady = Wire.endTransmission() == 0;

  unlockBus();

  return rtcReady;
}

bool rtc_is_valid(void)
{
  if (!rtcReady || !lockBus())
    return false;

  DateTime dateTime{};
  const bool valid = rtcIsValidUnlocked() &&
                     readDateTimeUnlocked(dateTime);
  unlockBus();

  return valid;
}

bool rtc_now_iso8601(char *bufferOut, size_t maxLen)
{
  if (bufferOut == nullptr || maxLen < RTC_ISO8601_BUF_SIZE)
    return false;

  bufferOut[0] = '\0';

  if (!rtcReady || !lockBus())
    return false;

  DateTime dateTime{};
  const bool valid = rtcIsValidUnlocked() &&
                     readDateTimeUnlocked(dateTime);
  unlockBus();

  if (!valid)
    return false;

  const int length = snprintf(
      bufferOut,
      maxLen,
      "%04u-%02u-%02uT%02u:%02u:%02uZ",
      dateTime.year,
      dateTime.month,
      dateTime.day,
      dateTime.hour,
      dateTime.minute,
      dateTime.second);

  return length == 20;
}

uint32_t rtc_now_epoch(void)
{
  if (!rtcReady || !lockBus())
    return 0;

  DateTime dateTime{};
  const bool valid = rtcIsValidUnlocked() &&
                     readDateTimeUnlocked(dateTime);
  unlockBus();

  if (!valid)
    return 0;

  const int64_t days = daysFromCivil(
      dateTime.year,
      dateTime.month,
      dateTime.day);
  const int64_t epoch = days * 86400 +
                        dateTime.hour * 3600 +
                        dateTime.minute * 60 +
                        dateTime.second;

  return epoch < 0 || epoch > UINT32_MAX
             ? 0
             : static_cast<uint32_t>(epoch);
}

bool rtc_sync_from_string(const char *isoTimestamp)
{
  if (isoTimestamp == nullptr || !rtcReady)
    return false;

  int year, month, day, hour, minute, second, consumed = 0;
  if (sscanf(
          isoTimestamp,
          "%4d-%2d-%2dT%2d:%2d:%2d%n",
          &year,
          &month,
          &day,
          &hour,
          &minute,
          &second,
          &consumed) != 6)
    return false;

  if (year < 2000 || year > 2099 ||
      month < 1 || month > 12 ||
      day < 1 || day > daysInMonth(year, month) ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      second < 0 || second > 59)
    return false;

  int offsetMinutes = 180; // Sans fuseau indiqué, l'heure saisie est Madagascar (UTC+3).
  const char *suffix = isoTimestamp + consumed;

  if (*suffix == 'Z' || *suffix == 'z')
  {
    if (suffix[1] != '\0')
      return false;
    offsetMinutes = 0;
  }
  else if (*suffix == '+' || *suffix == '-')
  {
    int offsetHour, offsetMinute, suffixLength = 0;
    if (sscanf(suffix + 1, "%2d:%2d%n", &offsetHour, &offsetMinute,
               &suffixLength) != 2 ||
        suffix[1 + suffixLength] != '\0' ||
        offsetHour < 0 || offsetHour > 14 ||
        offsetMinute < 0 || offsetMinute > 59 ||
        (offsetHour == 14 && offsetMinute != 0))
      return false;

    offsetMinutes = offsetHour * 60 + offsetMinute;
    if (*suffix == '-')
      offsetMinutes = -offsetMinutes;
  }
  else if (*suffix != '\0')
  {
    return false;
  }

  int64_t utcSeconds =
      daysFromCivil(year, month, day) * 86400 +
      hour * 3600 + minute * 60 + second -
      static_cast<int64_t>(offsetMinutes) * 60;

  int64_t utcDays = utcSeconds / 86400;
  int64_t secondsInDay = utcSeconds % 86400;
  if (secondsInDay < 0)
  {
    secondsInDay += 86400;
    utcDays--;
  }

  unsigned utcMonth, utcDay;
  int utcYear;
  civilFromDays(utcDays, utcYear, utcMonth, utcDay);

  if (utcYear < 2000 || utcYear > 2099)
    return false;

  const uint8_t utcHour = static_cast<uint8_t>(secondsInDay / 3600);
  const uint8_t utcMinute = static_cast<uint8_t>((secondsInDay % 3600) / 60);
  const uint8_t utcSecond = static_cast<uint8_t>(secondsInDay % 60);
  const uint8_t weekday =
      static_cast<uint8_t>(((utcDays + 4) % 7 + 7) % 7 + 1);

  if (!lockBus())
    return false;

  Wire.beginTransmission(DS3231_I2C_ADDR);
  Wire.write(DS3231_REG_TIME);
  Wire.write(decToBcd(utcSecond));
  Wire.write(decToBcd(utcMinute));
  Wire.write(decToBcd(utcHour));
  Wire.write(weekday);
  Wire.write(decToBcd(static_cast<uint8_t>(utcDay)));
  Wire.write(decToBcd(static_cast<uint8_t>(utcMonth)));
  Wire.write(decToBcd(static_cast<uint8_t>(utcYear % 100)));

  bool synced = Wire.endTransmission() == 0;

  if (synced)
  {
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    synced = Wire.endTransmission() == 0;
  }

  if (synced &&
      Wire.requestFrom(
          static_cast<uint8_t>(DS3231_I2C_ADDR),
          static_cast<uint8_t>(1)) == 1)
  {
    uint8_t status = Wire.read();
    status &= static_cast<uint8_t>(~DS3231_OSF_BIT);

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    Wire.write(status);
    synced = Wire.endTransmission() == 0;
  }
  else
  {
    synced = false;
  }

  unlockBus();

  return synced;
}

void rtc_power_down(void)
{
  if (!rtcReady || !lockBus())
    return;

  Wire.endTransmission();
  unlockBus();
}

DateTime rtc_now(void)
{
  DateTime dateTime{};

  if (!rtcReady || !lockBus())
    return dateTime;

  if (!rtcIsValidUnlocked() || !readDateTimeUnlocked(dateTime))
    dateTime.valid = false;

  unlockBus();

  return dateTime;
}
