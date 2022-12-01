#!/zsh

# Setup DM connection basics
attr set lwm2m_security 3
attr set lwm2m_bootstrap 0

# Disable logging for these attributes
attr quiet battery_age 1
attr quiet qrtc 1
attr quiet bluetooth_flags 1
attr quiet lwm2m_pwr_src_volt 1

# Generate private keys
pki keygen dm
pki keygen tel
pki keygen p2p
pki keygen fs
