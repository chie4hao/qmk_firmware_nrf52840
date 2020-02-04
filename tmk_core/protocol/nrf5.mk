PROTOCOL_DIR = protocol
NRF_DIR = $(PROTOCOL_DIR)/nrf5

SRC += \
	$(NRF_DIR)/adc.c \
	$(NRF_DIR)/outputselect.c \
	$(NRF_DIR)/main.c \
	$(NRF_DIR)/outputselect.c \
	$(NRF_DIR)/usb/nrf5_usb.c \
	$(NRF_DIR)/usb/fixed/app_usbd_hid_generic.c \
	$(NRF_DIR)/ble/ble_service.c \
	$(NRF_DIR)/ble/ble_hids_service.c \
	$(NRF_DIR)/ble/ble_midi.c \

VPATH += $(TMK_PATH)/$(PROTOCOL_DIR)
VPATH += $(TMK_PATH)/$(NRF_DIR)
VPATH += $(TMK_PATH)/$(NRF_DIR)/ble
VPATH += $(TMK_PATH)/$(NRF_DIR)/usb
VPATH += $(TMK_PATH)/$(NRF_DIR)/lufa_utils

ifeq ($(strip $(MIDI_ENABLE)), yes)
  include $(TMK_PATH)/protocol/midi.mk
endif
