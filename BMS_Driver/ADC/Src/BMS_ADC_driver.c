/**
  ******************************************************************************
  * @file      BMS.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's ADC peripheral
  * @brief     Functions contain logical implementation of BMS ADC workflow.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "BMS_ADC_driver.h"

HAL_StatusTypeDef BMS_ADC_Init(BMS_TypeDef* bms){


	// Init ADC
	if(ADC_Init(bms->bmsADC.hadc, &bms->bmsADC.badc1, &bms->bmsADC.cadc1) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}


HAL_StatusTypeDef BMS_ADC_ReadValues(BMS_TypeDef* bms){

	// reading voltage | OK
	if(BMS_ADC_Read_Voltage(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// reading temperature | Needs tests when physical thermistor is connected
	if(BMS_ADC_Read_Temperature(bms) != HAL_OK){
		return HAL_ERROR;
	}


	// reading current
	if(BMS_ADC_Read_Current(bms) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_ADC_Read_Voltage(BMS_TypeDef* bms){

	uint16_t voltage_b = 0; // binary type of voltage
	float voltage_f = 0.0f; // real type of voltage

	// reading channel's value
	if(ADC_ReadChannel(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, ADC_VOLTAGE_CH, &voltage_b) != HAL_OK){
		return HAL_ERROR;
	}

	// calculating real value of voltage
	if(ADC_GetValue(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, VCC_SUPPLY_VOLTAGE, ADC_CHANNEL_12, &voltage_f) != HAL_OK){
		return HAL_ERROR;
	}


	// scaling real value with factor and offset
	if(BMS_CAN_ScallingParams(bms, ADC_VOLTAGE_CH, &voltage_f) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_ADC_Read_Temperature(BMS_TypeDef* bms){

	float voltage_f     = 0.0f;		// real value of voltage on stm32's pin: PC0
	float Rt            = 0.0f;		// resistance of thermistor
	float temperature_f = 0.0f;		// calculated temperature

	uint16_t voltage_b  = 0;		// binary value of voltage on stm32's pin: PC0


	// reading value
	if(ADC_ReadChannel(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, ADC_TEMP_CH, &voltage_b) != HAL_OK){
		return HAL_ERROR;
	}

	// calculating voltage before voltage divider
	if(ADC_GetValue(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, VCC_SUPPLY_VOLTAGE, ADC_TEMP_CH, &voltage_f) != HAL_OK){
		return HAL_ERROR;
	}

	// calculating resistance
	Rt = 1000/voltage_f * (VCC_SUPPLY_VOLTAGE - voltage_f);

	// Calculating and calibrating temperature
	temperature_f = BMS_ADC_NTC_calibrateTemperature(BMS_ADC_NTC_GetTemperature(Rt));

	// security check if calculated temperature exceeds calculations range
	if(temperature_f > 100 || temperature_f < 0){
		return HAL_ERROR;
	}

	// converting real value of temperature with factor and offset to achieve type of value, which is ready to be sent via CAN1
	if(BMS_CAN_ScallingParams(bms, ADC_TEMP_CH, temperature_f) != HAL_OK){
		return HAL_ERROR;
	}


	return HAL_OK;
}

HAL_StatusTypeDef BMS_ADC_Read_Current(BMS_TypeDef* bms){
	uint16_t current_b = 0;     // binary type of current
	float current_f    = 0.0f;  // real type of current

	// reading channel's value
	if(ADC_ReadChannel(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, ADC_CURRENT_CH, &current_b) != HAL_OK){
		return HAL_ERROR;
	}

	// calculating real value of current
	current_f = ((float)current_b - 2108.0f)/4.0f;

	// converting real value into value, ready to be send via CAN1
	if(BMS_CAN_ScallingParams(bms, ADC_CURRENT_CH, current_f) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

float BMS_ADC_NTC_calibrateTemperature(float measured) {
    // Punkty kalibracyjne (dostosuj po swoich pomiarach)
    float T1 = 25.0f;  // rzeczywista temp.
    float M1 = 28.0f;  // zmierzona
    float T2 = 36.6f;
    float M2 = 39.2f;

    // interpolacja liniowa
    return T1 + (measured - M1) * (T2 - T1) / (M2 - M1);
}

float BMS_ADC_NTC_GetTemperature(float Rt) {


	// table of temperatures
    static const float T[101] = {
        0,1,2,3,4,5,6,7,8,9,10,
        11,12,13,14,15,16,17,18,19,20,
        21,22,23,24,25,26,27,28,29,30,
        31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,
        51,52,53,54,55,56,57,58,59,60,
        61,62,63,64,65,66,67,68,69,70,
        71,72,73,74,75,76,77,78,79,80,
        81,82,83,84,85,86,87,88,89,90,
        91,92,93,94,95,96,97,98,99,100
    };


    // table of Resistances
    static const float R[101] = {
        27515,26344,25230,24169,23159,22197,21281,20407,19574,18780,
        18017,17300,16611,15953,15324,14724,14150,13602,13079,12578,
        12099,11642,11204,10785,10384,10000,9632,9280,8942,8619,
        8309,8012,7727,7453,7191,6940,6699,6467,6244,6030,
        5825,5628,5438,5256,5081,4913,4751,4595,4445,4301,
        4168,4029,3900,3776,3657,3542,3431,3324,3222,3122,
        3027,2935,2845,2760,2677,2597,2520,2445,2373,2304,
        2237,2172,2109,2049,1991,1934,1879,1826,1775,1725,
        1678,1632,1586,1543,1501,1461,1421,1383,1346,1310,
        1276,1242,1210,1179,1147,1118,1090,1061,1034,1008,
        983
    };




    // if out of range
    if (Rt >= R[0]) return T[0];        // colder than 0°C
    if (Rt <= R[100]) return T[100];    // warmer than 100°C



    // search for closes restistance
    for (int i = 0; i < 100; i++) {


    	// checking if exact R is higher and lower to the closes resistance from table
        if (Rt <= R[i] && Rt >= R[i+1]) {

        	// returning interpolated value of temperature
            return T[i] + (T[i+1]-T[i]) * (Rt-R[i]) / (R[i+1]-R[i]);


        }


    }


    return -1000.0f; // error
}
