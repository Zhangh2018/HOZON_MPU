/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "HOZON"
 * 	found in "HOZON_PRIV_v1.0.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_FaultCodeClearanceReqInfo_H_
#define	_FaultCodeClearanceReqInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FaultCodeClearanceReqInfo */
typedef struct FaultCodeClearanceReqInfo {
	long	 diagType;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} FaultCodeClearanceReqInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_FaultCodeClearanceReqInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _FaultCodeClearanceReqInfo_H_ */
#include <asn_internal.h>
