/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016-2020 Toradex
 */

#ifndef _TDX_CFG_BLOCK_DATA_H
#define _TDX_CFG_BLOCK_DATA_H

enum {
	COLIBRI_PXA270_V1_312MHZ = 1,
	COLIBRI_PXA270_V1_520MHZ,
	COLIBRI_PXA320,
	COLIBRI_PXA300,
	COLIBRI_PXA310, /* 5 */
	COLIBRI_PXA320_IT,
	COLIBRI_PXA300_XT,
	COLIBRI_PXA270_312MHZ,
	COLIBRI_PXA270_520MHZ,
	COLIBRI_VF50, /* 10 */
	COLIBRI_VF61,
	COLIBRI_VF61_IT,
	COLIBRI_VF50_IT,
	COLIBRI_IMX6S,
	COLIBRI_IMX6DL, /* 15 */
	COLIBRI_IMX6S_IT,
	COLIBRI_IMX6DL_IT,
	/* 18 */
	/* 19 */
	COLIBRI_T20_256MB = 20,
	COLIBRI_T20_512MB,
	COLIBRI_T20_512MB_IT,
	COLIBRI_T30,
	COLIBRI_T20_256MB_IT,
	APALIS_T30_2GB, /* 25 */
	APALIS_T30_1GB,
	APALIS_IMX6Q,
	APALIS_IMX6Q_IT,
	APALIS_IMX6D,
	COLIBRI_T30_IT, /* 30 */
	APALIS_T30_IT,
	COLIBRI_IMX7S,
	COLIBRI_IMX7D,
	APALIS_TK1_2GB,
	APALIS_IMX6D_IT, /* 35 */
	COLIBRI_IMX6ULL,
	APALIS_IMX8QM_WIFI_BT_IT,
	COLIBRI_IMX8QXP_WIFI_BT_IT,
	COLIBRI_IMX7D_EMMC,
	COLIBRI_IMX6ULL_WIFI_BT_IT, /* 40 */
	COLIBRI_IMX7D_EPDC,
	APALIS_TK1_4GB,
	COLIBRI_T20_512MB_IT_SETEK,
	COLIBRI_IMX6ULL_IT,
	COLIBRI_IMX6ULL_WIFI_BT, /* 45 */
	APALIS_IMX8QXP_WIFI_BT_IT,
	APALIS_IMX8QM_IT,
	APALIS_IMX8QP_WIFI_BT,
	APALIS_IMX8QP,
	COLIBRI_IMX8QXP_IT, /* 50 */
	COLIBRI_IMX8DX_WIFI_BT,
	COLIBRI_IMX8DX,
	APALIS_IMX8QXP,
	APALIS_IMX8DXP,
	VERDIN_IMX8MMQ_WIFI_BT_IT, /* 55 */
	VERDIN_IMX8MNQ_WIFI_BT,
	VERDIN_IMX8MMDL,
	VERDIN_IMX8MPQ_WIFI_BT_IT,
	VERDIN_IMX8MMQ_IT,
	VERDIN_IMX8MMDL_WIFI_BT_IT, /* 60 */
	VERDIN_IMX8MPQ,
	COLIBRI_IMX6ULL_IT_EMMC,
	VERDIN_IMX8MPQ_IT,
	VERDIN_IMX8MPQ_2GB_WIFI_BT_IT,
	VERDIN_IMX8MPQL_IT, /* 65 */
	VERDIN_IMX8MPQ_8GB_WIFI_BT,
	APALIS_IMX8QM_8GB_WIFI_BT_IT,
	VERDIN_IMX8MMQ_WIFI_BT_IT_NO_CAN,
	/* 69 */
	VERDIN_IMX8MPQ_8GB_WIFI_BT_IT = 70, /* 70 */
	/* 71-85 */
	VERDIN_IMX8MMDL_2G_IT = 86,
	VERDIN_IMX8MMQ_2G_IT_NO_CAN,
};

enum {
	DAHLIA = 155,
	VERDIN_DEVELOPMENT_BOARD = 156,
	YAVIA = 173,
};

enum {
	VERDIN_DSI_TO_HDMI_ADAPTER = 157,
	VERDIN_DSI_TO_LVDS_ADAPTER = 159,
};


#define TARGET_IS_ENABLED(x) (1)

const struct toradex_som toradex_modules[] = {
	 [0] = { "UNKNOWN MODULE",                       0                                  },
	 [1] = { "Colibri PXA270 312MHz",                0                                  },
	 [2] = { "Colibri PXA270 520MHz",                0                                  },
	 [3] = { "Colibri PXA320 806MHz",                0                                  },
	 [4] = { "Colibri PXA300 208MHz",                0                                  },
	 [5] = { "Colibri PXA310 624MHz",                0                                  },
	 [6] = { "Colibri PXA320IT 806MHz",              0                                  },
	 [7] = { "Colibri PXA300 208MHz XT",             0                                  },
	 [8] = { "Colibri PXA270 312MHz",                0                                  },
	 [9] = { "Colibri PXA270 520MHz",                0                                  },
	[10] = { "Colibri VF50 128MB",                   TARGET_IS_ENABLED(COLIBRI_VF)      },
	[11] = { "Colibri VF61 256MB",                   TARGET_IS_ENABLED(COLIBRI_VF)      },
	[12] = { "Colibri VF61 256MB IT",                TARGET_IS_ENABLED(COLIBRI_VF)      },
	[13] = { "Colibri VF50 128MB IT",                TARGET_IS_ENABLED(COLIBRI_VF)      },
	[14] = { "Colibri iMX6S 256MB",                  TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[15] = { "Colibri iMX6DL 512MB",                 TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[16] = { "Colibri iMX6S 256MB IT",               TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[17] = { "Colibri iMX6DL 512MB IT",              TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[18] = { "UNKNOWN MODULE",                       0                                  },
	[19] = { "UNKNOWN MODULE",                       0                                  },
	[20] = { "Colibri T20 256MB",                    TARGET_IS_ENABLED(COLIBRI_T20)     },
	[21] = { "Colibri T20 512MB",                    TARGET_IS_ENABLED(COLIBRI_T20)     },
	[22] = { "Colibri T20 512MB IT",                 TARGET_IS_ENABLED(COLIBRI_T20)     },
	[23] = { "Colibri T30 1GB",                      TARGET_IS_ENABLED(COLIBRI_T30)     },
	[24] = { "Colibri T20 256MB IT",                 TARGET_IS_ENABLED(COLIBRI_T20)     },
	[25] = { "Apalis T30 2GB",                       TARGET_IS_ENABLED(APALIS_T30)      },
	[26] = { "Apalis T30 1GB",                       TARGET_IS_ENABLED(APALIS_T30)      },
	[27] = { "Apalis iMX6Q 1GB",                     TARGET_IS_ENABLED(APALIS_IMX6)     },
	[28] = { "Apalis iMX6Q 2GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[29] = { "Apalis iMX6D 512MB",                   TARGET_IS_ENABLED(APALIS_IMX6)     },
	[30] = { "Colibri T30 1GB IT",                   TARGET_IS_ENABLED(COLIBRI_T30)     },
	[31] = { "Apalis T30 1GB IT",                    TARGET_IS_ENABLED(APALIS_T30)      },
	[32] = { "Colibri iMX7S 256MB",                  TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[33] = { "Colibri iMX7D 512MB",                  TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[34] = { "Apalis TK1 2GB",                       TARGET_IS_ENABLED(APALIS_TK1)      },
	[35] = { "Apalis iMX6D 1GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[36] = { "Colibri iMX6ULL 256MB",                TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[37] = { "Apalis iMX8QM 4GB WB IT",              TARGET_IS_ENABLED(APALIS_IMX8)     },
	[38] = { "Colibri iMX8QXP 2GB WB IT",            TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[39] = { "Colibri iMX7D 1GB",                    TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[40] = { "Colibri iMX6ULL 512MB WB IT",          TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[41] = { "Colibri iMX7D 512MB EPDC",             TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[42] = { "Apalis TK1 4GB",                       TARGET_IS_ENABLED(APALIS_TK1)      },
	[43] = { "Colibri T20 512MB IT SETEK",           TARGET_IS_ENABLED(COLIBRI_T20)     },
	[44] = { "Colibri iMX6ULL 512MB IT",             TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[45] = { "Colibri iMX6ULL 512MB WB",             TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[46] = { "Apalis iMX8QXP 2GB WB IT",             0                                  },
	[47] = { "Apalis iMX8QM 4GB IT",                 TARGET_IS_ENABLED(APALIS_IMX8)     },
	[48] = { "Apalis iMX8QP 2GB WB",                 TARGET_IS_ENABLED(APALIS_IMX8)     },
	[49] = { "Apalis iMX8QP 2GB",                    TARGET_IS_ENABLED(APALIS_IMX8)     },
	[50] = { "Colibri iMX8QXP 2GB IT",               TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[51] = { "Colibri iMX8DX 1GB WB",                TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[52] = { "Colibri iMX8DX 1GB",                   TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[53] = { "Apalis iMX8QXP 2GB ECC IT",            0                                  },
	[54] = { "Apalis iMX8DXP 1GB",                   TARGET_IS_ENABLED(APALIS_IMX8)     },
	[55] = { "Verdin iMX8M Mini Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[56] = { "Verdin iMX8M Nano Quad 1GB WB",        0                                  },
	[57] = { "Verdin iMX8M Mini DualLite 1GB",       TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[58] = { "Verdin iMX8M Plus Quad 4GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[59] = { "Verdin iMX8M Mini Quad 2GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[60] = { "Verdin iMX8M Mini DualLite 1GB WB IT", TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[61] = { "Verdin iMX8M Plus Quad 2GB",           TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[62] = { "Colibri iMX6ULL 1GB IT",               TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[63] = { "Verdin iMX8M Plus Quad 4GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[64] = { "Verdin iMX8M Plus Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[65] = { "Verdin iMX8M Plus QuadLite 1GB IT",    TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[66] = { "Verdin iMX8M Plus Quad 8GB WB",        TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[67] = { "Apalis iMX8QM 8GB WB IT",              TARGET_IS_ENABLED(APALIS_IMX8)     },
	[68] = { "Verdin iMX8M Mini Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[70] = { "Verdin iMX8M Plus Quad 8GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[86] = { "Verdin iMX8M Mini DualLite 2GB IT",    TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[87] = { "Verdin iMX8M Mini Quad 2GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
};

const struct pid4list toradex_carrier_boards[] = {
	/* the code assumes unknown at index 0 */
	{0,				"UNKNOWN CARRIER BOARD"},
	{DAHLIA,			"Dahlia"},
	{VERDIN_DEVELOPMENT_BOARD,	"Verdin Development Board"},
	{YAVIA,				"Yavia"},
};

const struct pid4list toradex_display_adapters[] = {
	/* the code assumes unknown at index 0 */
	{0,				"UNKNOWN DISPLAY ADAPTER"},
	{VERDIN_DSI_TO_HDMI_ADAPTER,	"Verdin DSI to HDMI Adapter"},
	{VERDIN_DSI_TO_LVDS_ADAPTER,	"Verdin DSI to LVDS Adapter"},
};

const u32 toradex_ouis[] = {
	[0] = 0x00142dUL,
	[1] = 0x8c06cbUL,
};

#endif /* _TDX_CFG_BLOCK_DATA_H */

