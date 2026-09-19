#############
# Thermostat settings

# Uncomment to enable slave programming also via buttons/wheel
#export REMOTE_SETTING_ONLY=0

# Switch to 1 to enable slave motor startup battery compensation
export MOTOR_COMPENSATE_BATTERY=0

#############
# Master settings

# Set the proper master type
#MASTERTYPE=JEENODE=1
#MASTERTYPE=NANODE=1

#############
# RFM settings

# Enable diagnostic to fine tune RFM frequency by default
export RFM_TUNING=1
# Set default HR slave address, valid range is 1-28
export RFM_DEVICE_ADDRESS=28
# Set default security keys, shared by master and slave
export SECURITY_KEY_0=0x01
export SECURITY_KEY_1=0x23
export SECURITY_KEY_2=0x45
export SECURITY_KEY_3=0x67
export SECURITY_KEY_4=0x89
export SECURITY_KEY_5=0x01
export SECURITY_KEY_6=0x23
export SECURITY_KEY_7=0x45

# Main RFM frequency (depends on RFM variant)
export RFM_FREQ_MAIN=868

# Decimal part of the RFM frequency
export RFM_FREQ_FINE=0.35

#############

# Safe default for this fork: plain Honeywell HR20 without radio.
# Radio builds remain available through their explicit targets and `make all`.
default: HR20_original_sww

all: HR20_universal_jd HR20_rfm_int_sww HR20_rfm_int_hww HR20_rfm_ext_sww HR20_original_sww HR20_original_hww HR25_original_sww HR25_rfm_int_sww thermotronic_sww rfm_master
	 cp src/license.txt $(DEST)/

clean:
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_universal_jd/hr20 OBJDIR=HR20_universal_jd
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_rfm_int_sww/hr20 OBJDIR=HR20_rfm_int_sww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_rfm_int_hww/hr20 OBJDIR=HR20_rfm_int_hww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_rfm_ext_sww/hr20 OBJDIR=HR20_rfm_ext_sww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_original_sww/hr20 OBJDIR=HR20_original_sww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR20_original_hww/hr20 OBJDIR=HR20_original_hww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR25_original_sww/hr20 OBJDIR=HR25_original_sww
	 $(MAKE) clean -C src TARGET=../$(DEST)/HR25_rfm_int_sww/hr20 OBJDIR=HR25_rfm_int_sww
	 $(MAKE) clean -C src TARGET=../$(DEST)/thermotronic_sww/hr20 OBJDIR=thermotronic_sww
	 $(MAKE) clean -C rfm-master TARGET=../$(DEST)/rfm_master/rfm_master OBJDIR=rfm_master
	@rm -f $(DEST)/license.txt

beauty:
	 uncrustify rfm-master/*.h rfm-master/*.c src/*.h src/*.c common/*.h common/*.c -c uncrustify.conf --replace --no-backup

check:
	 cppcheck --inline-suppr --force --error-exitcode=1 --suppress=objectIndex src common >/dev/null

VER=

DEST=bin

HR20_universal_jd:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		RFM=1 \
		RFM_WIRE=JD_INTERNAL \
		RFM_TUNING=1 \
		RFM_RUNTIME_DETECT=1 \
		ENABLE_UART=1 \
		REMOTE_SETTING_ONLY=0 \
		HW_WINDOW_DETECTION=0 \
		WINDOW_DETECTION_RUNTIME=1 \
		GC_SECTIONS=1 \
		LTO=1 \
		FLAGS="-fno-inline-small-functions -fno-inline-functions-called-once" \
		REV=-DREVISION=\\\"$(REV)\\\"

HR20_rfm_int_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		RFM=1 \
		REV=-DREVISION=\\\"$(REV)\\\"

HR20_rfm_int_hww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=1 \
		RFM=1 \
		REV=-DREVISION=\\\"$(REV)\\\"

HR20_rfm_ext_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0\
		RFM=1 \
		RFM_WIRE=MARIOJTAG \
		REV=-DREVISION=\\\"$(REV)\\\"

HR20_original_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		WINDOW_DETECTION_RUNTIME=1 \
		RFM=0 \
		REV=-DREVISION=\\\"$(REV)\\\"

# Compatibility target: standalone HR20 now selects HW/SW from EEPROM.
HR20_original_hww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		WINDOW_DETECTION_RUNTIME=1 \
		RFM=0 \
		REV=-DREVISION=\\\"$(REV)\\\"

HR25_original_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		RFM=0 \
		HW=HR25 \
		REV=-DREVISION=\\\"$(REV)\\\"

HR25_rfm_int_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		RFM_WIRE=TK_INTERNAL \
		RFM=1 \
		HW=HR25 \
		REV=-DREVISION=\\\"$(REV)\\\"

thermotronic_sww:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C src \
		TARGET=../$(DEST)/$@/hr20 \
		OBJDIR=$@ \
		HW_WINDOW_DETECTION=0 \
		RFM=0 \
		HW=THERMOTRONIC \
		REV=-DREVISION=\\\"$(REV)\\\"

rfm_master:
	 $(shell mkdir $(DEST)/$@ 2>/dev/null)
	 $(MAKE) -C rfm-master \
		TARGET=../$(DEST)/$@/rfm_master \
		OBJDIR=$@ \
		${MASTERTYPE} \
		REV=-DREVISION=\\\"$(REV)\\\"