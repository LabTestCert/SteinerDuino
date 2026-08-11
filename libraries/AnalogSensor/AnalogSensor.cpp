#include "Arduino.h"
#include "AnalogSensor.h"

AnalogSensor::AnalogSensor(unsigned char maxCalPoints, double defaultSlope, double defaultIntercept, unsigned char rollingBufferSize){
  _numCalPoints = maxCalPoints;
  _defaultSlope = defaultSlope;
  _defaultIntercept = defaultIntercept;
  _rollingBufferSize = rollingBufferSize;

  CalibrationPoints = (CalPoint*)malloc(_numCalPoints*sizeof(CalPoint));
  for(int i=0; i<_numCalPoints; i++){
    CalibrationPoints[i].x = -1;
    CalibrationPoints[i].y = 0;
  }

  RollingBuffer = (double*)malloc(_rollingBufferSize*sizeof(double));
  for(int i=0; i<_rollingBufferSize; i++) RollingBuffer[i] = 0;

  Slope = defaultSlope;
  Intercept = defaultIntercept;

  StandardDeviation = CurrentValue = _rollingBufferIndex = 0;
}

void AnalogSensor::Calibrate(){
  double xyProds = 0, xSum = 0, ySum = 0, xSquared = 0;
  unsigned char n = 0;

  double defaultSlope, defaultIntercept;

  // BEST FIT LINE
  if(CalibrationPoints[0].x == -1){
    Slope = defaultSlope; 
    Intercept = defaultIntercept;
  }
  else{
    for(int i=0; i<_numCalPoints; i++){
      if(CalibrationPoints[i].x != -1){
        xyProds += CalibrationPoints[i].x * CalibrationPoints[i].y;
        xSum += CalibrationPoints[i].x;
        ySum += CalibrationPoints[i].y;
        xSquared += (CalibrationPoints[i].x * CalibrationPoints[i].x);
        n++;
      }
    }

    Slope = (n*xyProds - xSum*ySum) / (n*xSquared - xSum*xSum);
    Intercept = (ySum - Slope*xSum) / n;
  }

  // STANDARD DEVIATION
  double errorSqu;
  double sum = 0;
  n = 0;

  for(int i=0; i<_numCalPoints; i++){
    errorSqu = 0;
    if(CalibrationPoints[i].x != -1){
      errorSqu = CalibrationPoints[i].y - (Slope * CalibrationPoints[i].x + Intercept);
      errorSqu = errorSqu * errorSqu;
      sum += errorSqu;
      n++;
    }
  }
  n = n - 1;
  
  StandardDeviation = (double)sqrt(sum / (double)n);
}

void AnalogSensor::AddMeasurement(double measurement){
  RollingBuffer[_rollingBufferIndex] = measurement;

  CurrentValue = 0;

  for(int i=0; i<_rollingBufferSize; i++) CurrentValue += RollingBuffer[i] / (double) _rollingBufferSize;

  _rollingBufferIndex++;
  if(_rollingBufferIndex >= _rollingBufferSize) _rollingBufferIndex = 0;
}

void AnalogSensor::ClearAll(){
  for(int i=0; i<_numCalPoints; i++) {
    CalibrationPoints[i].x = -1;
    CalibrationPoints[i].y = 0;
  }
}

double AnalogSensor::GetConvertedValue(){
  return CurrentValue * Slope + Intercept;
}