/**
 * @file rti_vlan.h
 * @author CYK-Dot
 * @brief Brief description
 * @version 0.1
 * @date 2025-10-04
 *
 * @copyright Copyright (c) 2025 CYK-Dot, MIT License.
 */
#pragma once

/* Header import ------------------------------------------------------------------*/
#include "rti_internal.h"

/* Config macros -----------------------------------------------------------------*/

/* Export macros -----------------------------------------------------------------*/

/**
 * @brief Register a static VLAN with VLAN ID auto destributed by python script.
 * 
 * @param VLAN_IFX_ADDRESS VLAN interface address,should be a pointer
 * @param VLAN_NAME VLAN name.
 */
#define RTI_VLAN_REGISTER_STATIC(VLAN_IFX_ADDRESS, VLAN_NAME) \
    const RTI_VLAN_DESC RTI_VLAN_##VLAN_NAME = { \
        .ifx = VLAN_IFX_ADDRESS, \
        .name = #VLAN_NAME, \
        .id = RTI_VLANID_##VLAN_NAME, \
    };\
    RTI_TYPE_SECTION_VLAN_USED const RTI_VLAN_DESC *RTI_VLAN_##VLAN_NAME##_PTR = &RTI_VLAN_##VLAN_NAME

/**
 * @brief Register a static VLAN with specified VLAN ID.
 * 
 * @param VLAN_IFX_ADDRESS VLAN interface address,should be a pointer
 * @param VLAN_NAME VLAN name.
 * @param VLAN_ID VLAN ID.
 */
#define RTI_VLAN_REGISTER_STATIC_WITH_ID(VLAN_IFX_ADDRESS, VLAN_NAME, VLAN_ID) \
    const RTI_VLAN_DESC RTI_VLAN_##VLAN_NAME = { \
        .ifx = VLAN_IFX_ADDRESS, \
        .name = #VLAN_NAME, \
        .id = VLAN_ID, \
    };\
    RTI_TYPE_SECTION_VLAN_USED const RTI_VLAN_DESC *RTI_VLAN_##VLAN_NAME##_PTR = &RTI_VLAN_##VLAN_NAME

/**
 * @brief Get the size of VLAN table in bytes.
 * 
 * @param RECORD_COUNT Number of VLAN records in the table.
 * @return size_t Size of VLAN table in bytes.
 */
#define RTI_VLAN_VLANTABLE_SIZE(RECORD_COUNT) (sizeof(RTI_VLAN_RECORD) * (RECORD_COUNT))

/* Exported typedef --------------------------------------------------------------*/

typedef void* (*RTI_VlanCreateFptr)(void);
typedef void (*RTI_VlanDeleteFptr)(void* vlan);
typedef void* (*RTI_VlanCreateProducerFptr)(void);
typedef void (*RTI_VlanDeleteProducerFptr)(void* producer);
typedef void* (*RTI_VlanCreateConsumerFptr)(void);
typedef void (*RTI_VlanDeleteConsumerFptr)(void* consumer);
typedef uint16_t RTI_VlanId;

/**
 * @brief VLAN interface structure.
 * 
 */
typedef struct {
    RTI_VlanCreateFptr createF;
    RTI_VlanDeleteFptr deleteF;
    RTI_VlanCreateProducerFptr createProducerF;
    RTI_VlanDeleteProducerFptr deleteProducerF;
    RTI_VlanCreateConsumerFptr createConsumerF;
    RTI_VlanDeleteConsumerFptr deleteConsumerF;
} RTI_VLAN_IFX;

/**
 * @brief VLAN description structure.
 * 
 */
typedef struct {
    RTI_VLAN_IFX *ifx;
    char *name;
    RTI_VlanId id;
} RTI_VLAN_DESC;

/**
 * @brief VLAN record in VLAN-Table
 * @note VLAN table dont records VLAN description directly.
 *       instead, it records pointers to VLAN description.
 */
typedef RTI_VLAN_DESC* RTI_VLAN_RECORD;

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* Exported function -------------------------------------------------------------*/

/* RTI private functions */
RTI_ERR RTIPriv_VlanSelect(RTI_VlanId id, RTI_VLAN_DESC *descOut);

/* RTI exported functions */
#if RTI_ENABLE_DYNAMIC_VLAN == 1
RTI_ERR RTI_VlanDynamicSetup(void *start, size_t sizeBytes);
RTI_ERR RTI_VlanDynamicRegister(RTI_VLAN_DESC *vlan);
RTI_ERR RTI_VlanDynamicIsRegister(const RTI_VLAN_DESC *vlan);
RTI_ERR RTI_VlanDynamicGetFreeCount(size_t *freeRecordCount);
RTI_ERR RTI_VlanDynamicGetAllCount(size_t *allRecordCount);
#endif

/* RTI DFX functions */
RTI_VLAN_RECORD *RTIDFX_VlanGetTableAddr(void);
size_t RTIDFX_VlanGetTableRecordMaxCount(void);
RTI_ERR RTIDFX_VlanTableForceSet(RTI_VLAN_RECORD *start, size_t sizeBytes);
RTI_ERR RTIDFX_VlanTableUnregister(RTI_VlanId id);

/* C++ ---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif