/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.9.1 at Thu Nov 29 18:34:44 2018. */

#ifndef PB_KEYMASTER_ATTRIBUTES_PB_H_INCLUDED
#define PB_KEYMASTER_ATTRIBUTES_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef PB_BYTES_ARRAY_T(2048) AttestationCert_content_t;
typedef struct _AttestationCert {
    AttestationCert_content_t content;
/* @@protoc_insertion_point(struct:AttestationCert) */
} AttestationCert;

typedef PB_BYTES_ARRAY_T(32) KeymasterAttributes_uuid_t;
typedef PB_BYTES_ARRAY_T(16) KeymasterAttributes_product_id_t;
typedef struct _KeymasterAttributes {
    bool has_uuid;
    KeymasterAttributes_uuid_t uuid;
    bool has_product_id;
    KeymasterAttributes_product_id_t product_id;
/* @@protoc_insertion_point(struct:KeymasterAttributes) */
} KeymasterAttributes;

typedef PB_BYTES_ARRAY_T(2048) AttestationKey_key_t;
typedef struct _AttestationKey {
    bool has_key;
    AttestationKey_key_t key;
    pb_size_t certs_count;
    AttestationCert certs[3];
/* @@protoc_insertion_point(struct:AttestationKey) */
} AttestationKey;

/* Default values for struct fields */

/* Initializer values for message structs */
#define KeymasterAttributes_init_default         {false, {0, {0}}, false, {0, {0}}}
#define AttestationKey_init_default              {false, {0, {0}}, 0, {AttestationCert_init_default, AttestationCert_init_default, AttestationCert_init_default}}
#define AttestationCert_init_default             {{0, {0}}}
#define KeymasterAttributes_init_zero            {false, {0, {0}}, false, {0, {0}}}
#define AttestationKey_init_zero                 {false, {0, {0}}, 0, {AttestationCert_init_zero, AttestationCert_init_zero, AttestationCert_init_zero}}
#define AttestationCert_init_zero                {{0, {0}}}

/* Field tags (for use in manual encoding/decoding) */
#define AttestationCert_content_tag              1
#define KeymasterAttributes_uuid_tag             1
#define KeymasterAttributes_product_id_tag       2
#define AttestationKey_key_tag                   1
#define AttestationKey_certs_tag                 2

/* Struct field encoding specification for nanopb */
extern const pb_field_t KeymasterAttributes_fields[3];
extern const pb_field_t AttestationKey_fields[3];
extern const pb_field_t AttestationCert_fields[2];

/* Maximum encoded size of messages (where known) */
#define KeymasterAttributes_size                 52
#define AttestationKey_size                      8213
#define AttestationCert_size                     2051

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define KEYMASTER_ATTRIBUTES_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
