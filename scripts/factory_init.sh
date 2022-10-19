#!/zsh

# Setup DM connection basics
attr set lwm2m_security 3
attr set lwm2m_bootstrap 0

# Generate private keys
pki keygen p2p
