#ifndef AnalogSensor_h
#define AnalogSensor_h

typedef double (*converter) (double, unsigned char);

typedef struct CalPoint {
  double x;
  double y;
};

class AnalogSensor{
  public:
    CalPoint* CalibrationPoints;
    double* RollingBuffer;
    double Slope;
    double Intercept;
    double StandardDeviation;
    double CurrentValue;
    
    AnalogSensor(
      unsigned char maxCalPoints,
      double defaultSlope,
      double defaultIntercept,
      unsigned char rollingBufferSize
      );

    // Use the current calibration points to calculate the best-fit line parameters for this sensor
    void Calibrate(); 

    // Add a raw value to the rolling buffer, usually from an analogRead();
    void AddMeasurement(double measurement);

    void ClearAll();

    void ListAll();

    double GetConvertedValue();

  private:
    unsigned char _numCalPoints;
    double _defaultSlope;
    double _defaultIntercept;
    unsigned char _rollingBufferSize;
    unsigned char _rollingBufferIndex;
};

#endif