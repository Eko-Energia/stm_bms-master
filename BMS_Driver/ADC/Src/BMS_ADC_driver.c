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
#include "BMS_CAN_driver.h"

HAL_StatusTypeDef BMS_ADC_Init(BMS_TypeDef* bms){


	// Init ADC
	if(ADC_Init(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1) != HAL_OK){
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

	volatile float voltage_f = 0.0f; // real type of voltage on PC2
	volatile HAL_StatusTypeDef voltage_read_status;

	// calculating real value of voltage
	voltage_read_status = ADC_Get_PinVoltage(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, ADC_CHANNEL_12, (float *)&voltage_f);
	if(voltage_read_status != HAL_OK){
		return HAL_ERROR;
	}

	volatile float vcc_f = ((R1 / R2) + 1.0f) * (voltage_f + 0.38f);
	volatile float vcc_supply_f = 0.0f;

	/* Piecewise-linear calibration from measured divider voltage to pack voltage. */
	static const float measured_vcc[] = {
		53.31f, 59.60f, 60.50f, 60.80f, 61.00f,
		61.20f, 61.70f, 62.00f, 62.40f, 62.80f,
		63.10f, 63.50f, 63.80f, 64.10f, 64.40f,
		64.70f, 65.00f, 65.30f, 65.60f, 65.80f,
		66.10f, 66.40f, 66.60f, 66.90f, 67.10f,
		67.30f, 67.50f, 67.80f, 68.00f, 87.00f, 89.40f
	};
	static const float supply_vcc[] = {
		47.00f, 47.00f, 58.00f, 60.00f, 60.50f,
		62.00f, 63.00f, 64.00f, 65.00f, 66.00f,
		67.00f, 68.00f, 69.00f, 70.00f, 71.00f,
		72.00f, 73.00f, 74.00f, 75.00f, 76.00f,
		77.00f, 78.00f, 79.00f, 80.00f, 81.00f,
			82.00f, 83.00f, 84.00f, 85.00f, 86.00f, 87.00f
	};
	const uint8_t calibration_points = sizeof(measured_vcc) / sizeof(measured_vcc[0]);

	if (vcc_f <= measured_vcc[0])
	{
		vcc_supply_f = supply_vcc[0] + (vcc_f - measured_vcc[0]);
	}
	else
	{
		uint8_t i;

		for (i = 1; i < calibration_points; i++)
		{
			if (vcc_f <= measured_vcc[i])
			{
				vcc_supply_f = supply_vcc[i - 1] +
					(supply_vcc[i] - supply_vcc[i - 1]) *
					(vcc_f - measured_vcc[i - 1]) /
					(measured_vcc[i] - measured_vcc[i - 1]);
				break;
			}
		}

		if (i == calibration_points)
		{
			/* Continue the final calibration slope instead of clamping to 87 V. */
			vcc_supply_f = supply_vcc[calibration_points - 2] +
				(supply_vcc[calibration_points - 1] - supply_vcc[calibration_points - 2]) *
				(vcc_f - measured_vcc[calibration_points - 2]) /
				(measured_vcc[calibration_points - 1] - measured_vcc[calibration_points - 2]);
		}
	}


	// scaling real value with factor and offset
	if(BMS_CAN_ScallingParams(bms, ADC_VOLTAGE_CH, vcc_supply_f) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_ADC_Read_Temperature(BMS_TypeDef* bms){

	float voltage_f     = 0.0f;		// real value of voltage on stm32's pin: PC0
	volatile float Rt   = 0.0f;		// resistance of thermistor
	float temperature_f = 0.0f;		// calculated temperature

	// calculating voltage before voltage divider
	if(ADC_Get_PinVoltage(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1, ADC_TEMP_CH, &voltage_f) != HAL_OK){
		return BMS_CAN_ScallingParams(bms, ADC_TEMP_CH, 0.0f);
	}

	if(voltage_f <= 0.05f || voltage_f >= (STM32_VCC - 0.05f)){
		return BMS_CAN_ScallingParams(bms, ADC_TEMP_CH, 0.0f);
	}
	Rt = NTC_LOWER_OHM * (STM32_VCC / voltage_f - 1.0f);

	// Calculating and calibrating temperature
	temperature_f = BMS_ADC_NTC_GetTemperature(Rt);

#ifdef PROD
	// security check if calculated temperature exceeds calculations range
	if(temperature_f > TEMP_MAX || temperature_f < TEMP_MIN){

		// Report to EH
		EH_report(&bms->beh, EH_BMS_TEMP_HIGH, ERROR_SEVERITY_SAFE_STATE);

		return HAL_ERROR;
	}
#endif

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

	// Reserved for ADC current measurement tests (no active test code here).
	// calculating real value of current
	current_f = ((float)current_b - 2108.0f)/4.0f;

	// converting real value into value, ready to be send via CAN1
	if(BMS_CAN_ScallingParams(bms, ADC_CURRENT_CH, current_f) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}


float BMS_ADC_NTC_GetTemperature(float Rt) {


	// table of temperatures
    static float T[101] = {
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
    static float R[101] = {
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
    for (uint8_t i = 0; i < 100; i++) {

    	// checking if exact R is higher and lower to the closes resistance from table
        if (Rt <= R[i] && Rt >= R[i + 1]) {

        	// returning interpolated value of temperature
            return T[i] + (T[i+1] - T[i]) * (Rt - R[i]) / (R[i + 1]-R[i]);


        }


    }


    return -1000.0f; // error
}
