/*
 * capacitive_sensor.c
 *
 *  Created on: 31 juil. 2026
 *      Author: valentin
 */

#include "capacitive_sensor.h"

uint32_t read_capa_value(TSC_HandleTypeDef *htsc)
{
	uint32_t read_value = 0;

	HAL_TSC_IODischarge(htsc, ENABLE);
	HAL_Delay(1);

	// Démarrage de la mesure
	if (HAL_TSC_Start(htsc) != HAL_OK)
	{
		return 0;
	}

	// Attente de la fin de mesure (avec timeout de 10ms pour éviter de bloquer)
	if (HAL_TSC_PollForAcquisition(htsc) != HAL_OK)
	{
		return 0;
	}

	// Vérification et lecture du résultat
	if (HAL_TSC_GroupGetStatus(htsc, TSC_GROUP1_IDX) == TSC_GROUP_COMPLETED)
	{
		read_value = HAL_TSC_GroupGetValue(htsc, TSC_GROUP1_IDX);
	}

	return read_value;
}
