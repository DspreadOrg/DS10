#include <string.h>
#include <stdlib.h>

#include "fip.h"
#include "firmware_image_package.h"
#include "tbbr_img_def.h"

#include "log.h"
#define LOG_MODULE "Fip"
#define LOG_LEVEL LOG_LEVEL_MAIN

typedef int64_t (*fip_read_data)(uint64_t from, uint64_t size, uint8_t *to);

typedef struct fip_info {
  uint64_t address;
  fip_read_data read;
  fip_toc_header_t toc;
  fip_toc_entry_t *entry;
  int entry_count;
} fip_info_t;

typedef struct {
  unsigned int id;
  uuid_t uuid;
} uuid_map_t;

/* By default, ARM platforms load images from the FIP */
static const uuid_map_t image_uuid[] = {
  { BL2_IMAGE_ID, UUID_TRUSTED_BOOT_FIRMWARE_BL2 },
  { SCP_BL2_IMAGE_ID, UUID_SCP_FIRMWARE_SCP_BL2 },
  { BL31_IMAGE_ID, UUID_EL3_RUNTIME_FIRMWARE_BL31 },
  { BL32_IMAGE_ID, UUID_SECURE_PAYLOAD_BL32 },
  { BL33_IMAGE_ID, UUID_NON_TRUSTED_FIRMWARE_BL33 },
  { TRUSTED_BOOT_FW_CERT_ID, UUID_TRUSTED_BOOT_FW_CERT },
  { TRUSTED_KEY_CERT_ID, UUID_TRUSTED_KEY_CERT },
  { SCP_FW_KEY_CERT_ID, UUID_SCP_FW_KEY_CERT },
  { SOC_FW_KEY_CERT_ID, UUID_SOC_FW_KEY_CERT },
  { TRUSTED_OS_FW_KEY_CERT_ID, UUID_TRUSTED_OS_FW_KEY_CERT },
  { NON_TRUSTED_FW_KEY_CERT_ID, UUID_NON_TRUSTED_FW_KEY_CERT },
  { SCP_FW_CONTENT_CERT_ID, UUID_SCP_FW_CONTENT_CERT },
  { SOC_FW_CONTENT_CERT_ID, UUID_SOC_FW_CONTENT_CERT },
  { TRUSTED_OS_FW_CONTENT_CERT_ID, UUID_TRUSTED_OS_FW_CONTENT_CERT },
  { NON_TRUSTED_FW_CONTENT_CERT_ID, UUID_NON_TRUSTED_FW_CONTENT_CERT },
  { BL32_EXTRA1_IMAGE_ID, UUID_SECURE_PAYLOAD_BL32_EXTRA1 },
  { BL32_EXTRA2_IMAGE_ID, UUID_SECURE_PAYLOAD_BL32_EXTRA2 },
  { USER1_IMAGE_ID, UUID_NON_TRUSTED_OS_USER1 },
};

static const uuid_t uuid_null = { 0 };

static inline int
compare_uuids(const uuid_t *uuid1, const uuid_t *uuid2)
{
  return memcmp(uuid1, uuid2, sizeof(uuid_t));
}
static int64_t
read_data_from_memory(uint64_t from, uint64_t size, uint8_t *to)
{
  uint8_t *start = (uint8_t *)(uintptr_t)from;

  if(start + size < to || to + size < start) {
    memcpy(to, start, size);
  } else if(to != start) {
    memmove(to, start, size);
  }
  return (int64_t)size;
}
/*
 * See if a Firmware Image Package is available,
 * by checking if TOC is valid or not.
 */
int
is_valid_fip(void *address)
{
  fip_toc_header_t header;

  read_data_from_memory((uint64_t)(uintptr_t)address, sizeof(header), (uint8_t *)&header);

  return header.name == TOC_HEADER_NAME;
}
int
fip_get_plat_flag(void *address, uint16_t *flag)
{
  fip_toc_header_t header;

  read_data_from_memory((uint64_t)(uintptr_t)address, sizeof(header), (uint8_t *)&header);

  if(header.name == TOC_HEADER_NAME) {
    *flag = (header.flags >> 32) & 0xFFFF;
    return 0;
  } else {
    return -1;
  }
}
fip_handle
fip_open(void *address)
{
  fip_handle fh = calloc(1, sizeof(*fh));

  if(!fh) {
    LOG_ERR("Out of memory!\n");
    return NULL;
  }
  fh->read = read_data_from_memory;
  fh->address = (uint64_t)(uintptr_t)address;

  if(fh->read(fh->address, sizeof(fh->toc), (uint8_t *)&fh->toc) != sizeof(fh->toc)) {
    LOG_ERR("Read fail!\n");
    free(fh);
    return NULL;
  }

  if(fh->toc.name != TOC_HEADER_NAME) {
    LOG_ERR("Not a fip!\n");
    free(fh);
    return NULL;
  }

  fip_toc_entry_t entry;
  uint64_t entry_address = fh->address + sizeof(fh->toc);
  do {
    if(fh->read(entry_address, sizeof(entry), (uint8_t *)&entry) != sizeof(entry)) {
      LOG_ERR("Read fail!\n");
      free(fh);
      return NULL;
    }
    entry_address += sizeof(entry);
    fh->entry_count++;
  } while(compare_uuids(&entry.uuid, &uuid_null));
  fh->entry_count--;
  fh->entry = malloc(fh->entry_count * sizeof(*fh->entry));
  if(!fh->entry) {
    LOG_ERR("Out of memory\n");
    free(fh);
    return NULL;
  }
  entry_address = fh->address + sizeof(fh->toc);
  if(fh->read(entry_address, fh->entry_count * sizeof(entry), (uint8_t *)fh->entry) != fh->entry_count * sizeof(entry)) {
    LOG_ERR("Read fail!\n");
    free(fh->entry);
    free(fh);
    return NULL;
  }

  return fh;
}
int
fip_close(fip_handle fh)
{
  if(fh) {
    free(fh->entry);
    free(fh);
  }
  return 0;
}
static int
find_uuid_by_id(unsigned int image_id)
{
  for(int i = 0; i < (int)ARRAY_SIZE(image_uuid); i++) {
    if(image_uuid[i].id == image_id) {
      return i;
    }
  }
  LOG_INFO("Can't find uuid by id %u\n", image_id);
  return -1;
}
int
fip_get_image_info(fip_handle fh, unsigned int image_id, uint64_t *pstart, uint64_t *poffset, uint64_t *psize)
{
  if(!fh) {
    LOG_ERR("Fip handle is empty\n");
    return -1;
  }
  if(!fh->entry_count) {
    LOG_INFO("Fip has no any images!\n");
    return -1;
  }

  int index = find_uuid_by_id(image_id);
  if(index < 0) {
    return -1;
  }
  int entry_index = -1;
  for(int i = 0; i < fh->entry_count; i++) {
    if(!compare_uuids(&fh->entry[i].uuid, &image_uuid[index].uuid)) {
      entry_index = i;
      break;
    }
  }
  if(entry_index < 0) {
    LOG_ERR("Fip doesn't contain image(id=%u)\n", image_id);
    return -1;
  }

  if(pstart) {
    *pstart = fh->address;
  }
  if(poffset) {
    *poffset = fh->entry[entry_index].offset_address;
  }
  if(psize) {
    *psize = fh->entry[entry_index].size;
  }
  return 0;
}
typedef struct fip_image {
  unsigned int id;
  uint64_t start;
  uint64_t offset;
  uint64_t size;
  fip_read_data read;
} fip_image_t;

fip_image_handle
fip_open_image(fip_handle fh, unsigned int image_id)
{
  fip_image_t *fih = malloc(sizeof(*fih));

  if(!fih) {
    LOG_ERR("Out of memory!\n");
    return NULL;
  }

  if(fip_get_image_info(fh, image_id, &fih->start, &fih->offset, &fih->size) < 0) {
    free(fih);
    return NULL;
  }
  fih->id = image_id;
  fih->read = fh->read;

  return fih;
}
int
fip_close_image(fip_image_handle fih)
{
  free(fih);
  return 0;
}
int64_t
fip_read_image(fip_image_handle fih, uint8_t *data, uint64_t size)
{
  if(!fih) {
    LOG_ERR("Can't read empty fip img\n");
    return 0;
  }
  if(!data) {
    LOG_ERR("Can't store to empty buffer\n");
    return 0;
  }
  uint64_t real_size = MIN(size, fih->size);
  if(!real_size) {
    return 0;
  }
  int64_t ret = fih->read(fih->start + fih->offset, real_size, data);
  if(ret < 0) {
    LOG_ERR("Read fail!\n");
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
int
fip_get_image(void *fip_start, unsigned int img_id,
              uint64_t *image_addr, uint64_t *image_size)
{
  int rc;
  fip_handle fh;
  uint64_t start, offset, size;

  fh = fip_open(fip_start);
  if(!fh) {
    return -1;
  }

  rc = fip_get_image_info(fh, img_id, &start, &offset, &size);
  if(rc < 0) {
    fip_close(fh);
    return -1;
  }

  if(image_addr) {
    *image_addr = start + offset;
  }

  if(image_size) {
    *image_size = size;
  }

  fip_close(fh);

  return 0;
}
