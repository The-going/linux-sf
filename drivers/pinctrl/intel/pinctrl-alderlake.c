// SPDX-License-Identifier: GPL-2.0
/*
 * Intel Alder Lake PCH pinctrl/GPIO driver
 *
 * Copyright (C) 2020, 2022 Intel Corporation
 * Author: Andy Shevchenko <andriy.shevchenko@linux.intel.com>
 */

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>

#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-intel.h"

#define ADL_N_PAD_OWN		0x020
#define ADL_N_PADCFGLOCK	0x080
#define ADL_N_HOSTSW_OWN	0x0b0
#define ADL_N_GPI_IS		0x100
#define ADL_N_GPI_IE		0x120

#define ADL_S_PAD_OWN		0x0a0
#define ADL_S_PADCFGLOCK	0x110
#define ADL_S_HOSTSW_OWN	0x150
#define ADL_S_GPI_IS		0x200
#define ADL_S_GPI_IE		0x220

#define ADL_GPP(r, s, e, g)				\
	{						\
		.reg_num = (r),				\
		.base = (s),				\
		.size = ((e) - (s) + 1),		\
		.gpio_base = (g),			\
	}

#define ADL_N_COMMUNITY(b, s, e, g)			\
	INTEL_COMMUNITY_GPPS(b, s, e, g, ADL_N)

#define ADL_S_COMMUNITY(b, s, e, g)			\
	INTEL_COMMUNITY_GPPS(b, s, e, g, ADL_S)

/* Alder Lake-N */
static const struct pinctrl_pin_desc adln_pins[] = {
	/* GPP_B */
	PINCTRL_PIN(0, "CORE_VID_0"),
	PINCTRL_PIN(1, "CORE_VID_1"),
	PINCTRL_PIN(2, "GPPC_B_2"),
	PINCTRL_PIN(3, "GPPC_B_3"),
	PINCTRL_PIN(4, "GPPC_B_4"),
	PINCTRL_PIN(5, "GPPC_B_5"),
	PINCTRL_PIN(6, "GPPC_B_6"),
	PINCTRL_PIN(7, "GPPC_B_7"),
	PINCTRL_PIN(8, "GPPC_B_8"),
	PINCTRL_PIN(9, "GPPC_B_9"),
	PINCTRL_PIN(10, "GPPC_B_10"),
	PINCTRL_PIN(11, "GPPC_B_11"),
	PINCTRL_PIN(12, "SLP_S0B"),
	PINCTRL_PIN(13, "PLTRSTB"),
	PINCTRL_PIN(14, "GPPC_B_14"),
	PINCTRL_PIN(15, "GPPC_B_15"),
	PINCTRL_PIN(16, "GPPC_B_16"),
	PINCTRL_PIN(17, "GPPC_B_17"),
	PINCTRL_PIN(18, "GPPC_B_18"),
	PINCTRL_PIN(19, "GPPC_B_19"),
	PINCTRL_PIN(20, "GPPC_B_20"),
	PINCTRL_PIN(21, "GPPC_B_21"),
	PINCTRL_PIN(22, "GPPC_B_22"),
	PINCTRL_PIN(23, "GPPC_B_23"),
	PINCTRL_PIN(24, "GSPI0_CLK_LOOPBK"),
	PINCTRL_PIN(25, "GSPI1_CLK_LOOPBK"),
	/* GPP_T */
	PINCTRL_PIN(26, "GPPC_T_0"),
	PINCTRL_PIN(27, "GPPC_T_1"),
	PINCTRL_PIN(28, "FUSA_DIAGTEST_EN"),
	PINCTRL_PIN(29, "FUSA_DIAGTEST_MODE"),
	PINCTRL_PIN(30, "GPPC_T_4"),
	PINCTRL_PIN(31, "GPPC_T_5"),
	PINCTRL_PIN(32, "GPPC_T_6"),
	PINCTRL_PIN(33, "GPPC_T_7"),
	PINCTRL_PIN(34, "GPPC_T_8"),
	PINCTRL_PIN(35, "GPPC_T_9"),
	PINCTRL_PIN(36, "GPPC_T_10"),
	PINCTRL_PIN(37, "GPPC_T_11"),
	PINCTRL_PIN(38, "GPPC_T_12"),
	PINCTRL_PIN(39, "GPPC_T_13"),
	PINCTRL_PIN(40, "GPPC_T_14"),
	PINCTRL_PIN(41, "GPPC_T_15"),
	/* GPP_A */
	PINCTRL_PIN(42, "ESPI_IO_0"),
	PINCTRL_PIN(43, "ESPI_IO_1"),
	PINCTRL_PIN(44, "ESPI_IO_2"),
	PINCTRL_PIN(45, "ESPI_IO_3"),
	PINCTRL_PIN(46, "ESPI_CS0B"),
	PINCTRL_PIN(47, "ESPI_ALERT0B"),
	PINCTRL_PIN(48, "ESPI_ALERT1B"),
	PINCTRL_PIN(49, "GPPC_A_7"),
	PINCTRL_PIN(50, "GPPC_A_8"),
	PINCTRL_PIN(51, "ESPI_CLK"),
	PINCTRL_PIN(52, "ESPI_RESETB"),
	PINCTRL_PIN(53, "GPPC_A_11"),
	PINCTRL_PIN(54, "GPPC_A_12"),
	PINCTRL_PIN(55, "GPPC_A_13"),
	PINCTRL_PIN(56, "GPPC_A_14"),
	PINCTRL_PIN(57, "GPPC_A_15"),
	PINCTRL_PIN(58, "GPPC_A_16"),
	PINCTRL_PIN(59, "GPPC_A_17"),
	PINCTRL_PIN(60, "GPPC_A_18"),
	PINCTRL_PIN(61, "GPPC_A_19"),
	PINCTRL_PIN(62, "GPPC_A_20"),
	PINCTRL_PIN(63, "GPPC_A_21"),
	PINCTRL_PIN(64, "GPPC_A_22"),
	PINCTRL_PIN(65, "ESPI_CS1B"),
	PINCTRL_PIN(66, "ESPI_CLK_LOOPBK"),
	/* GPP_S */
	PINCTRL_PIN(67, "GPP_S_0"),
	PINCTRL_PIN(68, "GPP_S_1"),
	PINCTRL_PIN(69, "GPP_S_2"),
	PINCTRL_PIN(70, "GPP_S_3"),
	PINCTRL_PIN(71, "GPP_S_4"),
	PINCTRL_PIN(72, "GPP_S_5"),
	PINCTRL_PIN(73, "GPP_S_6"),
	PINCTRL_PIN(74, "GPP_S_7"),
	/* GPP_I */
	PINCTRL_PIN(75, "GPP_F_0_CNV_BRI_DT_UART0_RTSB"),
	PINCTRL_PIN(76, "GPP_F_1_CNV_BRI_RSP_UART0_RXD"),
	PINCTRL_PIN(77, "GPP_F_2_CNV_RGI_DT_UART0_TXD"),
	PINCTRL_PIN(78, "GPP_F_3_CNV_RGI_RSP_UART0_CTSB"),
	PINCTRL_PIN(79, "GPP_F_4_CNV_RF_RESET_B"),
	PINCTRL_PIN(80, "GPP_F_5_MODEM_CLKREQ"),
	PINCTRL_PIN(81, "GPP_F_6_CNV_PA_BLANKING"),
	PINCTRL_PIN(82, "GPP_F_7_EMMC_CMD"),
	PINCTRL_PIN(83, "GPP_F_8_EMMC_DATA0"),
	PINCTRL_PIN(84, "GPP_F_9_EMMC_DATA1"),
	PINCTRL_PIN(85, "GPP_F_10_EMMC_DATA2"),
	PINCTRL_PIN(86, "GPP_F_11_EMMC_DATA3"),
	PINCTRL_PIN(87, "GPP_F_12_EMMC_DATA4"),
	PINCTRL_PIN(88, "GPP_F_13_EMMC_DATA5"),
	PINCTRL_PIN(89, "GPP_F_14_EMMC_DATA6"),
	PINCTRL_PIN(90, "GPP_F_15_EMMC_DATA7"),
	PINCTRL_PIN(91, "GPP_F_16_EMMC_RCLK"),
	PINCTRL_PIN(92, "GPP_F_17_EMMC_CLK"),
	PINCTRL_PIN(93, "GPP_F_18_EMMC_RESETB"),
	PINCTRL_PIN(94, "GPP_F_19_A4WP_PRESENT"),
	/* GPP_H */
	PINCTRL_PIN(95, "GPPC_H_0"),
	PINCTRL_PIN(96, "GPPC_H_1"),
	PINCTRL_PIN(97, "GPPC_H_2"),
	PINCTRL_PIN(98, "GPPC_H_3"),
	PINCTRL_PIN(99, "GPPC_H_4"),
	PINCTRL_PIN(100, "GPPC_H_5"),
	PINCTRL_PIN(101, "GPPC_H_6"),
	PINCTRL_PIN(102, "GPPC_H_7"),
	PINCTRL_PIN(103, "GPPC_H_8"),
	PINCTRL_PIN(104, "GPPC_H_9"),
	PINCTRL_PIN(105, "GPPC_H_10"),
	PINCTRL_PIN(106, "GPPC_H_11"),
	PINCTRL_PIN(107, "I2C7_SDA"),
	PINCTRL_PIN(108, "I2C7_SCL"),
	PINCTRL_PIN(109, "GPPC_H_14"),
	PINCTRL_PIN(110, "GPPC_H_15"),
	PINCTRL_PIN(111, "GPPC_H_16"),
	PINCTRL_PIN(112, "GPPC_H_17"),
	PINCTRL_PIN(113, "CPU_C10_GATEB"),
	PINCTRL_PIN(114, "GPPC_H_19"),
	PINCTRL_PIN(115, "GPPC_H_20"),
	PINCTRL_PIN(116, "GPPC_H_21"),
	PINCTRL_PIN(117, "GPPC_H_22"),
	PINCTRL_PIN(118, "GPPC_H_23"),
	/* GPP_D */
	PINCTRL_PIN(119, "GPPC_D_0"),
	PINCTRL_PIN(120, "GPPC_D_1"),
	PINCTRL_PIN(121, "GPPC_D_2"),
	PINCTRL_PIN(122, "GPPC_D_3"),
	PINCTRL_PIN(123, "GPPC_D_4"),
	PINCTRL_PIN(124, "GPPC_D_5"),
	PINCTRL_PIN(125, "GPPC_D_6"),
	PINCTRL_PIN(126, "GPPC_D_7"),
	PINCTRL_PIN(127, "GPPC_D_8"),
	PINCTRL_PIN(128, "BSSB_LS2_RX"),
	PINCTRL_PIN(129, "BSSB_LS2_TX"),
	PINCTRL_PIN(130, "BSSB_LS3_RX"),
	PINCTRL_PIN(131, "BSSB_LS3_TX"),
	PINCTRL_PIN(132, "GPPC_D_13"),
	PINCTRL_PIN(133, "GPPC_D_14"),
	PINCTRL_PIN(134, "GPPC_D_15"),
	PINCTRL_PIN(135, "GPPC_D_16"),
	PINCTRL_PIN(136, "GPPC_D_17"),
	PINCTRL_PIN(137, "GPPC_D_18"),
	PINCTRL_PIN(138, "GPPC_D_19"),
	PINCTRL_PIN(139, "GSPI2_CLK_LOOPBK"),
	/* vGPIO */
	PINCTRL_PIN(140, "CNV_BTEN"),
	PINCTRL_PIN(141, "CNV_BT_HOST_WAKEB"),
	PINCTRL_PIN(142, "CNV_BT_IF_SELECT"),
	PINCTRL_PIN(143, "vCNV_BT_UART_TXD"),
	PINCTRL_PIN(144, "vCNV_BT_UART_RXD"),
	PINCTRL_PIN(145, "vCNV_BT_UART_CTS_B"),
	PINCTRL_PIN(146, "vCNV_BT_UART_RTS_B"),
	PINCTRL_PIN(147, "vCNV_MFUART1_TXD"),
	PINCTRL_PIN(148, "vCNV_MFUART1_RXD"),
	PINCTRL_PIN(149, "vCNV_MFUART1_CTS_B"),
	PINCTRL_PIN(150, "vCNV_MFUART1_RTS_B"),
	PINCTRL_PIN(151, "vUART0_TXD"),
	PINCTRL_PIN(152, "vUART0_RXD"),
	PINCTRL_PIN(153, "vUART0_CTS_B"),
	PINCTRL_PIN(154, "vUART0_RTS_B"),
	PINCTRL_PIN(155, "vISH_UART0_TXD"),
	PINCTRL_PIN(156, "vISH_UART0_RXD"),
	PINCTRL_PIN(157, "vISH_UART0_CTS_B"),
	PINCTRL_PIN(158, "vISH_UART0_RTS_B"),
	PINCTRL_PIN(159, "vCNV_BT_I2S_BCLK"),
	PINCTRL_PIN(160, "vCNV_BT_I2S_WS_SYNC"),
	PINCTRL_PIN(161, "vCNV_BT_I2S_SDO"),
	PINCTRL_PIN(162, "vCNV_BT_I2S_SDI"),
	PINCTRL_PIN(163, "vI2S2_SCLK"),
	PINCTRL_PIN(164, "vI2S2_SFRM"),
	PINCTRL_PIN(165, "vI2S2_TXD"),
	PINCTRL_PIN(166, "vI2S2_RXD"),
	PINCTRL_PIN(167, "THC0_WOT_INT"),
	PINCTRL_PIN(168, "THC1_WOT_INT"),
	/* GPP_C */
	PINCTRL_PIN(169, "SMBCLK"),
	PINCTRL_PIN(170, "SMBDATA"),
	PINCTRL_PIN(171, "SMBALERTB"),
	PINCTRL_PIN(172, "SML0CLK"),
	PINCTRL_PIN(173, "SML0DATA"),
	PINCTRL_PIN(174, "GPPC_C_5"),
	PINCTRL_PIN(175, "GPPC_C_6"),
	PINCTRL_PIN(176, "GPPC_C_7"),
	PINCTRL_PIN(177, "GPPC_C_8"),
	PINCTRL_PIN(178, "GPPC_C_9"),
	PINCTRL_PIN(179, "GPPC_C_10"),
	PINCTRL_PIN(180, "GPPC_C_11"),
	PINCTRL_PIN(181, "GPPC_C_12"),
	PINCTRL_PIN(182, "GPPC_C_13"),
	PINCTRL_PIN(183, "GPPC_C_14"),
	PINCTRL_PIN(184, "GPPC_C_15"),
	PINCTRL_PIN(185, "GPPC_C_16"),
	PINCTRL_PIN(186, "GPPC_C_17"),
	PINCTRL_PIN(187, "GPPC_C_18"),
	PINCTRL_PIN(188, "GPPC_C_19"),
	PINCTRL_PIN(189, "GPPC_C_20"),
	PINCTRL_PIN(190, "GPPC_C_21"),
	PINCTRL_PIN(191, "GPPC_C_22"),
	PINCTRL_PIN(192, "GPPC_C_23"),
	/* GPP_F */
	PINCTRL_PIN(193, "CNV_BRI_DT"),
	PINCTRL_PIN(194, "CNV_BRI_RSP"),
	PINCTRL_PIN(195, "CNV_RGI_DT"),
	PINCTRL_PIN(196, "CNV_RGI_RSP"),
	PINCTRL_PIN(197, "CNV_RF_RESET_B"),
	PINCTRL_PIN(198, "MODEM_CLKREQ"),
	PINCTRL_PIN(199, "GPPC_F_6"),
	PINCTRL_PIN(200, "GPPC_F_7"),
	PINCTRL_PIN(201, "GPPC_F_8"),
	PINCTRL_PIN(202, "BOOTMPC"),
	PINCTRL_PIN(203, "GPPC_F_10"),
	PINCTRL_PIN(204, "GPPC_F_11"),
	PINCTRL_PIN(205, "GPPC_F_12"),
	PINCTRL_PIN(206, "GPPC_F_13"),
	PINCTRL_PIN(207, "GPPC_F_14"),
	PINCTRL_PIN(208, "GPPC_F_15"),
	PINCTRL_PIN(209, "GPPC_F_16"),
	PINCTRL_PIN(210, "GPPC_F_17"),
	PINCTRL_PIN(211, "GPPC_F_18"),
	PINCTRL_PIN(212, "GPPC_F_19"),
	PINCTRL_PIN(213, "EXT_PWR_GATEB"),
	PINCTRL_PIN(214, "EXT_PWR_GATE2B"),
	PINCTRL_PIN(215, "GPPC_F_22"),
	PINCTRL_PIN(216, "GPPC_F_23"),
	PINCTRL_PIN(217, "GPPF_CLK_LOOPBACK"),
	/* HVCMOS */
	PINCTRL_PIN(218, "L_BKLTEN"),
	PINCTRL_PIN(219, "L_BKLTCTL"),
	PINCTRL_PIN(220, "L_VDDEN"),
	PINCTRL_PIN(221, "SYS_PWROK"),
	PINCTRL_PIN(222, "SYS_RESETB"),
	PINCTRL_PIN(223, "MLK_RSTB"),
	/* GPP_E */
	PINCTRL_PIN(224, "GPPC_E_0"),
	PINCTRL_PIN(225, "GPPC_E_1"),
	PINCTRL_PIN(226, "GPPC_E_2"),
	PINCTRL_PIN(227, "GPPC_E_3"),
	PINCTRL_PIN(228, "GPPC_E_4"),
	PINCTRL_PIN(229, "GPPC_E_5"),
	PINCTRL_PIN(230, "GPPC_E_6"),
	PINCTRL_PIN(231, "GPPC_E_7"),
	PINCTRL_PIN(232, "GPPC_E_8"),
	PINCTRL_PIN(233, "GPPC_E_9"),
	PINCTRL_PIN(234, "GPPC_E_10"),
	PINCTRL_PIN(235, "GPPC_E_11"),
	PINCTRL_PIN(236, "GPPC_E_12"),
	PINCTRL_PIN(237, "GPPC_E_13"),
	PINCTRL_PIN(238, "GPPC_E_14"),
	PINCTRL_PIN(239, "FIVR_DIGPB_0"),
	PINCTRL_PIN(240, "FIVR_DIGPB_1"),
	PINCTRL_PIN(241, "GPPC_E_17"),
	PINCTRL_PIN(242, "BSSB_LS0_RX"),
	PINCTRL_PIN(243, "BSSB_LS0_TX"),
	PINCTRL_PIN(244, "BSSB_LS1_RX"),
	PINCTRL_PIN(245, "BSSB_LS1_TX"),
	PINCTRL_PIN(246, "DNX_FORCE_RELOAD"),
	PINCTRL_PIN(247, "GPPC_E_23"),
	PINCTRL_PIN(248, "GPPE_CLK_LOOPBACK"),
	/* GPP_R */
	PINCTRL_PIN(249, "HDA_BCLK"),
	PINCTRL_PIN(250, "HDA_SYNC"),
	PINCTRL_PIN(251, "HDA_SDO"),
	PINCTRL_PIN(252, "HDA_SDI_0"),
	PINCTRL_PIN(253, "HDA_RSTB"),
	PINCTRL_PIN(254, "GPP_R_5"),
	PINCTRL_PIN(255, "GPP_R_6"),
	PINCTRL_PIN(256, "GPP_R_7"),
};

static const struct intel_padgroup adln_community0_gpps[] = {
	ADL_GPP(0, 0, 25, 0),				/* GPP_B */
	ADL_GPP(1, 26, 41, 32),				/* GPP_T */
	ADL_GPP(2, 42, 66, 64),				/* GPP_A */
};

static const struct intel_padgroup adln_community1_gpps[] = {
	ADL_GPP(0, 67, 74, 96),				/* GPP_S */
	ADL_GPP(1, 75, 94, 128),			/* GPP_I */
	ADL_GPP(2, 95, 118, 160),			/* GPP_H */
	ADL_GPP(3, 119, 139, 192),			/* GPP_D */
	ADL_GPP(4, 140, 168, 224),			/* vGPIO */
};

static const struct intel_padgroup adln_community4_gpps[] = {
	ADL_GPP(0, 169, 192, 256),			/* GPP_C */
	ADL_GPP(1, 193, 217, 288),			/* GPP_F */
	ADL_GPP(2, 218, 223, INTEL_GPIO_BASE_NOMAP),	/* HVCMOS */
	ADL_GPP(3, 224, 248, 320),			/* GPP_E */
};

static const struct intel_padgroup adln_community5_gpps[] = {
	ADL_GPP(0, 249, 256, 352),			/* GPP_R */
};

static const struct intel_community adln_communities[] = {
	ADL_N_COMMUNITY(0, 0, 66, adln_community0_gpps),
	ADL_N_COMMUNITY(1, 67, 168, adln_community1_gpps),
	ADL_N_COMMUNITY(2, 169, 248, adln_community4_gpps),
	ADL_N_COMMUNITY(3, 249, 256, adln_community5_gpps),
};

static const struct intel_pinctrl_soc_data adln_soc_data = {
	.pins = adln_pins,
	.npins = ARRAY_SIZE(adln_pins),
	.communities = adln_communities,
	.ncommunities = ARRAY_SIZE(adln_communities),
};

/* Alder Lake-S */
static const struct pinctrl_pin_desc adls_pins[] = {
	/* GPP_I */
	PINCTRL_PIN(0, "EXT_PWR_GATEB"),
	PINCTRL_PIN(1, "DDSP_HPD_1"),
	PINCTRL_PIN(2, "DDSP_HPD_2"),
	PINCTRL_PIN(3, "DDSP_HPD_3"),
	PINCTRL_PIN(4, "DDSP_HPD_4"),
	PINCTRL_PIN(5, "DDPB_CTRLCLK"),
	PINCTRL_PIN(6, "DDPB_CTRLDATA"),
	PINCTRL_PIN(7, "DDPC_CTRLCLK"),
	PINCTRL_PIN(8, "DDPC_CTRLDATA"),
	PINCTRL_PIN(9, "GSPI0_CS1B"),
	PINCTRL_PIN(10, "GSPI1_CS1B"),
	PINCTRL_PIN(11, "USB2_OCB_4"),
	PINCTRL_PIN(12, "USB2_OCB_5"),
	PINCTRL_PIN(13, "USB2_OCB_6"),
	PINCTRL_PIN(14, "USB2_OCB_7"),
	PINCTRL_PIN(15, "GSPI0_CS0B"),
	PINCTRL_PIN(16, "GSPI0_CLK"),
	PINCTRL_PIN(17, "GSPI0_MISO"),
	PINCTRL_PIN(18, "GSPI0_MOSI"),
	PINCTRL_PIN(19, "GSPI1_CS0B"),
	PINCTRL_PIN(20, "GSPI1_CLK"),
	PINCTRL_PIN(21, "GSPI1_MISO"),
	PINCTRL_PIN(22, "GSPI1_MOSI"),
	PINCTRL_PIN(23, "GSPI0_CLK_LOOPBK"),
	PINCTRL_PIN(24, "GSPI1_CLK_LOOPBK"),
	/* GPP_R */
	PINCTRL_PIN(25, "HDA_BCLK"),
	PINCTRL_PIN(26, "HDA_SYNC"),
	PINCTRL_PIN(27, "HDA_SDO"),
	PINCTRL_PIN(28, "HDA_SDI_0"),
	PINCTRL_PIN(29, "HDA_RSTB"),
	PINCTRL_PIN(30, "HDA_SDI_1"),
	PINCTRL_PIN(31, "GPP_R_6"),
	PINCTRL_PIN(32, "GPP_R_7"),
	PINCTRL_PIN(33, "GPP_R_8"),
	PINCTRL_PIN(34, "DDSP_HPD_A"),
	PINCTRL_PIN(35, "DDSP_HPD_B"),
	PINCTRL_PIN(36, "DDSP_HPD_C"),
	PINCTRL_PIN(37, "ISH_SPI_CSB"),
	PINCTRL_PIN(38, "ISH_SPI_CLK"),
	PINCTRL_PIN(39, "ISH_SPI_MISO"),
	PINCTRL_PIN(40, "ISH_SPI_MOSI"),
	PINCTRL_PIN(41, "DDP1_CTRLCLK"),
	PINCTRL_PIN(42, "DDP1_CTRLDATA"),
	PINCTRL_PIN(43, "DDP2_CTRLCLK"),
	PINCTRL_PIN(44, "DDP2_CTRLDATA"),
	PINCTRL_PIN(45, "DDPA_CTRLCLK"),
	PINCTRL_PIN(46, "DDPA_CTRLDATA"),
	PINCTRL_PIN(47, "GSPI2_CLK_LOOPBK"),
	/* GPP_J */
	PINCTRL_PIN(48, "CNV_PA_BLANKING"),
	PINCTRL_PIN(49, "CPU_C10_GATEB"),
	PINCTRL_PIN(50, "CNV_BRI_DT"),
	PINCTRL_PIN(51, "CNV_BRI_RSP"),
	PINCTRL_PIN(52, "CNV_RGI_DT"),
	PINCTRL_PIN(53, "CNV_RGI_RSP"),
	PINCTRL_PIN(54, "CNV_MFUART2_RXD"),
	PINCTRL_PIN(55, "CNV_MFUART2_TXD"),
	PINCTRL_PIN(56, "SRCCLKREQB_16"),
	PINCTRL_PIN(57, "SRCCLKREQB_17"),
	PINCTRL_PIN(58, "BSSB_LS_RX"),
	PINCTRL_PIN(59, "BSSB_LS_TX"),
	/* vGPIO */
	PINCTRL_PIN(60, "CNV_BTEN"),
	PINCTRL_PIN(61, "CNV_BT_HOST_WAKEB"),
	PINCTRL_PIN(62, "CNV_BT_IF_SELECT"),
	PINCTRL_PIN(63, "vCNV_BT_UART_TXD"),
	PINCTRL_PIN(64, "vCNV_BT_UART_RXD"),
	PINCTRL_PIN(65, "vCNV_BT_UART_CTS_B"),
	PINCTRL_PIN(66, "vCNV_BT_UART_RTS_B"),
	PINCTRL_PIN(67, "vCNV_MFUART1_TXD"),
	PINCTRL_PIN(68, "vCNV_MFUART1_RXD"),
	PINCTRL_PIN(69, "vCNV_MFUART1_CTS_B"),
	PINCTRL_PIN(70, "vCNV_MFUART1_RTS_B"),
	PINCTRL_PIN(71, "vUART0_TXD"),
	PINCTRL_PIN(72, "vUART0_RXD"),
	PINCTRL_PIN(73, "vUART0_CTS_B"),
	PINCTRL_PIN(74, "vUART0_RTS_B"),
	PINCTRL_PIN(75, "vISH_UART0_TXD"),
	PINCTRL_PIN(76, "vISH_UART0_RXD"),
	PINCTRL_PIN(77, "vISH_UART0_CTS_B"),
	PINCTRL_PIN(78, "vISH_UART0_RTS_B"),
	PINCTRL_PIN(79, "vCNV_BT_I2S_BCLK"),
	PINCTRL_PIN(80, "vCNV_BT_I2S_WS_SYNC"),
	PINCTRL_PIN(81, "vCNV_BT_I2S_SDO"),
	PINCTRL_PIN(82, "vCNV_BT_I2S_SDI"),
	PINCTRL_PIN(83, "vI2S2_SCLK"),
	PINCTRL_PIN(84, "vI2S2_SFRM"),
	PINCTRL_PIN(85, "vI2S2_TXD"),
	PINCTRL_PIN(86, "vI2S2_RXD"),
	/* vGPIO_0 */
	PINCTRL_PIN(87, "ESPI_USB_OCB_0"),
	PINCTRL_PIN(88, "ESPI_USB_OCB_1"),
	PINCTRL_PIN(89, "ESPI_USB_OCB_2"),
	PINCTRL_PIN(90, "ESPI_USB_OCB_3"),
	PINCTRL_PIN(91, "USB_CPU_OCB_0"),
	PINCTRL_PIN(92, "USB_CPU_OCB_1"),
	PINCTRL_PIN(93, "USB_CPU_OCB_2"),
	PINCTRL_PIN(94, "USB_CPU_OCB_3"),
	/* GPP_B */
	PINCTRL_PIN(95, "PCIE_LNK_DOWN"),
	PINCTRL_PIN(96, "ISH_UART0_RTSB"),
	PINCTRL_PIN(97, "VRALERTB"),
	PINCTRL_PIN(98, "CPU_GP_2"),
	PINCTRL_PIN(99, "CPU_GP_3"),
	PINCTRL_PIN(100, "SX_EXIT_HOLDOFFB"),
	PINCTRL_PIN(101, "CLKOUT_48"),
	PINCTRL_PIN(102, "ISH_GP_7"),
	PINCTRL_PIN(103, "ISH_GP_0"),
	PINCTRL_PIN(104, "ISH_GP_1"),
	PINCTRL_PIN(105, "ISH_GP_2"),
	PINCTRL_PIN(106, "I2S_MCLK"),
	PINCTRL_PIN(107, "SLP_S0B"),
	PINCTRL_PIN(108, "PLTRSTB"),
	PINCTRL_PIN(109, "SPKR"),
	PINCTRL_PIN(110, "ISH_GP_3"),
	PINCTRL_PIN(111, "ISH_GP_4"),
	PINCTRL_PIN(112, "ISH_GP_5"),
	PINCTRL_PIN(113, "PMCALERTB"),
	PINCTRL_PIN(114, "FUSA_DIAGTEST_EN"),
	PINCTRL_PIN(115, "FUSA_DIAGTEST_MODE"),
	PINCTRL_PIN(116, "GPP_B_21"),
	PINCTRL_PIN(117, "GPP_B_22"),
	PINCTRL_PIN(118, "SML1ALERTB"),
	/* GPP_G */
	PINCTRL_PIN(119, "GPP_G_0"),
	PINCTRL_PIN(120, "GPP_G_1"),
	PINCTRL_PIN(121, "DNX_FORCE_RELOAD"),
	PINCTRL_PIN(122, "GMII_MDC_0"),
	PINCTRL_PIN(123, "GMII_MDIO_0"),
	PINCTRL_PIN(124, "SLP_DRAMB"),
	PINCTRL_PIN(125, "GPP_G_6"),
	PINCTRL_PIN(126, "GPP_G_7"),
	/* GPP_H */
	PINCTRL_PIN(127, "SRCCLKREQB_18"),
	PINCTRL_PIN(128, "GPP_H_1"),
	PINCTRL_PIN(129, "SRCCLKREQB_8"),
	PINCTRL_PIN(130, "SRCCLKREQB_9"),
	PINCTRL_PIN(131, "SRCCLKREQB_10"),
	PINCTRL_PIN(132, "SRCCLKREQB_11"),
	PINCTRL_PIN(133, "SRCCLKREQB_12"),
	PINCTRL_PIN(134, "SRCCLKREQB_13"),
	PINCTRL_PIN(135, "SRCCLKREQB_14"),
	PINCTRL_PIN(136, "SRCCLKREQB_15"),
	PINCTRL_PIN(137, "SML2CLK"),
	PINCTRL_PIN(138, "SML2DATA"),
	PINCTRL_PIN(139, "SML2ALERTB"),
	PINCTRL_PIN(140, "SML3CLK"),
	PINCTRL_PIN(141, "SML3DATA"),
	PINCTRL_PIN(142, "SML3ALERTB"),
	PINCTRL_PIN(143, "SML4CLK"),
	PINCTRL_PIN(144, "SML4DATA"),
	PINCTRL_PIN(145, "SML4ALERTB"),
	PINCTRL_PIN(146, "ISH_I2C0_SDA"),
	PINCTRL_PIN(147, "ISH_I2C0_SCL"),
	PINCTRL_PIN(148, "ISH_I2C1_SDA"),
	PINCTRL_PIN(149, "ISH_I2C1_SCL"),
	PINCTRL_PIN(150, "TIME_SYNC_0"),
	/* SPI0 */
	PINCTRL_PIN(151, "SPI0_IO_2"),
	PINCTRL_PIN(152, "SPI0_IO_3"),
	PINCTRL_PIN(153, "SPI0_MOSI_IO_0"),
	PINCTRL_PIN(154, "SPI0_MISO_IO_1"),
	PINCTRL_PIN(155, "SPI0_TPM_CSB"),
	PINCTRL_PIN(156, "SPI0_FLASH_0_CSB"),
	PINCTRL_PIN(157, "SPI0_FLASH_1_CSB"),
	PINCTRL_PIN(158, "SPI0_CLK"),
	PINCTRL_PIN(159, "SPI0_CLK_LOOPBK"),
	/* GPP_A */
	PINCTRL_PIN(160, "ESPI_IO_0"),
	PINCTRL_PIN(161, "ESPI_IO_1"),
	PINCTRL_PIN(162, "ESPI_IO_2"),
	PINCTRL_PIN(163, "ESPI_IO_3"),
	PINCTRL_PIN(164, "ESPI_CS0B"),
	PINCTRL_PIN(165, "ESPI_CLK"),
	PINCTRL_PIN(166, "ESPI_RESETB"),
	PINCTRL_PIN(167, "ESPI_CS1B"),
	PINCTRL_PIN(168, "ESPI_CS2B"),
	PINCTRL_PIN(169, "ESPI_CS3B"),
	PINCTRL_PIN(170, "ESPI_ALERT0B"),
	PINCTRL_PIN(171, "ESPI_ALERT1B"),
	PINCTRL_PIN(172, "ESPI_ALERT2B"),
	PINCTRL_PIN(173, "ESPI_ALERT3B"),
	PINCTRL_PIN(174, "GPP_A_14"),
	PINCTRL_PIN(175, "ESPI_CLK_LOOPBK"),
	/* GPP_C */
	PINCTRL_PIN(176, "SMBCLK"),
	PINCTRL_PIN(177, "SMBDATA"),
	PINCTRL_PIN(178, "SMBALERTB"),
	PINCTRL_PIN(179, "ISH_UART0_RXD"),
	PINCTRL_PIN(180, "ISH_UART0_TXD"),
	PINCTRL_PIN(181, "SML0ALERTB"),
	PINCTRL_PIN(182, "ISH_I2C2_SDA"),
	PINCTRL_PIN(183, "ISH_I2C2_SCL"),
	PINCTRL_PIN(184, "UART0_RXD"),
	PINCTRL_PIN(185, "UART0_TXD"),
	PINCTRL_PIN(186, "UART0_RTSB"),
	PINCTRL_PIN(187, "UART0_CTSB"),
	PINCTRL_PIN(188, "UART1_RXD"),
	PINCTRL_PIN(189, "UART1_TXD"),
	PINCTRL_PIN(190, "UART1_RTSB"),
	PINCTRL_PIN(191, "UART1_CTSB"),
	PINCTRL_PIN(192, "I2C0_SDA"),
	PINCTRL_PIN(193, "I2C0_SCL"),
	PINCTRL_PIN(194, "I2C1_SDA"),
	PINCTRL_PIN(195, "I2C1_SCL"),
	PINCTRL_PIN(196, "UART2_RXD"),
	PINCTRL_PIN(197, "UART2_TXD"),
	PINCTRL_PIN(198, "UART2_RTSB"),
	PINCTRL_PIN(199, "UART2_CTSB"),
	/* GPP_S */
	PINCTRL_PIN(200, "SNDW1_CLK"),
	PINCTRL_PIN(201, "SNDW1_DATA"),
	PINCTRL_PIN(202, "SNDW2_CLK"),
	PINCTRL_PIN(203, "SNDW2_DATA"),
	PINCTRL_PIN(204, "SNDW3_CLK"),
	PINCTRL_PIN(205, "SNDW3_DATA"),
	PINCTRL_PIN(206, "SNDW4_CLK"),
	PINCTRL_PIN(207, "SNDW4_DATA"),
	/* GPP_E */
	PINCTRL_PIN(208, "SATAXPCIE_0"),
	PINCTRL_PIN(209, "SATAXPCIE_1"),
	PINCTRL_PIN(210, "SATAXPCIE_2"),
	PINCTRL_PIN(211, "CPU_GP_0"),
	PINCTRL_PIN(212, "SATA_DEVSLP_0"),
	PINCTRL_PIN(213, "SATA_DEVSLP_1"),
	PINCTRL_PIN(214, "SATA_DEVSLP_2"),
	PINCTRL_PIN(215, "CPU_GP_1"),
	PINCTRL_PIN(216, "SATA_LEDB"),
	PINCTRL_PIN(217, "USB2_OCB_0"),
	PINCTRL_PIN(218, "USB2_OCB_1"),
	PINCTRL_PIN(219, "USB2_OCB_2"),
	PINCTRL_PIN(220, "USB2_OCB_3"),
	PINCTRL_PIN(221, "SPI1_CSB"),
	PINCTRL_PIN(222, "SPI1_CLK"),
	PINCTRL_PIN(223, "SPI1_MISO_IO_1"),
	PINCTRL_PIN(224, "SPI1_MOSI_IO_0"),
	PINCTRL_PIN(225, "SPI1_IO_2"),
	PINCTRL_PIN(226, "SPI1_IO_3"),
	PINCTRL_PIN(227, "GPP_E_19"),
	PINCTRL_PIN(228, "GPP_E_20"),
	PINCTRL_PIN(229, "ISH_UART0_CTSB"),
	PINCTRL_PIN(230, "SPI1_CLK_LOOPBK"),
	/* GPP_K */
	PINCTRL_PIN(231, "GSXDOUT"),
	PINCTRL_PIN(232, "GSXSLOAD"),
	PINCTRL_PIN(233, "GSXDIN"),
	PINCTRL_PIN(234, "GSXSRESETB"),
	PINCTRL_PIN(235, "GSXCLK"),
	PINCTRL_PIN(236, "ADR_COMPLETE"),
	PINCTRL_PIN(237, "GPP_K_6"),
	PINCTRL_PIN(238, "GPP_K_7"),
	PINCTRL_PIN(239, "CORE_VID_0"),
	PINCTRL_PIN(240, "CORE_VID_1"),
	PINCTRL_PIN(241, "GPP_K_10"),
	PINCTRL_PIN(242, "GPP_K_11"),
	PINCTRL_PIN(243, "SYS_PWROK"),
	PINCTRL_PIN(244, "SYS_RESETB"),
	PINCTRL_PIN(245, "MLK_RSTB"),
	/* GPP_F */
	PINCTRL_PIN(246, "SATAXPCIE_3"),
	PINCTRL_PIN(247, "SATAXPCIE_4"),
	PINCTRL_PIN(248, "SATAXPCIE_5"),
	PINCTRL_PIN(249, "SATAXPCIE_6"),
	PINCTRL_PIN(250, "SATAXPCIE_7"),
	PINCTRL_PIN(251, "SATA_DEVSLP_3"),
	PINCTRL_PIN(252, "SATA_DEVSLP_4"),
	PINCTRL_PIN(253, "SATA_DEVSLP_5"),
	PINCTRL_PIN(254, "SATA_DEVSLP_6"),
	PINCTRL_PIN(255, "SATA_DEVSLP_7"),
	PINCTRL_PIN(256, "SATA_SCLOCK"),
	PINCTRL_PIN(257, "SATA_SLOAD"),
	PINCTRL_PIN(258, "SATA_SDATAOUT1"),
	PINCTRL_PIN(259, "SATA_SDATAOUT0"),
	PINCTRL_PIN(260, "PS_ONB"),
	PINCTRL_PIN(261, "M2_SKT2_CFG_0"),
	PINCTRL_PIN(262, "M2_SKT2_CFG_1"),
	PINCTRL_PIN(263, "M2_SKT2_CFG_2"),
	PINCTRL_PIN(264, "M2_SKT2_CFG_3"),
	PINCTRL_PIN(265, "L_VDDEN"),
	PINCTRL_PIN(266, "L_BKLTEN"),
	PINCTRL_PIN(267, "L_BKLTCTL"),
	PINCTRL_PIN(268, "VNN_CTRL"),
	PINCTRL_PIN(269, "GPP_F_23"),
	/* GPP_D */
	PINCTRL_PIN(270, "SRCCLKREQB_0"),
	PINCTRL_PIN(271, "SRCCLKREQB_1"),
	PINCTRL_PIN(272, "SRCCLKREQB_2"),
	PINCTRL_PIN(273, "SRCCLKREQB_3"),
	PINCTRL_PIN(274, "SML1CLK"),
	PINCTRL_PIN(275, "I2S2_SFRM"),
	PINCTRL_PIN(276, "I2S2_TXD"),
	PINCTRL_PIN(277, "I2S2_RXD"),
	PINCTRL_PIN(278, "I2S2_SCLK"),
	PINCTRL_PIN(279, "SML0CLK"),
	PINCTRL_PIN(280, "SML0DATA"),
	PINCTRL_PIN(281, "SRCCLKREQB_4"),
	PINCTRL_PIN(282, "SRCCLKREQB_5"),
	PINCTRL_PIN(283, "SRCCLKREQB_6"),
	PINCTRL_PIN(284, "SRCCLKREQB_7"),
	PINCTRL_PIN(285, "SML1DATA"),
	PINCTRL_PIN(286, "GSPI3_CS0B"),
	PINCTRL_PIN(287, "GSPI3_CLK"),
	PINCTRL_PIN(288, "GSPI3_MISO"),
	PINCTRL_PIN(289, "GSPI3_MOSI"),
	PINCTRL_PIN(290, "UART3_RXD"),
	PINCTRL_PIN(291, "UART3_TXD"),
	PINCTRL_PIN(292, "UART3_RTSB"),
	PINCTRL_PIN(293, "UART3_CTSB"),
	PINCTRL_PIN(294, "GSPI3_CLK_LOOPBK"),
	/* JTAG */
	PINCTRL_PIN(295, "JTAG_TDO"),
	PINCTRL_PIN(296, "JTAGX"),
	PINCTRL_PIN(297, "PRDYB"),
	PINCTRL_PIN(298, "PREQB"),
	PINCTRL_PIN(299, "JTAG_TDI"),
	PINCTRL_PIN(300, "JTAG_TMS"),
	PINCTRL_PIN(301, "JTAG_TCK"),
	PINCTRL_PIN(302, "DBG_PMODE"),
	PINCTRL_PIN(303, "CPU_TRSTB"),
};

static const struct intel_padgroup adls_community0_gpps[] = {
	ADL_GPP(0, 0, 24, 0),				/* GPP_I */
	ADL_GPP(1, 25, 47, 32),				/* GPP_R */
	ADL_GPP(2, 48, 59, 64),				/* GPP_J */
	ADL_GPP(3, 60, 86, 96),				/* vGPIO */
	ADL_GPP(4, 87, 94, 128),			/* vGPIO_0 */
};

static const struct intel_padgroup adls_community1_gpps[] = {
	ADL_GPP(0, 95, 118, 160),			/* GPP_B */
	ADL_GPP(1, 119, 126, 192),			/* GPP_G */
	ADL_GPP(2, 127, 150, 224),			/* GPP_H */
};

static const struct intel_padgroup adls_community3_gpps[] = {
	ADL_GPP(0, 151, 159, INTEL_GPIO_BASE_NOMAP),	/* SPI0 */
	ADL_GPP(1, 160, 175, 256),			/* GPP_A */
	ADL_GPP(2, 176, 199, 288),			/* GPP_C */
};

static const struct intel_padgroup adls_community4_gpps[] = {
	ADL_GPP(0, 200, 207, 320),			/* GPP_S */
	ADL_GPP(1, 208, 230, 352),			/* GPP_E */
	ADL_GPP(2, 231, 245, 384),			/* GPP_K */
	ADL_GPP(3, 246, 269, 416),			/* GPP_F */
};

static const struct intel_padgroup adls_community5_gpps[] = {
	ADL_GPP(0, 270, 294, 448),			/* GPP_D */
	ADL_GPP(1, 295, 303, INTEL_GPIO_BASE_NOMAP),	/* JTAG */
};

static const struct intel_community adls_communities[] = {
	ADL_S_COMMUNITY(0, 0, 94, adls_community0_gpps),
	ADL_S_COMMUNITY(1, 95, 150, adls_community1_gpps),
	ADL_S_COMMUNITY(2, 151, 199, adls_community3_gpps),
	ADL_S_COMMUNITY(3, 200, 269, adls_community4_gpps),
	ADL_S_COMMUNITY(4, 270, 303, adls_community5_gpps),
};

static const struct intel_pinctrl_soc_data adls_soc_data = {
	.pins = adls_pins,
	.npins = ARRAY_SIZE(adls_pins),
	.communities = adls_communities,
	.ncommunities = ARRAY_SIZE(adls_communities),
};

static const struct acpi_device_id adl_pinctrl_acpi_match[] = {
	{ "INTC1056", (kernel_ulong_t)&adls_soc_data },
	{ "INTC1057", (kernel_ulong_t)&adln_soc_data },
	{ "INTC1085", (kernel_ulong_t)&adls_soc_data },
	{ }
};
MODULE_DEVICE_TABLE(acpi, adl_pinctrl_acpi_match);

static struct platform_driver adl_pinctrl_driver = {
	.probe = intel_pinctrl_probe_by_hid,
	.driver = {
		.name = "alderlake-pinctrl",
		.acpi_match_table = adl_pinctrl_acpi_match,
		.pm = pm_sleep_ptr(&intel_pinctrl_pm_ops),
	},
};
module_platform_driver(adl_pinctrl_driver);

MODULE_AUTHOR("Andy Shevchenko <andriy.shevchenko@linux.intel.com>");
MODULE_DESCRIPTION("Intel Alder Lake PCH pinctrl/GPIO driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS("PINCTRL_INTEL");
