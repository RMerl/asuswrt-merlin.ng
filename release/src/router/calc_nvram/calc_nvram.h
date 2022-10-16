enum {
  CKN_STR_DEFAULT = 0,
  CKN_STR1,
  CKN_STR2,
  CKN_STR3,
  CKN_STR4,
  CKN_STR5,
  CKN_STR6,
  CKN_STR7,
  CKN_STR8,
  CKN_STR10   = 10,
  CKN_STR12   = 12,
  CKN_STR15   = 15,
  CKN_STR16,
  CKN_STR17,
  CKN_STR20   = 20,
  CKN_STR32   = 32,
  CKN_STR39   = 39,
  CKN_STR44   = 44,
  CKN_STR63   = 63,
  CKN_STR64   = 64,
  CKN_STR65,
  CKN_STR100  = 100,
  CKN_STR128  = 128,
  CKN_STR255  = 255,
  CKN_STR256,
  CKN_STR512  = 512,
  CKN_STR1024 = 1024,
  CKN_STR2048 = 2048,
  CKN_STR2500 = 2500,
  CKN_STR2999 = 2999,
  CKN_STR3999 = 3999,
  CKN_STR4096 = 4096,
  CKN_STR5500 = 5500,
  CKN_STR7999 = 7999,
  CKN_STR8192 = 8192,
  CKN_STR_MAX = 65535
};

enum {
  CKN_TYPE_DEFAULT = 0
};

enum {
  CKN_ACC_LEVEL_DEFAULT = 0
};

enum {
  CKN_ENC_DEFAULT = 0,
  CKN_ENC_SVR
};

struct nvram_tuple {
  char *name;
  char *value;
  unsigned short len;
  unsigned short type;
  unsigned short acc_level;
  unsigned short enc;
  struct nvram_tuple *next;
};
