#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "patchlib.h"
#include "cpu.h"
#include "secureboot_arom_patch.h"
#include "assert.h"
#include "tbbr_img_def.h"

/*---------------------------------------------------------------------------*/
/* CraneGCraneM_A0_AROM_Final_Release_0803, bootrom version: 2020.07.30 */
#define AROM_PATCH_2_ADDRESS_AUTH_MOD_GET_PARENT_ID   (0xd1007244)
#define AROM_PATCH_3_ADDRESS_AUTH_MOD_VERIFY_IMG      (0xd100727C)
/*---------------------------------------------------------------------------*/
asm (
  ".section .patch.rodata\n"
  "cmd_prefix_array: .word .L_complete, .L_erase, .L_flash, .L_fuse, 0\n"
  ".L_complete: .asciz \"complete\"\n"
  ".L_erase: .asciz \"erase:\"\n"
  ".L_flash: .asciz \"flash:\"\n"
  ".L_fuse: .asciz \"fuse\"\n"
  );
/*---------------------------------------------------------------------------*/
/* arom interfaces */
typedef int (*img_parser_get_auth_param_t)(img_type_t img_type,
                                           const auth_param_type_desc_t *type_desc,
                                           void *img_ptr, unsigned int img_len,
                                           void **param_ptr, unsigned int *param_len);
#define IMG_PARSER_GET_AUTH_PARAM       (0xd1007374 + 1)
int
arom_img_parser_get_auth_param(img_type_t img_type,
                               const auth_param_type_desc_t *type_desc,
                               void *img_ptr, unsigned int img_len,
                               void **param_ptr, unsigned int *param_len)
{
  img_parser_get_auth_param_t img_parser_get_auth_param = (img_parser_get_auth_param_t)IMG_PARSER_GET_AUTH_PARAM;
  return img_parser_get_auth_param(img_type, type_desc, \
                                   img_ptr, img_len, param_ptr, param_len);
}
typedef int (*crypto_mod_verify_hash_t)(void *data_ptr, unsigned int data_len,
                                        void *digest_info_ptr, unsigned int digest_info_len);
#define CRYPTO_MOD_VERIFY_HASH       (0xd1007750 + 1)
int
arom_crypto_mod_verify_hash(void *data_ptr, unsigned int data_len,
                            void *digest_info_ptr, unsigned int digest_info_len)
{
  crypto_mod_verify_hash_t crypto_mod_verify_hash = (crypto_mod_verify_hash_t)CRYPTO_MOD_VERIFY_HASH;
  return crypto_mod_verify_hash(data_ptr, data_len, digest_info_ptr, digest_info_len);
}
typedef int (*plat_get_rotpk_info_t)(void *cookie, void **key_ptr, unsigned int *key_len, unsigned int *flags);
#define PLAT_GET_ROTPK_INFO          (0xd1007bf8 + 1)
int
arom_plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len, unsigned int *flags)
{
  plat_get_rotpk_info_t plat_get_rotpk_info = (plat_get_rotpk_info_t)PLAT_GET_ROTPK_INFO;

  return plat_get_rotpk_info(cookie, key_ptr, key_len, flags);
}
typedef int (*crypto_mod_verify_signature_t)(void *data_ptr, unsigned int data_len,
                                             void *sig_ptr, unsigned int sig_len,
                                             void *sig_alg_ptr, unsigned int sig_alg_len,
                                             void *pk_ptr, unsigned int pk_len);
#define CRYPTO_MOD_VERIFY_SIGNATURE   (0xd10076ec + 1)
int
arom_crypto_mod_verify_signature(void *data_ptr, unsigned int data_len,
                                 void *sig_ptr, unsigned int sig_len,
                                 void *sig_alg_ptr, unsigned int sig_alg_len,
                                 void *pk_ptr, unsigned int pk_len)
{
  crypto_mod_verify_signature_t crypto_mod_verify_signature = (crypto_mod_verify_signature_t)CRYPTO_MOD_VERIFY_SIGNATURE;
  return crypto_mod_verify_signature(data_ptr, data_len, sig_ptr, sig_len, \
                                     sig_alg_ptr, sig_alg_len, pk_ptr, pk_len);
}
typedef int (*crypto_mod_get_hash_sig_alg_t)(void *sig_alg, unsigned int sig_alg_len,
                                             mbedtls_md_type_t *pmd_alg, mbedtls_pk_type_t *ppk_alg);
#define CRYPTO_MOD_GET_HASH_SIG_ALG    (0xd10076c0 + 1)
int
arom_crypto_mod_get_hash_sig_alg(void *sig_alg, unsigned int sig_alg_len,
                                 mbedtls_md_type_t *pmd_alg, mbedtls_pk_type_t *ppk_alg)
{
  crypto_mod_get_hash_sig_alg_t crypto_mod_get_hash_sig_alg = (CRYPTO_MOD_GET_HASH_SIG_ALG);
  return crypto_mod_get_hash_sig_alg(sig_alg, sig_alg_len, pmd_alg, ppk_alg);
}
typedef int (*img_parser_check_integrity_t)(img_type_t img_type, void *img_ptr, unsigned int img_len);
#define IMG_PARSER_CHECK_INTEGRITY     (0xd1007338 + 1)
int
arom_img_parser_check_integrity(img_type_t img_type, void *img_ptr, unsigned int img_len)
{
  img_parser_check_integrity_t img_parser_check_integrity = (img_parser_check_integrity_t)IMG_PARSER_CHECK_INTEGRITY;
  return img_parser_check_integrity(img_type, img_ptr, img_len);
}


/*---------------------------------------------------------------------------*/
/* tbbr_cot.c */
/*---------------------------------------------------------------------------*/
/*
 * Maximum key and hash sizes (in DER format)
 */
#define PK_DER_LEN          294
#define HASH_DER_LEN        51

/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */
static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char hw_config_hash_buf[HASH_DER_LEN];
static unsigned char scp_fw_hash_buf[HASH_DER_LEN];
static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra1_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra2_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];
static unsigned char trusted_world_pk_buf[PK_DER_LEN];
static unsigned char non_trusted_world_pk_buf[PK_DER_LEN];
static unsigned char content_pk_buf[PK_DER_LEN];
static unsigned char soc_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char nt_fw_config_hash_buf[HASH_DER_LEN];

/*
 * Parameter type descriptors
 */
static auth_param_type_desc_t trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_NV_CTR, TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);

static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, 0);
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_RAW_DATA, 0);

static auth_param_type_desc_t trusted_world_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);

static auth_param_type_desc_t scp_fw_content_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, SCP_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t tos_fw_content_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, TRUSTED_OS_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);

static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t tb_fw_config_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_BOOT_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t hw_config_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, HW_CONFIG_HASH_OID);
static auth_param_type_desc_t scp_fw_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, SCP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_config_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, SOC_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_OS_FW_HASH_OID);
static auth_param_type_desc_t tos_fw_config_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_OS_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_extra1_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA1_HASH_OID);
static auth_param_type_desc_t tos_fw_extra2_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA2_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);
static auth_param_type_desc_t nt_fw_config_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, NON_TRUSTED_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
  AUTH_PARAM_HASH, FWU_HASH_OID);

/*
 * TBBR Chain of trust definition
 */
static const auth_img_desc_t cot_desc[] = {
  /*
   * BL2
   */
  [TRUSTED_BOOT_FW_CERT_ID] = {
    .img_id = TRUSTED_BOOT_FW_CERT_ID,
    .img_type = IMG_CERT,
    .parent = NULL,
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &subject_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &tb_fw_hash,
        .data = {
          .ptr = (void *)tb_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [1] = {
        .type_desc = &tb_fw_config_hash,
        .data = {
          .ptr = (void *)tb_fw_config_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [2] = {
        .type_desc = &hw_config_hash,
        .data = {
          .ptr = (void *)hw_config_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  [BL2_IMAGE_ID] = {
    .img_id = BL2_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_BOOT_FW_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tb_fw_hash,
        }
      }
    }
  },
  /* HW Config */
  [HW_CONFIG_ID] = {
    .img_id = HW_CONFIG_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_BOOT_FW_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &hw_config_hash,
        }
      }
    }
  },
  /* TB FW Config */
  [TB_FW_CONFIG_ID] = {
    .img_id = TB_FW_CONFIG_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_BOOT_FW_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tb_fw_config_hash,
        }
      }
    }
  },
  /*
   * Trusted key certificate
   */
  [TRUSTED_KEY_CERT_ID] = {
    .img_id = TRUSTED_KEY_CERT_ID,
    .img_type = IMG_CERT,
    .parent = NULL,
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &subject_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &trusted_world_pk,
        .data = {
          .ptr = (void *)trusted_world_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      },
      [1] = {
        .type_desc = &non_trusted_world_pk,
        .data = {
          .ptr = (void *)non_trusted_world_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      }
    }
  },
  /*
   * SCP Firmware
   */
  [SCP_FW_KEY_CERT_ID] = {
    .img_id = SCP_FW_KEY_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[TRUSTED_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &trusted_world_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &scp_fw_content_pk,
        .data = {
          .ptr = (void *)content_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      }
    }
  },
  [SCP_FW_CONTENT_CERT_ID] = {
    .img_id = SCP_FW_CONTENT_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[SCP_FW_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &scp_fw_content_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &scp_fw_hash,
        .data = {
          .ptr = (void *)scp_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  [SCP_BL2_IMAGE_ID] = {
    .img_id = SCP_BL2_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[SCP_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &scp_fw_hash,
        }
      }
    }
  },
  /*
   * SoC Firmware
   */
  [SOC_FW_KEY_CERT_ID] = {
    .img_id = SOC_FW_KEY_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[TRUSTED_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &trusted_world_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &soc_fw_content_pk,
        .data = {
          .ptr = (void *)content_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      }
    }
  },
  [SOC_FW_CONTENT_CERT_ID] = {
    .img_id = SOC_FW_CONTENT_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[SOC_FW_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &soc_fw_content_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &soc_fw_hash,
        .data = {
          .ptr = (void *)soc_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [1] = {
        .type_desc = &soc_fw_config_hash,
        .data = {
          .ptr = (void *)soc_fw_config_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  [BL31_IMAGE_ID] = {
    .img_id = BL31_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[SOC_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &soc_fw_hash,
        }
      }
    }
  },
  /* SOC FW Config */
  [SOC_FW_CONFIG_ID] = {
    .img_id = SOC_FW_CONFIG_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[SOC_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &soc_fw_config_hash,
        }
      }
    }
  },
  /*
   * Trusted OS Firmware
   */
  [TRUSTED_OS_FW_KEY_CERT_ID] = {
    .img_id = TRUSTED_OS_FW_KEY_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[TRUSTED_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &trusted_world_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &tos_fw_content_pk,
        .data = {
          .ptr = (void *)content_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      }
    }
  },
  [TRUSTED_OS_FW_CONTENT_CERT_ID] = {
    .img_id = TRUSTED_OS_FW_CONTENT_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[TRUSTED_OS_FW_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &tos_fw_content_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &trusted_nv_ctr,
          .plat_nv_ctr = &trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &tos_fw_hash,
        .data = {
          .ptr = (void *)tos_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [1] = {
        .type_desc = &tos_fw_extra1_hash,
        .data = {
          .ptr = (void *)tos_fw_extra1_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [2] = {
        .type_desc = &tos_fw_extra2_hash,
        .data = {
          .ptr = (void *)tos_fw_extra2_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [3] = {
        .type_desc = &tos_fw_config_hash,
        .data = {
          .ptr = (void *)tos_fw_config_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  [BL32_IMAGE_ID] = {
    .img_id = BL32_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_OS_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tos_fw_hash,
        }
      }
    }
  },
  [BL32_EXTRA1_IMAGE_ID] = {
    .img_id = BL32_EXTRA1_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_OS_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tos_fw_extra1_hash,
        }
      }
    }
  },
  [BL32_EXTRA2_IMAGE_ID] = {
    .img_id = BL32_EXTRA2_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_OS_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tos_fw_extra2_hash,
        }
      }
    }
  },
  /* TOS FW Config */
  [TOS_FW_CONFIG_ID] = {
    .img_id = TOS_FW_CONFIG_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[TRUSTED_OS_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &tos_fw_config_hash,
        }
      }
    }
  },
  /*
   * Non-Trusted Firmware
   */
  [NON_TRUSTED_FW_KEY_CERT_ID] = {
    .img_id = NON_TRUSTED_FW_KEY_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[TRUSTED_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &non_trusted_world_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &non_trusted_nv_ctr,
          .plat_nv_ctr = &non_trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &nt_fw_content_pk,
        .data = {
          .ptr = (void *)content_pk_buf,
          .len = (unsigned int)PK_DER_LEN
        }
      }
    }
  },
  [NON_TRUSTED_FW_CONTENT_CERT_ID] = {
    .img_id = NON_TRUSTED_FW_CONTENT_CERT_ID,
    .img_type = IMG_CERT,
    .parent = &cot_desc[NON_TRUSTED_FW_KEY_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &nt_fw_content_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      },
      [1] = {
        .type = AUTH_METHOD_NV_CTR,
        .param.nv_ctr = {
          .cert_nv_ctr = &non_trusted_nv_ctr,
          .plat_nv_ctr = &non_trusted_nv_ctr
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &nt_world_bl_hash,
        .data = {
          .ptr = (void *)nt_world_bl_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [1] = {
        .type_desc = &nt_fw_config_hash,
        .data = {
          .ptr = (void *)nt_fw_config_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  [BL33_IMAGE_ID] = {
    .img_id = BL33_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[NON_TRUSTED_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &nt_world_bl_hash,
        }
      }
    }
  },
  /* NT FW Config */
  [NT_FW_CONFIG_ID] = {
    .img_id = NT_FW_CONFIG_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[NON_TRUSTED_FW_CONTENT_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &nt_fw_config_hash,
        }
      }
    }
  },
  /*
   * FWU auth descriptor.
   */
  [FWU_CERT_ID] = {
    .img_id = FWU_CERT_ID,
    .img_type = IMG_CERT,
    .parent = NULL,
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_SIG,
        .param.sig = {
          .pk = &subject_pk,
          .sig = &sig,
          .alg = &sig_alg,
          .data = &raw_data,
        }
      }
    },
    .authenticated_data = {
      [0] = {
        .type_desc = &scp_bl2u_hash,
        .data = {
          .ptr = (void *)scp_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [1] = {
        .type_desc = &bl2u_hash,
        .data = {
          .ptr = (void *)tb_fw_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      },
      [2] = {
        .type_desc = &ns_bl2u_hash,
        .data = {
          .ptr = (void *)nt_world_bl_hash_buf,
          .len = (unsigned int)HASH_DER_LEN
        }
      }
    }
  },
  /*
   * SCP_BL2U
   */
  [SCP_BL2U_IMAGE_ID] = {
    .img_id = SCP_BL2U_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[FWU_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &scp_bl2u_hash,
        }
      }
    }
  },
  /*
   * BL2U
   */
  [BL2U_IMAGE_ID] = {
    .img_id = BL2U_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[FWU_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &bl2u_hash,
        }
      }
    }
  },
  /*
   * NS_BL2U
   */
  [NS_BL2U_IMAGE_ID] = {
    .img_id = NS_BL2U_IMAGE_ID,
    .img_type = IMG_RAW,
    .parent = &cot_desc[FWU_CERT_ID],
    .img_auth_methods = {
      [0] = {
        .type = AUTH_METHOD_HASH,
        .param.hash = {
          .data = &raw_data,
          .hash = &ns_bl2u_hash,
        }
      }
    }
  }
};

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
/*---------------------------------------------------------------------------*/
__attribute__((section(".patch.cmp_auth_param_type_desc")))
static int
cmp_auth_param_type_desc(const auth_param_type_desc_t *a,
                         const auth_param_type_desc_t *b)
{
  if((a->type == b->type) && (a->cookie == b->cookie)) {
    return 0;
  }
  return 1;
}
/*
 * This function obtains the requested authentication parameter data from the
 * information extracted from the parent image after its authentication.
 */
__attribute__((section(".patch.auth_get_param")))
static int
auth_get_param(const auth_param_type_desc_t *param_type_desc,
               const auth_img_desc_t *img_desc,
               void **param, unsigned int *len)
{
  int i;

  for(i = 0; i < COT_MAX_VERIFIED_PARAMS; i++) {
    if(0 == cmp_auth_param_type_desc(param_type_desc,
                                     img_desc->authenticated_data[i].type_desc)) {
      *param = img_desc->authenticated_data[i].data.ptr;
      *len = img_desc->authenticated_data[i].data.len;
      return 0;
    }
  }

  return 1;
}
/*
 * Authenticate an image by matching the data hash
 *
 * This function implements 'AUTH_METHOD_HASH'. To authenticate an image using
 * this method, the image must contain:
 *
 *   - The data to calculate the hash from
 *
 * The parent image must contain:
 *
 *   - The hash to be matched with (including hash algorithm)
 *
 * For a successful authentication, both hashes must match. The function calls
 * the crypto-module to check this matching.
 *
 * Parameters:
 *   param: parameters to perform the hash authentication
 *   img_desc: pointer to image descriptor so we can know the image type
 *             and parent image
 *   img: pointer to image in memory
 *   img_len: length of image (in bytes)
 *
 * Return:
 *   0 = success, Otherwise = error
 */
__attribute__((section(".patch.auth_hash")))
static int
auth_hash(const auth_method_param_hash_t *param,
          const auth_img_desc_t *img_desc,
          void *img, unsigned int img_len)
{
  void *data_ptr, *hash_der_ptr;
  unsigned int data_len, hash_der_len;
  int rc = 0;

  /* Get the hash from the parent image. This hash will be DER encoded
   * and contain the hash algorithm */
  rc = auth_get_param(param->hash, img_desc->parent,
                      &hash_der_ptr, &hash_der_len);
  return_if_error(rc);

  /* Get the data to be hashed from the current image */
  rc = arom_img_parser_get_auth_param(img_desc->img_type, param->data,
                                      img, img_len, &data_ptr, &data_len);
  return_if_error(rc);

  /* Ask the crypto module to verify this hash */
  rc = arom_crypto_mod_verify_hash(data_ptr, data_len,
                                   hash_der_ptr, hash_der_len);

  return rc;
}
/*
 * Authenticate by digital signature
 *
 * This function implements 'AUTH_METHOD_SIG'. To authenticate an image using
 * this method, the image must contain:
 *
 *   - Data to be signed
 *   - Signature
 *   - Signature algorithm
 *
 * We rely on the image parser module to extract this data from the image.
 * The parent image must contain:
 *
 *   - Public key (or a hash of it)
 *
 * If the parent image contains only a hash of the key, we will try to obtain
 * the public key from the image itself (i.e. self-signed certificates). In that
 * case, the signature verification is considered just an integrity check and
 * the authentication is established by calculating the hash of the key and
 * comparing it with the hash obtained from the parent.
 *
 * If the image has no parent (NULL), it means it has to be authenticated using
 * the ROTPK stored in the platform. Again, this ROTPK could be the key itself
 * or a hash of it.
 *
 * Return: 0 = success, Otherwise = error
 */
__attribute__((section(".patch.auth_signature")))
static int
auth_signature(const auth_method_param_sig_t *param,
               const auth_img_desc_t *img_desc,
               void *img, unsigned int img_len)
{
  void *data_ptr, *pk_ptr, *pk_hash_ptr, *sig_ptr, *sig_alg_ptr;
  unsigned int data_len, pk_len, pk_hash_len, sig_len, sig_alg_len;
  unsigned int flags = 0;
  int rc = 0;

  /* Get the data to be signed from current image */
  rc = arom_img_parser_get_auth_param(img_desc->img_type, param->data,
                                      img, img_len, &data_ptr, &data_len);
  return_if_error(rc);

  /* Get the signature from current image */
  rc = arom_img_parser_get_auth_param(img_desc->img_type, param->sig,
                                      img, img_len, &sig_ptr, &sig_len);
  return_if_error(rc);

  /* Get the signature algorithm from current image */
  rc = arom_img_parser_get_auth_param(img_desc->img_type, param->alg,
                                      img, img_len, &sig_alg_ptr, &sig_alg_len);
  return_if_error(rc);

  /* Get the public key from the parent. If there is no parent (NULL),
   * the certificate has been signed with the ROTPK, so we have to get
   * the PK from the platform */
  if(img_desc->parent) {
    rc = auth_get_param(param->pk, img_desc->parent,
                        &pk_ptr, &pk_len);
  } else {
    mbedtls_pk_type_t pk_alg;
    rc = arom_crypto_mod_get_hash_sig_alg(sig_alg_ptr, sig_alg_len, NULL, &pk_alg);
    return_if_error(rc);

    rc = arom_plat_get_rotpk_info((void *)pk_alg, &pk_ptr, &pk_len, &flags);
  }
  return_if_error(rc);

  if(flags & (ROTPK_IS_HASH | ROTPK_NOT_DEPLOYED)) {
    /* If the PK is a hash of the key or if the ROTPK is not
       deployed on the platform, retrieve the key from the image */
    pk_hash_ptr = pk_ptr;
    pk_hash_len = pk_len;
    rc = arom_img_parser_get_auth_param(img_desc->img_type,
                                        param->pk, img, img_len,
                                        &pk_ptr, &pk_len);
    return_if_error(rc);

    if(flags & ROTPK_NOT_DEPLOYED) {
/*      LOG_WARN("ROTPK is not deployed on platform.Skipping ROTPK verification.\n");
 */
    } else {
      /* Ask the crypto-module to verify the key hash */
      rc = arom_crypto_mod_verify_hash(pk_ptr, pk_len,
                                       pk_hash_ptr, pk_hash_len);
    }
  }

  /* Ask the crypto module to verify the signature */
  rc = arom_crypto_mod_verify_signature(data_ptr, data_len,
                                        sig_ptr, sig_len,
                                        sig_alg_ptr, sig_alg_len,
                                        pk_ptr, pk_len);
  return rc;
}
/*
 * Return the parent id in the output parameter '*parent_id'
 *
 * Return value:
 *   0 = Image has parent, 1 = Image has no parent or parent is authenticated
 */
__attribute__((section(".patch.auth_mod_get_parent_id")))
int
auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id)
{
  const auth_img_desc_t *img_desc = NULL;

  assert(parent_id != NULL);

  /* Get the image descriptor */
  img_desc = &cot_desc_ptr[img_id];

  /* Check if the image has no parent (ROT) */
  if(img_desc->parent == NULL) {
    *parent_id = 0;
    return 1;
  }

  *parent_id = img_desc->parent->img_id;

  return 0;
}
/*
 * Authenticate a certificate/image
 *
 * Return: 0 = success, Otherwise = error
 */
__attribute__((section(".patch.auth_mod_verify_img")))
int
auth_mod_verify_img(unsigned int img_id,
                    void *img_ptr,
                    unsigned int img_len)
{
  const auth_img_desc_t *img_desc = NULL;
  const auth_method_desc_t *auth_method = NULL;
  void *param_ptr;
  unsigned int param_len;
  int rc, i;

  /* Get the image descriptor from the chain of trust */
  img_desc = &cot_desc_ptr[img_id];

  /* Ask the parser to check the image integrity */
  rc = arom_img_parser_check_integrity(img_desc->img_type, img_ptr, img_len);
  return_if_error(rc);

  /* Authenticate the image using the methods indicated in the image
   * descriptor. */
  for(i = 0; i < AUTH_METHOD_NUM; i++) {
    auth_method = &img_desc->img_auth_methods[i];
    switch(auth_method->type) {
    case AUTH_METHOD_NONE:
      rc = 0;
      break;
    case AUTH_METHOD_HASH:
      rc = auth_hash(&auth_method->param.hash,
                     img_desc, img_ptr, img_len);
      break;
    case AUTH_METHOD_SIG:
      rc = auth_signature(&auth_method->param.sig,
                          img_desc, img_ptr, img_len);
      break;

    case AUTH_METHOD_NV_CTR:
      /*rc = auth_nvctr(&auth_method->param.nv_ctr, */
      /*                 img_desc, img_ptr, img_len); */
      rc = 0;
      break;

    default:
      /* Unknown authentication method */
      rc = 1;
      break;
    }
    return_if_error(rc);
  }

  /* Extract the parameters indicated in the image descriptor to
   * authenticate the children images. */
  for(i = 0; i < COT_MAX_VERIFIED_PARAMS; i++) {
    if(img_desc->authenticated_data[i].type_desc == NULL) {
      continue;
    }

    /* Get the parameter from the image parser module */
    rc = arom_img_parser_get_auth_param(img_desc->img_type,
                                        img_desc->authenticated_data[i].type_desc,
                                        img_ptr, img_len, &param_ptr, &param_len);
    return_if_error(rc);

    /* Check parameter size */
    if(param_len > img_desc->authenticated_data[i].data.len) {
      return 1;
    }

    /* Copy the parameter for later use */
    memcpy((void *)img_desc->authenticated_data[i].data.ptr,
           (void *)param_ptr, param_len);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
secureboot_arom_patch(void)
{
  unsigned int chip_id = hw_chip_id();

  if(CHIP_ID_CRANEG == chip_id || CHIP_ID_CRANEM == chip_id) {
    volatile unsigned *address = AROM_PATCH_2_ADDRESS_AUTH_MOD_GET_PARENT_ID;
    *address = bl_encode((unsigned)address, &auth_mod_get_parent_id);

    address = AROM_PATCH_3_ADDRESS_AUTH_MOD_VERIFY_IMG;
    *address = bl_encode((unsigned)address, &auth_mod_verify_img);
  }
}
/*---------------------------------------------------------------------------*/

