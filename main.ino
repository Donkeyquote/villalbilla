#include <settings.h>

#include <poolvalve.h>
#include <rfvalve.h>
#include <extractor.h>
#include <solarpump.h>
#include <returnpump.h>
#include <radiantfloorpump.h>
#include <boiler.h>
#include <heatpump.h>

#include <temperaturesensor.h>
#include <humiditysensor.h>
#include <photosensor.h>

#include <seasonselector.h>

// Display Timer
unsigned long int aPreviousMillis = 0;

// Globals Temperature
TemperatureSensor * aSolarSensor_AB = nullptr;
TemperatureSensor * aTankSensor_A = nullptr;
TemperatureSensor * aPoolSensor_B = nullptr;
TemperatureSensor * aRadiantFloorSensor_B = nullptr;

// Globals Humidity
HumiditySensor * aExternalHumiditySensor = nullptr;
HumiditySensor * aHumiditySensor_1 = nullptr;
HumiditySensor * aHumiditySensor_2 = nullptr; 

// Globals Light
PhotoSensor * aPhotosensor = nullptr;

// Globals Season Selector
SeasonSelector * aSeasonSelector = nullptr;

// Globals Generators
Boiler * aBoiler = nullptr;
HeatPump * aHeatPump = nullptr;

// Globals Pumps
SolarPump * aSolarPump = nullptr;
ReturnPump * aReturnPump = nullptr;
RadiantFloorPump * aRadiantFloorPump = nullptr;

// Globals Extractor
Extractor * aExtractor = nullptr;

// Globals Valves
PoolValve * aPoolValve = nullptr;
RF_Valve * aRF_Valve = nullptr;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //invoking Temperature sensors
  aSolarSensor_AB = new TemperatureSensor(__solar_temperature_sensor_pin__, __lower_threshold_AB__, __upper_threshold_AB__, __setup_value_AB__);
  aTankSensor_A = new TemperatureSensor(__tank_temperature_sensor_pin__, __lower_threshold_tank__, __upper_threshold_tank__, __setup_value_tank__);
  aPoolSensor_B = new TemperatureSensor(__pool_temperature_sensor_pin__, __lower_threshold_pool__, __upper_threshold_pool__, __setup_value_pool__);
  aRadiantFloorSensor_B = new TemperatureSensor(__radiant_temperature_sensor_pin__, __lower_threshold_radiant__, __upper_threshold_radiant__, __setup_value_radiant__);

  //invoking Humidity sensors
  aExternalHumiditySensor = new HumiditySensor(__external_humidity_sensor_pin__);
  aHumiditySensor_1 = new HumiditySensor(__humidity_sensor_pin_1__);
  aHumiditySensor_2 = new HumiditySensor(__humidity_sensor_pin_2__);  

  //invoking photosensor
  aPhotosensor = new PhotoSensor( __photosensor_pin__ );

  //invoking selector
  aSeasonSelector = new SeasonSelector( __selector_winter_pin__, __selector_summer_pin__ );

  //loading temperature sensor values
  aSolarSensor_AB -> loadCurrentValue();
  aTankSensor_A -> loadCurrentValue();
  aPoolSensor_B -> loadCurrentValue();
  aRadiantFloorSensor_B -> loadCurrentValue();  


  //loading humidity sensor values
  aExternalHumiditySensor -> loadCurrentValue(); 
  aHumiditySensor_1 -> loadCurrentValue();
  aHumiditySensor_2 -> loadCurrentValue();

  //invoking boiler
  aBoiler = new Boiler( aSeasonSelector, aRadiantFloorSensor_B, __bridge_boiler_pin__ ); 


  //invoking heat pump
  aHeatPump = new HeatPump( aSeasonSelector, __bridge_heat_pump_pin__ );

  //invoking solar pump
  aSolarPump = new SolarPump( __solar_pump_pin__, aPhotosensor, aExternalHumiditySensor, aSolarSensor_AB, __solar_check_time_off__, __solar_check_time_on__ );

  //invoking hot water return pump
  aReturnPump = new ReturnPump( _return_pump_pin_, __return_check_time_off__, __return_check_time_on__ );

  //invoking radiant floor pump
  aRadiantFloorPump = new RadiantFloorPump( _radiant_floor_pump_pin_, aSeasonSelector );

  //invoking extractor
  aExtractor = new Extractor( __extractor_pin__, aHumiditySensor_1, aHumiditySensor_2, aExternalHumiditySensor, __extractor_off_time__, __extractor_on_time__ );

  //invoking 3-ways valves
  aRF_Valve = new RF_Valve( aSeasonSelector, __radiant_valve_A_pin__, __radiant_valve_B_pin__, aSolarSensor_AB, aTankSensor_A, aRadiantFloorSensor_B, -99, __time_valves_on__ );
  
  aPoolValve = new PoolValve( aSeasonSelector, __pool_valve_A_pin__, __pool_valve_B_pin__, aSolarSensor_AB, aTankSensor_A, aPoolSensor_B, -99, __time_valves_on__ );

  displayInfo();

}

void loop() {
  // put your main code here, to run repeatedly:

//******************
/*
digitalWrite(45, LOW);

if (digitalRead(45) == LOW)
  Serial.println("45 LOW");

*/
delay(10000);

//******************


  //Generators
  aBoiler -> act();
  aHeatPump -> act();

  //Pumps
  aSolarPump -> act();
  aReturnPump -> act();
  aRadiantFloorPump -> act();

  //Fans
  aExtractor -> act();

  //Valves
  aRF_Valve -> act();
  aPoolValve -> act();

  displayInfo();

  
} 

void displayInfo(){

  if ( millis() - aPreviousMillis > 5000){

    Serial.println("==========================INPUTS==============================");

    aSolarSensor_AB -> loadCurrentValue();
    Serial.print("Solar Temp (AB): ");    
    Serial.println(aSolarSensor_AB -> getCurrentValue());

    aTankSensor_A -> loadCurrentValue();
    Serial.print("Tank Temp (A): ");
    Serial.println(aTankSensor_A -> getCurrentValue());

    aPoolSensor_B -> loadCurrentValue();
    Serial.print("Pool Temp (B): ");
    Serial.println(aPoolSensor_B -> getCurrentValue());

    aRadiantFloorSensor_B -> loadCurrentValue();
    Serial.print("Radiant Floor Temp (B): ");
    Serial.println(aRadiantFloorSensor_B -> getCurrentValue());

    Serial.print("External Temperature: ");
    Serial.println(aExternalHumiditySensor -> readTemperature());

    aExternalHumiditySensor -> loadCurrentValue();
    Serial.print("External Humidity: ");
    Serial.println(aExternalHumiditySensor -> getCurrentValue());

    aHumiditySensor_1 -> loadCurrentValue();
    Serial.print("Floor -1 Humidity: ");
    Serial.println(aHumiditySensor_1 -> getCurrentValue());

    if ( aHumiditySensor_1 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() > __start_external_vs_1__ ){      
      Serial.print( "EXTRACTOR 1 SIGNAL (ON): " );
      Serial.print( aHumiditySensor_1 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() );
      Serial.print( " > " );
      Serial.println( __start_external_vs_1__ );      
    }

    else
      Serial.print( "EXTRACTOR 1 SIGNAL (OFF): " );
      Serial.print( aHumiditySensor_1 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() );
      Serial.print( " < " );
      Serial.println( __start_external_vs_1__ );
      

    aHumiditySensor_2 -> loadCurrentValue();
    Serial.print("Floor -2 Humidity: ");
    Serial.println(aHumiditySensor_2 -> getCurrentValue());

    if ( aHumiditySensor_2 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() > __start_external_vs_2__ ){      
      Serial.print( "EXTRACTOR 2 SIGNAL (ON): " );
      Serial.print( aHumiditySensor_2 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() );
      Serial.print( " > " );
      Serial.println( __start_external_vs_2__ );      
    }

    else
      Serial.print( "EXTRACTOR 2 SIGNAL (OFF): " );
      Serial.print( aHumiditySensor_2 -> getCurrentValue() / aExternalHumiditySensor -> getCurrentValue() );
      Serial.print( " < " );
      Serial.println( __start_external_vs_2__ );

    //loading photosensor
    aPhotosensor -> loadState();

    if ( aPhotosensor -> isOn() )
      Serial.println("DAY");
    else
      Serial.println("NIGHT");
  

    //loading selector button
    aSeasonSelector -> loadSeason();

    if ( aSeasonSelector->isSummer() )
      Serial.println("SUMMER");

    else if ( aSeasonSelector->isWinter() )
      Serial.println("WINTER");

    Serial.println("==========================OUTPUTS=============================");
  
    if ( digitalRead(__bridge_boiler_pin__) == LOW)
      Serial.println("HEATING ON");
  
    else
      Serial.println("HEATING OFF");

    if ( digitalRead(__bridge_heat_pump_pin__) == LOW)
      Serial.println("AEROTHERM ON");
  
    else
      Serial.println("AEROTHERM OFF");

    if ( digitalRead(__solar_pump_pin__) == LOW)
      Serial.println("SOLAR PUMP ON");
  
    else
      Serial.println("SOLAR PUMP OFF");

    if ( digitalRead(_return_pump_pin_) == LOW)
      Serial.println("RETURN PUMP ON");
  
    else
      Serial.println("RETURN PUMP OFF");

    if ( digitalRead(_radiant_floor_pump_pin_) == LOW)
      Serial.println("RADIANT PUMP ON");
  
    else
      Serial.println("RADIANT PUMP OFF");

    if ( digitalRead(__extractor_pin__) == LOW)
      Serial.println("EXTRACTOR ON");
  
    else
      Serial.println("EXTRACTOR OFF");



    if ( digitalRead( __radiant_valve_A_pin__ ) == LOW)
      Serial.println("RADIANT FLOOR VALVE A-WAY ON");
  
    else
      Serial.println("RADIANT FLOOR VALVE A-WAY OFF");  

    if ( digitalRead( __radiant_valve_B_pin__ ) == LOW)
      Serial.println("RADIANT FLOOR VALVE B-WAY ON");
  
    else
      Serial.println("RADIANT FLOOR VALVE B-WAY OFF");



    if ( digitalRead( __pool_valve_A_pin__ ) == LOW)
      Serial.println("POOL VALVE A-WAY ON");
  
    else
      Serial.println("POOL VALVE A-WAY OFF");  

    if ( digitalRead( __pool_valve_B_pin__ ) == LOW)
      Serial.println("POOL VALVE B-WAY ON");
  
    else
      Serial.println("POOL VALVE B-WAY OFF");

/*
    Serial.println("========================PARAMETERS===========================");

    Serial.println("(+) BOILER HEATING:");
    Serial.print("  TEMPERATURE SET: ");
    Serial.print( __setup_value_radiant__ - __lower_threshold_radiant__ );
    Serial.print( " -> " );
    Serial.print( __setup_value_radiant__  );
    Serial.print( " <- " );
    Serial.println( __setup_value_radiant__+__upper_threshold_radiant__  );
    Serial.println("");     
    
    Serial.println("(+) HOT WATER TANK:");
    Serial.print("  TEMPERATURE SET: ");
    Serial.print( __setup_value_tank__ - __lower_threshold_tank__ );
    Serial.print( " -> " );
    Serial.print( __setup_value_tank__  );
    Serial.print( " <- " );
    Serial.println( __setup_value_tank__ + __upper_threshold_tank__  );
    Serial.println("");

    Serial.println("(+) SOLAR WATER CIRCUIT:");
    Serial.print("  TEMPERATURE SET: ");
    Serial.print( __setup_value_AB__ - __lower_threshold_AB__ );
    Serial.print( " -> " );
    Serial.print( __setup_value_AB__  );
    Serial.print( " <- " );
    Serial.println( __setup_value_AB__+__upper_threshold_AB__  );
    Serial.println("");    

    Serial.println("(+) SOLAR PUMP: ");
    Serial.print("  SWEEPING ON TIME SET: ");
    Serial.print(__solar_check_time_on__ / 1000.0);
    Serial.println("sec");
    Serial.print("  SWEEPING OFF TIME SET: ");
    Serial.print(__solar_check_time_off__/1000.0);
    Serial.println("sec");
    Serial.print("  MINIMUM EXTERNAL TEMPERATURE SET: ");
    Serial.println( __solar_min_temperature__ );
    Serial.println("");

    Serial.println("(+) HOT WATER RETURN PUMP: ");
    Serial.print("  TIME ON SET: ");
    Serial.print( __return_check_time_on__/1000.0 );
    Serial.println("sec");
    Serial.print("  TIME OFF SET: ");
    Serial.print( __return_check_time_off__ /1000.0);
    Serial.println("sec");
    Serial.println("");

    Serial.println("(+) VALVES: ");
    Serial.print("  RF VALVE POSITION: ");
    if ( aRF_Valve -> getPosition() == _A_ )
        Serial.println( "A - TO HOT WATER TANK)" );

    else if ( aRF_Valve -> getPosition() == _B_ )
        Serial.println( "B - TO RADIANT FLOOR)" );

    Serial.print("  POOL VALVE POSITION: ");
    if ( aPoolValve -> getPosition() == _A_ )
        Serial.println( "A - TO HOT WATER TANK)" );

    else if ( aPoolValve -> getPosition() == _B_ )
        Serial.println( "B - TO POOL)" );

    Serial.print("  VALVES TIME ON SET: ");
    Serial.print( __time_valves_on__/1000.0 );
    Serial.println("sec");   
    Serial.println("");

    Serial.println("(+) EXTRACTORS: ");
    Serial.print("  TIME ON SET: ");
    Serial.print( __extractor_on_time__/1000.0 );  
    Serial.println("sec");  
    Serial.print("  TIME OFF SET: ");
    Serial.print( __extractor_off_time__/1000.0 );
    Serial.println("sec");

    Serial.print("  EXTRACTOR HUMIDITY -1 VS EXTERNAL ON SET: ");
    Serial.println(__start_external_vs_1__);

    Serial.print("  EXTRACTOR HUMIDITY -1 VS EXTERNAL OFF SET: ");
    Serial.println(__stop_external_vs_1__);
    
    Serial.print("  EXTRACTOR HUMIDITY -2 VS EXTERNAL ON SET: ");
    Serial.println(__start_external_vs_2__);

    Serial.print("  EXTRACTOR HUMIDITY -2 VS EXTERNAL OFF SET: ");
    Serial.println(__stop_external_vs_2__);
*/

//    if ( millis() >  )
//      Serial.println("WARNING: millis() has reset up");

    Serial.println("");
    Serial.println("");
    Serial.println("");

    aPreviousMillis = millis();
  }
}

