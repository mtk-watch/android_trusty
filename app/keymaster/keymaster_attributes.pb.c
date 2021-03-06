/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9.1 at Thu Nov 29 18:34:44 2018. */

#include "keymaster_attributes.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t KeymasterAttributes_fields[3] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, KeymasterAttributes, uuid, uuid, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, STATIC  , OTHER, KeymasterAttributes, product_id, uuid, 0),
    PB_LAST_FIELD
};

const pb_field_t AttestationKey_fields[3] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, STATIC  , FIRST, AttestationKey, key, key, 0),
    PB_FIELD(  2, MESSAGE , REPEATED, STATIC  , OTHER, AttestationKey, certs, key, &AttestationCert_fields),
    PB_LAST_FIELD
};

const pb_field_t AttestationCert_fields[2] = {
    PB_FIELD(  1, BYTES   , REQUIRED, STATIC  , FIRST, AttestationCert, content, content, 0),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(AttestationKey, key) < 65536 && pb_membersize(AttestationKey, certs[0]) < 65536 && pb_membersize(AttestationCert, content) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_KeymasterAttributes_AttestationKey_AttestationCert)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
#error Field descriptor for AttestationCert.content is too large. Define PB_FIELD_16BIT to fix this.
#endif


/* @@protoc_insertion_point(eof) */
