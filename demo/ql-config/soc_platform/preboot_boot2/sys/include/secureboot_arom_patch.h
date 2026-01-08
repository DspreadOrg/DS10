#ifndef SECUREBOOT_AROM_PATCH_H_
#define SECUREBOOT_AROM_PATCH_H_

/*---------------------------------------------------------------------------*/
/* tbbr_oid.h */
/*---------------------------------------------------------------------------*/
/*
 * The following is a list of OID values defined and reserved by ARM, which
 * are used to define the extension fields of the certificate structure, as
 * defined in the Trusted Board Boot Requirements (TBBR) specification,
 * ARM DEN0006C-1.
 */


/* TrustedFirmwareNVCounter - Non-volatile counter extension */
#define TRUSTED_FW_NVCOUNTER_OID "1.3.6.1.4.1.4128.2100.1"
/* NonTrustedFirmwareNVCounter - Non-volatile counter extension */
#define NON_TRUSTED_FW_NVCOUNTER_OID "1.3.6.1.4.1.4128.2100.2"


/*
 * Non-Trusted Firmware Updater Certificate
 */

/* APFirmwareUpdaterConfigHash - BL2U */
#define AP_FWU_CFG_HASH_OID "1.3.6.1.4.1.4128.2100.101"
/* SCPFirmwareUpdaterConfigHash - SCP_BL2U */
#define SCP_FWU_CFG_HASH_OID "1.3.6.1.4.1.4128.2100.102"
/* FirmwareUpdaterHash - NS_BL2U */
#define FWU_HASH_OID "1.3.6.1.4.1.4128.2100.103"
/* TrustedWatchdogRefreshTime */
#define TRUSTED_WATCHDOG_TIME_OID "1.3.6.1.4.1.4128.2100.104"


/*
 * Trusted Boot Firmware Certificate
 */

/* TrustedBootFirmwareHash - BL2 */
#define TRUSTED_BOOT_FW_HASH_OID "1.3.6.1.4.1.4128.2100.201"
#define TRUSTED_BOOT_FW_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.202"
#define HW_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.203"

/*
 * Trusted Key Certificate
 */

/* PrimaryDebugCertificatePK */
#define PRIMARY_DEBUG_PK_OID "1.3.6.1.4.1.4128.2100.301"
/* TrustedWorldPK */
#define TRUSTED_WORLD_PK_OID "1.3.6.1.4.1.4128.2100.302"
/* NonTrustedWorldPK */
#define NON_TRUSTED_WORLD_PK_OID "1.3.6.1.4.1.4128.2100.303"


/*
 * Trusted Debug Certificate
 */

/* DebugScenario */
#define TRUSTED_DEBUG_SCENARIO_OID "1.3.6.1.4.1.4128.2100.401"
/* SoC Specific */
#define TRUSTED_DEBUG_SOC_SPEC_OID "1.3.6.1.4.1.4128.2100.402"
/* SecondaryDebugCertPK */
#define SECONDARY_DEBUG_PK_OID "1.3.6.1.4.1.4128.2100.403"


/*
 * SoC Firmware Key Certificate
 */

/* SoCFirmwareContentCertPK */
#define SOC_FW_CONTENT_CERT_PK_OID "1.3.6.1.4.1.4128.2100.501"

/*
 * SoC Firmware Content Certificate
 */

/* APRomPatchHash - BL1_PATCH */
#define APROM_PATCH_HASH_OID "1.3.6.1.4.1.4128.2100.601"
/* SoCConfigHash */
#define SOC_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.602"
/* SoCAPFirmwareHash - BL31 */
#define SOC_AP_FW_HASH_OID "1.3.6.1.4.1.4128.2100.603"
/* SoCFirmwareConfigHash = SOC_FW_CONFIG */
#define SOC_FW_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.604"

/*
 * SCP Firmware Key Certificate
 */

/* SCPFirmwareContentCertPK */
#define SCP_FW_CONTENT_CERT_PK_OID "1.3.6.1.4.1.4128.2100.701"


/*
 * SCP Firmware Content Certificate
 */

/* SCPFirmwareHash - SCP_BL2 */
#define SCP_FW_HASH_OID "1.3.6.1.4.1.4128.2100.801"
/* SCPRomPatchHash - SCP_BL1_PATCH */
#define SCP_ROM_PATCH_HASH_OID "1.3.6.1.4.1.4128.2100.802"


/*
 * Trusted OS Firmware Key Certificate
 */

/* TrustedOSFirmwareContentCertPK */
#define TRUSTED_OS_FW_CONTENT_CERT_PK_OID "1.3.6.1.4.1.4128.2100.901"


/*
 * Trusted OS Firmware Content Certificate
 */

/* TrustedOSFirmwareHash - BL32 */
#define TRUSTED_OS_FW_HASH_OID "1.3.6.1.4.1.4128.2100.1001"
/* TrustedOSExtra1FirmwareHash - BL32 Extra1 */
#define TRUSTED_OS_FW_EXTRA1_HASH_OID "1.3.6.1.4.1.4128.2100.1002"
/* TrustedOSExtra2FirmwareHash - BL32 Extra2 */
#define TRUSTED_OS_FW_EXTRA2_HASH_OID "1.3.6.1.4.1.4128.2100.1003"
/* TrustedOSFirmwareConfigHash - TOS_FW_CONFIG */
#define TRUSTED_OS_FW_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.1004"


/*
 * Non-Trusted Firmware Key Certificate
 */

/* NonTrustedFirmwareContentCertPK */
#define NON_TRUSTED_FW_CONTENT_CERT_PK_OID "1.3.6.1.4.1.4128.2100.1101"


/*
 * Non-Trusted Firmware Content Certificate
 */

/* NonTrustedWorldBootloaderHash - BL33 */
#define NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID "1.3.6.1.4.1.4128.2100.1201"
/* NonTrustedFirmwareConfigHash - NT_FW_CONFIG */
#define NON_TRUSTED_FW_CONFIG_HASH_OID "1.3.6.1.4.1.4128.2100.1202"

/*---------------------------------------------------------------------------*/
/* cot_def.h */
/*---------------------------------------------------------------------------*/
/* TBBR CoT definitions */

#define COT_MAX_VERIFIED_PARAMS 4

/*---------------------------------------------------------------------------*/
/* auth_common.h */
/*---------------------------------------------------------------------------*/
/*
 * Type of parameters that can be extracted from an image and
 * used for authentication
 */
typedef enum auth_param_type_enum {
  AUTH_PARAM_NONE,
  AUTH_PARAM_RAW_DATA,      /* Raw image data */
  AUTH_PARAM_SIG,           /* The image signature */
  AUTH_PARAM_SIG_ALG,       /* The image signature algorithm */
  AUTH_PARAM_HASH,          /* A hash (including the algorithm) */
  AUTH_PARAM_PUB_KEY,       /* A public key */
  AUTH_PARAM_NV_CTR,        /* A non-volatile counter */
} auth_param_type_t;

/*
 * Authentication framework common types
 */
/*
 * Store a pointer to the authentication parameter and its length
 */
typedef struct auth_param_data_desc_s {
    void *ptr;
    unsigned int len;
} auth_param_data_desc_t;
/*
 * Defines an authentication parameter. The cookie will be interpreted by the
 * image parser module.
 */
typedef struct auth_param_type_desc_s {
  auth_param_type_t type;
  void *cookie;
} auth_param_type_desc_t;
/*
 * Authentication parameter descriptor, including type and value
 */
typedef struct auth_param_desc_s {
    auth_param_type_desc_t *type_desc;
    auth_param_data_desc_t data;
} auth_param_desc_t;

/*
 * The method type defines how an image is authenticated
 */
typedef enum auth_method_type_enum {
    AUTH_METHOD_NONE = 0,
    AUTH_METHOD_HASH,   /* Authenticate by hash matching */
    AUTH_METHOD_SIG,    /* Authenticate by PK operation */
    AUTH_METHOD_NV_CTR, /* Authenticate by Non-Volatile Counter */
    AUTH_METHOD_NUM     /* Number of methods */
} auth_method_type_t;

/*
 * Parameters for authentication by hash matching
 */
typedef struct auth_method_param_hash_s {
    auth_param_type_desc_t *data;   /* Data to hash */
    auth_param_type_desc_t *hash;   /* Hash to match with */
} auth_method_param_hash_t;

/*
 * Parameters for authentication by signature
 */
typedef struct auth_method_param_sig_s {
    auth_param_type_desc_t *pk;     /* Public key */
    auth_param_type_desc_t *sig;    /* Signature to check */
    auth_param_type_desc_t *alg;    /* Signature algorithm */
    auth_param_type_desc_t *data;   /* Data signed */
} auth_method_param_sig_t;

/*
 * Parameters for authentication by NV counter
 */
typedef struct auth_method_param_nv_ctr_s {
    auth_param_type_desc_t *cert_nv_ctr;    /* NV counter in certificate */
    auth_param_type_desc_t *plat_nv_ctr;    /* NV counter in platform */
} auth_method_param_nv_ctr_t;

/*
 * Authentication method descriptor
 */
typedef struct auth_method_desc_s {
    auth_method_type_t type;
    union {
        auth_method_param_hash_t hash;
        auth_method_param_sig_t sig;
        auth_method_param_nv_ctr_t nv_ctr;
    } param;
} auth_method_desc_t;

/*
 * Helper macro to define an authentication parameter type descriptor
 */
#define AUTH_PARAM_TYPE_DESC(_type, _cookie) \
    { \
        .type = _type, \
        .cookie = (void *)_cookie \
    }

/*
 * Helper macro to define an authentication parameter data descriptor
 */
#define AUTH_PARAM_DATA_DESC(_ptr, _len) \
    { \
        .ptr = (void *)_ptr, \
        .len = (unsigned int)_len \
    }

/*---------------------------------------------------------------------------*/
/* auth_mode.h */
/*---------------------------------------------------------------------------*/
/*
 * Image flags
 */
#define IMG_FLAG_AUTHENTICATED      (1 << 0)

/**
 * \brief          Public key types
 */
typedef enum {
  MBEDTLS_PK_NONE = 0,
  MBEDTLS_PK_RSA,
  MBEDTLS_PK_ECKEY,
  MBEDTLS_PK_ECKEY_DH,
  MBEDTLS_PK_ECDSA,
  MBEDTLS_PK_RSA_ALT,
  MBEDTLS_PK_RSASSA_PSS,
} mbedtls_pk_type_t;

typedef enum {
  MBEDTLS_MD_NONE = 0,
  MBEDTLS_MD_MD2,
  MBEDTLS_MD_MD4,
  MBEDTLS_MD_MD5,
  MBEDTLS_MD_SHA1,
  MBEDTLS_MD_SHA224,
  MBEDTLS_MD_SHA256,
  MBEDTLS_MD_SHA384,
  MBEDTLS_MD_SHA512,
  MBEDTLS_MD_RIPEMD160,
} mbedtls_md_type_t;
/*
 * Image types. A parser should be instantiated and registered for each type
 */
typedef enum img_type_enum {
  IMG_RAW,              /* Binary image */
  IMG_PLAT,             /* Platform specific format */
  IMG_CERT,             /* X509v3 certificate */
  IMG_MAX_TYPES,
} img_type_t;

/*
 * Authentication image descriptor
 */
typedef struct auth_img_desc_s {
  unsigned int img_id;
  img_type_t img_type;
  const struct auth_img_desc_s *parent;
  auth_method_desc_t img_auth_methods[AUTH_METHOD_NUM];
  auth_param_desc_t authenticated_data[COT_MAX_VERIFIED_PARAMS];
} auth_img_desc_t;
/*---------------------------------------------------------------------------*/
#define return_if_error(rc) \
  do { \
    if(rc != 0) { \
      return rc; \
    } \
  } while(0)

/*******************************************************************************
 * plat_get_rotpk_info() flags
 ******************************************************************************/
#define ROTPK_IS_HASH           (1 << 0)
/* Flag used to skip verification of the certificate ROTPK while the platform
   ROTPK is not deployed */
#define ROTPK_NOT_DEPLOYED      (1 << 1)

/* ASN.1 tags */
#define ASN1_INTEGER                 0x02

/* Pointer to CoT */
extern const auth_img_desc_t *const cot_desc_ptr;

/* Public functions */
int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id);
int auth_mod_verify_img(unsigned int img_id,
                        void *img_ptr,
                        unsigned int img_len);

/* Macro to register a CoT defined as an array of auth_img_desc_t */
#define REGISTER_COT(_cot) \
  const auth_img_desc_t *const cot_desc_ptr = \
    (const auth_img_desc_t *const)&_cot[0]

/*---------------------------------------------------------------------------*/
int arom_img_parser_get_auth_param(img_type_t img_type,
                          const auth_param_type_desc_t *type_desc,
                          void *img_ptr, unsigned int img_len,
                          void **param_ptr, unsigned int *param_len);

int arom_crypto_mod_verify_hash(void *data_ptr, unsigned int data_len,
                       void *digest_info_ptr, unsigned int digest_info_len);

int arom_plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len, unsigned int *flags);

int arom_crypto_mod_verify_signature(void *data_ptr, unsigned int data_len,
                            void *sig_ptr, unsigned int sig_len,
                            void *sig_alg_ptr, unsigned int sig_alg_len,
                            void *pk_ptr, unsigned int pk_len);
int arom_crypto_mod_get_hash_sig_alg(void *sig_alg, unsigned int sig_alg_len,
                            mbedtls_md_type_t *pmd_alg, mbedtls_pk_type_t *ppk_alg);
int arom_img_parser_check_integrity(img_type_t img_type, void *img_ptr, unsigned int img_len);
/*---------------------------------------------------------------------------*/
void secureboot_arom_patch(void);
/*---------------------------------------------------------------------------*/

#endif /* SECUREBOOT_AROM_PATCH_H_ */
