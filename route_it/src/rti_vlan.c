/**
 * @file rti_vlan.c
 * @author CYK-Dot
 * @brief RouteIt-Framework VLAN module implementation.
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025 CYK-Dot, MIT License.
 */

/* Header import ------------------------------------------------------------------*/
#include "rti_vlan.h"
#include <string.h>
#include <stdbool.h>

/* Private typedef ----------------------------------------------------------------*/

/* Private defines ----------------------------------------------------------------*/

/**
 * @brief Get the ID of VLAN description from VLAN record.
 * 
 * @param RECORD[RTI_VLAN_RECORD*] The VLAN record from VLAN table.
 * @return RTI_VlanId The ID of VLAN description.
 */
#define RTI_VLAN_GET_DESC_ID(RECORD) ((RTI_VLAN_DESC*)(RECORD))->id

/**
 * @brief Get the pointer to VLAN description from VLAN record.
 * 
 * @param RECORD[RTI_VLAN_RECORD*] The VLAN record from VLAN table.
 * @return RTI_VLAN_DESC* The reference of VLAN description.
 */
#define RTI_VLAN_GET_DESC_REF(RECORD) ((RTI_VLAN_DESC*)(RECORD))

/* Global variables ---------------------------------------------------------------*/

/* external variables from linker script */
extern RTI_VLAN_RECORD __start_rti_vlan[]; 
extern RTI_VLAN_RECORD __end_rti_vlan[];
#if RTI_ENABLE_DYNAMIC_VLAN == 1
static RTI_VLAN_RECORD *g_RTI_vlanTableStartPtr = __start_rti_vlan;
static RTI_VLAN_RECORD *g_RTI_vlanTableEndPtr = __end_rti_vlan;
static size_t g_RTI_vlanRecordUsedCnt =0;
#endif

/* Private function prototypes ---------------------------------------------------*/

/* Exported function prototypes --------------------------------------------------*/

/* Private function definitions --------------------------------------------------*/

/**
 * @brief Check if the dynamic VLAN table is uninitialized.
 * 
 * @return true The dynamic VLAN table is uninitialized.
 * @return false The dynamic VLAN table is initialized.
 */
RTI_FORCE_INLINE static bool RTI_VlanIsDynamicUninitialized(void)
{
    return (g_RTI_vlanTableStartPtr == __start_rti_vlan || g_RTI_vlanTableEndPtr == __end_rti_vlan);
}

/**
 * @brief Get record count of VLAN table.
 * 
 * @return size_t The record count of VLAN table.
 * @note this function should be called after dynamic table setup.
 */
RTI_FORCE_INLINE static size_t RTI_VlanGetRecordCount(RTI_VLAN_RECORD *recordStart, RTI_VLAN_RECORD *recordEnd)
{
    return (((size_t)(recordEnd) - (size_t)(recordStart)) / sizeof(RTI_VLAN_RECORD));
}

/* Exported function definitions -------------------------------------------------*/

/**
 * @brief Select a VLAN by ID.
 * 
 * @param vlanId The ID of the VLAN to select.
 * @param vlanOut Pointer to store the selected VLAN description.
 * @return RTI_ERR Error code indicating success or failure.
 * @note this function is only for RTI internal use.
 * @warning do not pass pointer variable to vlanOut.
 *          instead, you shold pass the address of RTI_VLAN_DESC variable
 */
RTI_ERR RTIPriv_VlanSelect(RTI_VlanId vlanId, RTI_VLAN_DESC *descOut)
{
    RTI_ERR err = RTI_OK;
    #if RTI_ENABLE_DYNAMIC_VLAN == 1
        RTI_VLAN_RECORD *itr = g_RTI_vlanTableStartPtr;
        RTI_VLAN_RECORD *end = g_RTI_vlanTableEndPtr;
    #else
        RTI_VLAN_RECORD *itr = __start_rti_vlan;
        RTI_VLAN_RECORD *end = __end_rti_vlan;
    #endif
    if (itr == end) {
        return RTI_ERR_OBJECT_EMPTY;
    }
    while (itr < end) {
        // data may unregister,skip it
        if (*itr == NULL) {
            itr++;
            continue;
        }
        if (RTI_VLAN_GET_DESC_ID(*itr) == vlanId) {
            // copy VLAN description from record to output
            memcpy(descOut, RTI_VLAN_GET_DESC_REF(*itr), sizeof(RTI_VLAN_DESC));
            break;
        }
        itr++;
    }
    if (itr == end) {
        err = RTI_ERR_INVALID_PARAM;
    }
    return err;
}

#if RTI_ENABLE_DYNAMIC_VLAN == 1
/**
 * @brief Setup the dynamic VLAN table.
 * 
 * @param start Pointer to the start of the VLAN table.
 * @param sizeBytes Size of the VLAN table in bytes.
 * @return RTI_ERR Error code indicating success or failure.
 * 
 * @note start is recommended to be aligned to RTI_VLAN_RECORD size.
 *       eg. in C11,use aligned_alloc() to allocate aligned memory
 */
RTI_ERR RTI_VlanDynamicSetup(void *start, size_t sizeBytes)
{
    size_t recordCntOld = 0;
    // static table do not have record count,as it is always filled
    if (RTI_VlanIsDynamicUninitialized() == true) {
        recordCntOld = RTI_VlanGetRecordCount(__start_rti_vlan, __end_rti_vlan);
    }
    else {
        recordCntOld = g_RTI_vlanRecordUsedCnt;
    }
    // check if table size is enough
    if (start == NULL) {
        return RTI_ERR_INVALID_PARAM;
    }
    else if (sizeBytes < RTI_VLAN_VLANTABLE_SIZE(recordCntOld)) {
        return RTI_ERR_VLANTABLE_TOO_SHORT;
    }
    // reset new table
    memset(start, 0, sizeBytes);
    // copy to new table
    memcpy(start, g_RTI_vlanTableStartPtr, RTI_VLAN_VLANTABLE_SIZE(recordCntOld));
    g_RTI_vlanTableStartPtr = start;
    g_RTI_vlanTableEndPtr = (RTI_VLAN_RECORD*)(start + sizeBytes);
    g_RTI_vlanRecordUsedCnt = recordCntOld;
    return RTI_OK;
}

/**
 * @brief Register a dynamic VLAN.
 * 
 * @param vlan Pointer to the VLAN description to register.
 * @return RTI_ERR Error code indicating success or failure.
 * @warning 
 *       vlanDesc should be a pointer to a global/static/heap variables,
 *       do not pass a pointer to a stack variable.
 *       because the VLAN-Table do not copy the VLAN description,
 *       it only store the pointer to the VLAN description.
 */
RTI_ERR RTI_VlanDynamicRegister(RTI_VLAN_DESC *vlanDesc)
{
    if (vlanDesc == NULL)
    {
        return RTI_ERR_INVALID_PARAM;
    }
    else if (g_RTI_vlanRecordUsedCnt >= RTI_VlanGetRecordCount(g_RTI_vlanTableStartPtr, g_RTI_vlanTableEndPtr))
    {
        return RTI_ERR_VLANTABLE_OVERFLOW;
    }
    else if (RTI_VlanIsDynamicUninitialized() == true)
    {
        return RTI_ERR_VLANTABLE_NOT_SETUP;
    }
    g_RTI_vlanTableStartPtr[g_RTI_vlanRecordUsedCnt++] = (RTI_VLAN_RECORD)vlanDesc;
    return RTI_OK;
}

/**
 * @brief Check if a VLAN is registered.
 * 
 * @param vlan Pointer to the VLAN description to check.
 * @return RTI_ERR Error code indicating success or failure.
 */
RTI_ERR RTI_VlanDynamicIsRegister(const RTI_VLAN_DESC *vlan)
{
    if (vlan == NULL)
    {
        return RTI_ERR_INVALID_PARAM;
    }
    RTI_VLAN_RECORD *itr = g_RTI_vlanTableStartPtr;
    RTI_VLAN_RECORD *end = g_RTI_vlanTableEndPtr;
    while (itr < end) {
        if (RTI_VLAN_GET_DESC_ID(*itr) == RTI_VLAN_GET_DESC_ID((RTI_VLAN_RECORD)vlan)) {
            return RTI_OK;
        }
        itr++;
    }
    return RTI_ERR_INVALID_PARAM;
}

/**
 * @brief Get the number of free VLAN records.
 * 
 * @param freeRecord Pointer to store the number of free VLAN records.
 * @return RTI_ERR Error code indicating success or failure.
 */
RTI_ERR RTI_VlanDynamicGetFreeCount(size_t *freeRecord)
{
    if (freeRecord == NULL)
    {
        return RTI_ERR_INVALID_PARAM;
    }
    if (RTI_VlanIsDynamicUninitialized() == true)
    {
        *freeRecord = 0;
        return RTI_ERR_VLANTABLE_NOT_SETUP;
    }
    *freeRecord = RTI_VlanGetRecordCount(g_RTI_vlanTableStartPtr, g_RTI_vlanTableEndPtr) - g_RTI_vlanRecordUsedCnt;
    return RTI_OK;
}

/**
 * @brief Get the number of all VLAN records.
 * 
 * @param allRecordCount Pointer to store the number of all VLAN records.
 * @return RTI_ERR Error code indicating success or failure.
 */
RTI_ERR RTI_VlanDynamicGetAllCount(size_t *allRecordCount)
{
    if (allRecordCount == NULL)
    {
        return RTI_ERR_INVALID_PARAM;
    }
    if (RTI_VlanIsDynamicUninitialized() == true)
    {
        *allRecordCount = 0;
        return RTI_ERR_VLANTABLE_NOT_SETUP;
    }
    *allRecordCount = RTI_VlanGetRecordCount(g_RTI_vlanTableStartPtr, g_RTI_vlanTableEndPtr);
    return RTI_OK;
}
#endif

/**
 * @brief Get the pointer to VLAN table start.
 * 
 * @return RTI_VLAN_RECORD* The pointer to VLAN table start.
 */
RTI_VLAN_RECORD *RTIDFX_VlanGetTableAddr(void)
{
    return g_RTI_vlanTableStartPtr;
}
/**
 * @brief force set VLAN table start pointer,but not copy records from old table.
 * 
 * @param start Pointer to the start of the VLAN table.
 * @param sizeBytes Size of the VLAN table in bytes.
 * @return RTI_ERR 
 */
RTI_ERR RTIDFX_VlanTableForceSet(RTI_VLAN_RECORD *start, size_t sizeBytes)
{
    if (start == NULL) {
        return RTI_ERR_INVALID_PARAM;
    }
    g_RTI_vlanTableStartPtr = start;
    g_RTI_vlanTableEndPtr = (RTI_VLAN_RECORD*)(start + sizeBytes);
    return RTI_OK;
}
/**
 * @brief Get the maximum record count of VLAN table.
 * 
 * @return size_t The maximum record count of VLAN table.
 */
size_t RTIDFX_VlanGetTableRecordMaxCount(void)
{
    return RTI_VlanGetRecordCount(g_RTI_vlanTableStartPtr, g_RTI_vlanTableEndPtr);
}
/**
 * @brief Unregister a dynamic VLAN.
 * 
 * @param id VLAN ID to unregister.
 * @return RTI_ERR Error code indicating success or failure.
 * @warning do not unregister static records, this function will not check that.
 */
RTI_ERR RTIDFX_VlanTableUnregister(RTI_VlanId id)
{
    RTI_VLAN_RECORD *itr = g_RTI_vlanTableStartPtr;
    RTI_VLAN_RECORD *end = g_RTI_vlanTableEndPtr;
    while (itr < end) {
        if (RTI_VLAN_GET_DESC_ID(*itr) == id) {
            memmove(itr, itr + 1, (end - itr - 1) * sizeof(RTI_VLAN_RECORD));
            g_RTI_vlanRecordUsedCnt--;
            // reset last record to NULL
            g_RTI_vlanTableStartPtr[g_RTI_vlanRecordUsedCnt] = NULL;
            return RTI_OK;
        }
        itr++;
    }
    return RTI_ERR_INVALID_PARAM;
}