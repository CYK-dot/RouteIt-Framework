/**
 * @file rti_vlan.c
 * @author CYK-Dot
 * @brief Brief description
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025CYK-Dot, All Rights Reserved.
 */

/* Header import ------------------------------------------------------------------*/
#include "rti_vlan.h"

/* Private typedef ----------------------------------------------------------------*/

/* Private defines ----------------------------------------------------------------*/

/* Global variables ---------------------------------------------------------------*/
extern RTI_VLAN_DESC __start_rti_vlan[];
extern RTI_VLAN_DESC __stop_rti_vlan[];

/* Private function prototypes ---------------------------------------------------*/

/* Exported function prototypes --------------------------------------------------*/

/* Private function definitions --------------------------------------------------*/

/* Exported function definitions -------------------------------------------------*/

/**
 * @brief Select a VLAN by ID.
 * 
 * @param vlanId The ID of the VLAN to select.
 * @param vlanOut Pointer to store the selected VLAN description.
 * @return RTI_ERR Error code indicating success or failure.
 * @note this function is only for RTI internal use.
 */
RTI_ERR RTIPriv_VlanSelect(RTI_VlanId vlanId, RTI_VLAN_DESC *vlanOut)
{
    RTI_ERR err = RTI_OK;
    RTI_VLAN_DESC *itr = __start_rti_vlan;
    if (__start_rti_vlan == __stop_rti_vlan)
    {
        return RTI_ERR_OBJECT_EMPTY;
    }
    while (itr < __stop_rti_vlan)
    {
        if (itr->id == vlanId)
        {
            *vlanOut = *itr;
            break;
        }
        itr++;
    }
    if (itr == __stop_rti_vlan)
    {
        err = RTI_ERR_INVALID_PARAM;
    }
    return err;
}